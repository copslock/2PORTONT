/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1991, 1992  Linus Torvalds
 * Copyright (C) 1994 - 2000  Ralf Baechle
 * Copyright (C) 1999, 2000 Silicon Graphics, Inc.
 */
#include <linux/cache.h>
#include <linux/irqflags.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/personality.h>
#include <linux/smp.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/ptrace.h>
#include <linux/unistd.h>
#include <linux/compiler.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>

#include <asm/abi.h>
#include <asm/asm.h>
#include <linux/bitops.h>
#include <asm/cacheflush.h>
#include <asm/sim.h>
#include <asm/ucontext.h>
#include <asm/cpu-features.h>
#include <asm/vdso.h>

#include "signal-common.h"

/*
 * Horribly complicated - with the bloody RM9000 workarounds enabled
 * the signal trampolines is moving to the end of the structure so we can
 * increase the alignment without breaking software compatibility.
 */
struct sigframe {
	u32 sf_ass[4];		/* argument save space for o32 */
	u32 sf_pad[2];		/* Was: signal trampoline */
	struct sigcontext sf_sc;
	sigset_t sf_mask;
};

struct rt_sigframe {
	u32 rs_ass[4];		/* argument save space for o32 */
	u32 rs_pad[2];		/* Was: signal trampoline */
	struct siginfo rs_info;
	struct ucontext rs_uc;
};

/*
 * Helper routines
 */
int setup_sigcontext(struct pt_regs *regs, struct sigcontext __user *sc)
{
	int err = 0;
	int i;

	err |= __put_user(0, &sc->sc_regs[0]);
	for (i = 1; i < 32; i++)
		err |= __put_user(regs->regs[i], &sc->sc_regs[i]);

	err |= __put_user(regs->hi, &sc->sc_mdhi);
	err |= __put_user(regs->lo, &sc->sc_mdlo);
	err |= __put_user(regs->cp0_epc, &sc->sc_pc);

	return err;
}

int restore_sigcontext(struct pt_regs *regs, struct sigcontext __user *sc)
{
	int err = 0;
	int i;

	/* Always make any pending restarted system calls return -EINTR */
	current_thread_info()->restart_block.fn = do_no_restart_syscall;

	err |= __get_user(regs->cp0_epc, &sc->sc_pc);
	err |= __get_user(regs->lo, &sc->sc_mdlo);
	err |= __get_user(regs->hi, &sc->sc_mdhi);

	for (i = 1; i < 32; i++)
		err |= __get_user(regs->regs[i], &sc->sc_regs[i]);

	return err;
}

void __user *get_sigframe(struct k_sigaction *ka, struct pt_regs *regs,
			  size_t frame_size)
{
	unsigned long sp;

	/* Default to using normal stack */
	sp = regs->regs[29];

	/*
	 * FPU emulator may have it's own trampoline active just
	 * above the user stack, 16-bytes before the next lowest
	 * 16 byte boundary.  Try to avoid trashing it.
	 */
	sp -= 32;

	/* This is the X/Open sanctioned signal stack switching.  */
	if ((ka->sa.sa_flags & SA_ONSTACK) && (sas_ss_flags (sp) == 0))
		sp = current->sas_ss_sp + current->sas_ss_size;

	return (void __user *)((sp - frame_size) & ALMASK);
}

/*
 * Atomically swap in the new signal mask, and wait for a signal.
 */

#ifdef CONFIG_TRAD_SIGNALS
asmlinkage int sys_sigsuspend(nabi_no_regargs struct pt_regs regs)
{
	sigset_t newset;
	sigset_t __user *uset;

	uset = (sigset_t __user *) regs.regs[4];
	if (copy_from_user(&newset, uset, sizeof(sigset_t)))
		return -EFAULT;
	sigdelsetmask(&newset, ~_BLOCKABLE);

	spin_lock_irq(&current->sighand->siglock);
	current->saved_sigmask = current->blocked;
	current->blocked = newset;
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);

	current->state = TASK_INTERRUPTIBLE;
	schedule();
	set_thread_flag(TIF_RESTORE_SIGMASK);
	return -ERESTARTNOHAND;
}
#endif

asmlinkage int sys_rt_sigsuspend(nabi_no_regargs struct pt_regs regs)
{
	sigset_t newset;
	sigset_t __user *unewset;
	size_t sigsetsize;

	/* XXX Don't preclude handling different sized sigset_t's.  */
	sigsetsize = regs.regs[5];
	if (sigsetsize != sizeof(sigset_t))
		return -EINVAL;

	unewset = (sigset_t __user *) regs.regs[4];
	if (copy_from_user(&newset, unewset, sizeof(newset)))
		return -EFAULT;
	sigdelsetmask(&newset, ~_BLOCKABLE);

	spin_lock_irq(&current->sighand->siglock);
	current->saved_sigmask = current->blocked;
	current->blocked = newset;
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);

	current->state = TASK_INTERRUPTIBLE;
	schedule();
	set_thread_flag(TIF_RESTORE_SIGMASK);
	return -ERESTARTNOHAND;
}

#ifdef CONFIG_TRAD_SIGNALS
SYSCALL_DEFINE3(sigaction, int, sig, const struct sigaction __user *, act,
	struct sigaction __user *, oact)
{
	struct k_sigaction new_ka, old_ka;
	int ret;
	int err = 0;

	if (act) {
		old_sigset_t mask;

		if (!access_ok(VERIFY_READ, act, sizeof(*act)))
			return -EFAULT;
		err |= __get_user(new_ka.sa.sa_handler, &act->sa_handler);
		err |= __get_user(new_ka.sa.sa_flags, &act->sa_flags);
		err |= __get_user(mask, &act->sa_mask.sig[0]);
		if (err)
			return -EFAULT;

		siginitset(&new_ka.sa.sa_mask, mask);
	}

	ret = do_sigaction(sig, act ? &new_ka : NULL, oact ? &old_ka : NULL);

	if (!ret && oact) {
		if (!access_ok(VERIFY_WRITE, oact, sizeof(*oact)))
			return -EFAULT;
		err |= __put_user(old_ka.sa.sa_flags, &oact->sa_flags);
		err |= __put_user(old_ka.sa.sa_handler, &oact->sa_handler);
		err |= __put_user(old_ka.sa.sa_mask.sig[0], oact->sa_mask.sig);
		err |= __put_user(0, &oact->sa_mask.sig[1]);
		err |= __put_user(0, &oact->sa_mask.sig[2]);
		err |= __put_user(0, &oact->sa_mask.sig[3]);
		if (err)
			return -EFAULT;
	}

	return ret;
}
#endif

asmlinkage int sys_sigaltstack(nabi_no_regargs struct pt_regs regs)
{
	const stack_t __user *uss = (const stack_t __user *) regs.regs[4];
	stack_t __user *uoss = (stack_t __user *) regs.regs[5];
	unsigned long usp = regs.regs[29];

	return do_sigaltstack(uss, uoss, usp);
}

#ifdef CONFIG_TRAD_SIGNALS
asmlinkage void sys_sigreturn(nabi_no_regargs struct pt_regs regs)
{
	struct sigframe __user *frame;
	sigset_t blocked;
	int sig;

	frame = (struct sigframe __user *) regs.regs[29];
	if (!access_ok(VERIFY_READ, frame, sizeof(*frame)))
		goto badframe;
	if (__copy_from_user(&blocked, &frame->sf_mask, sizeof(blocked)))
		goto badframe;

	sigdelsetmask(&blocked, ~_BLOCKABLE);
	spin_lock_irq(&current->sighand->siglock);
	current->blocked = blocked;
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);

	sig = restore_sigcontext(&regs, &frame->sf_sc);
	if (sig < 0)
		goto badframe;
	else if (sig)
		force_sig(sig, current);

	/*
	 * Don't let your children do this ...
	 */
	__asm__ __volatile__(
		"move\t$29, %0\n\t"
		"j\tsyscall_exit"
		:/* no outputs */
		:"r" (&regs));
	/* Unreached */

badframe:
	force_sig(SIGSEGV, current);
}
#endif /* CONFIG_TRAD_SIGNALS */

asmlinkage void sys_rt_sigreturn(nabi_no_regargs struct pt_regs regs)
{
	struct rt_sigframe __user *frame;
	sigset_t set;
	int sig;

	frame = (struct rt_sigframe __user *) regs.regs[29];
	if (!access_ok(VERIFY_READ, frame, sizeof(*frame)))
		goto badframe;

	if (__copy_from_user(&set, &frame->rs_uc.uc_sigmask, sizeof(set)))
		goto badframe;

	sigdelsetmask(&set, ~_BLOCKABLE);
	spin_lock_irq(&current->sighand->siglock);
	current->blocked = set;
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);

	sig = restore_sigcontext(&regs, &frame->rs_uc.uc_mcontext);
	if (sig < 0)
		goto badframe;
	else if (sig)
		force_sig(sig, current);

	/* It is more difficult to avoid calling this function than to
	   call it and ignore errors.  */
    do_sigaltstack(&frame->rs_uc.uc_stack, NULL, regs.regs[29]);

	/*
	 * Don't let your children do this ...
	 */
	__asm__ __volatile__(
		"move\t$29, %0\n\t"
		"j\tsyscall_exit"
		:/* no outputs */
		:"r" (&regs));
	/* Unreached */

badframe:
	force_sig(SIGSEGV, current);
}

#ifdef CONFIG_TRAD_SIGNALS
static int setup_frame(void *sig_return, struct k_sigaction * ka, struct pt_regs *regs,
	int signr, sigset_t *set)
{
	struct sigframe __user *frame;
	int err = 0;

	frame = get_sigframe(ka, regs, sizeof(*frame));
	if (!access_ok(VERIFY_WRITE, frame, sizeof (*frame)))
		goto give_sigsegv;

	err |= setup_sigcontext(regs, &frame->sf_sc);
	err |= __copy_to_user(&frame->sf_mask, set, sizeof(*set));
	if (err)
		goto give_sigsegv;

	/*
	 * Arguments to signal handler:
	 *
	 *   a0 = signal number
	 *   a1 = 0 (should be cause)
	 *   a2 = pointer to struct sigcontext
	 *
	 * $25 and c0_epc point to the signal handler, $29 points to the
	 * struct sigframe.
	 */
	regs->regs[ 4] = signr;
	regs->regs[ 5] = 0;
	regs->regs[ 6] = (unsigned long) &frame->sf_sc;
	regs->regs[29] = (unsigned long) frame;
	regs->regs[31] = (unsigned long) sig_return;
	regs->cp0_epc = regs->regs[25] = (unsigned long) ka->sa.sa_handler;

	DEBUGP("SIG deliver (%s:%d): sp=0x%p pc=0x%lx ra=0x%lx\n",
	       current->comm, current->pid,
	       frame, regs->cp0_epc, regs->regs[31]);
	return 0;

give_sigsegv:
	force_sigsegv(signr, current);
	return -EFAULT;
}
#endif

static int setup_rt_frame(void *sig_return, struct k_sigaction * ka, struct pt_regs *regs,
	int signr, sigset_t *set, siginfo_t *info)
{
	struct rt_sigframe __user *frame;
	int err = 0;

	frame = get_sigframe(ka, regs, sizeof(*frame));
	if (!access_ok(VERIFY_WRITE, frame, sizeof (*frame)))
		goto give_sigsegv;

	/* Create siginfo.  */
	err |= copy_siginfo_to_user(&frame->rs_info, info);

	/* Create the ucontext.  */
	err |= __put_user(0, &frame->rs_uc.uc_flags);
	err |= __put_user(NULL, &frame->rs_uc.uc_link);
	err |= __put_user((void __user *)current->sas_ss_sp,
	                  &frame->rs_uc.uc_stack.ss_sp);
	err |= __put_user(sas_ss_flags(regs->regs[29]),
	                  &frame->rs_uc.uc_stack.ss_flags);
	err |= __put_user(current->sas_ss_size,
	                  &frame->rs_uc.uc_stack.ss_size);
	err |= setup_sigcontext(regs, &frame->rs_uc.uc_mcontext);
	err |= __copy_to_user(&frame->rs_uc.uc_sigmask, set, sizeof(*set));

	if (err)
		goto give_sigsegv;

	/*
	 * Arguments to signal handler:
	 *
	 *   a0 = signal number
	 *   a1 = 0 (should be cause)
	 *   a2 = pointer to ucontext
	 *
	 * $25 and c0_epc point to the signal handler, $29 points to
	 * the struct rt_sigframe.
	 */
	regs->regs[ 4] = signr;
	regs->regs[ 5] = (unsigned long) &frame->rs_info;
	regs->regs[ 6] = (unsigned long) &frame->rs_uc;
	regs->regs[29] = (unsigned long) frame;
	regs->regs[31] = (unsigned long) sig_return;
	regs->cp0_epc = regs->regs[25] = (unsigned long) ka->sa.sa_handler;

	DEBUGP("SIG deliver (%s:%d): sp=0x%p pc=0x%lx ra=0x%lx\n",
	       current->comm, current->pid,
	       frame, regs->cp0_epc, regs->regs[31]);

	return 0;

give_sigsegv:
	force_sigsegv(signr, current);
	return -EFAULT;
}

struct mips_abi mips_abi = {
#ifdef CONFIG_TRAD_SIGNALS
	.setup_frame	= setup_frame,
    .signal_return_offset = offsetof(struct mips_vdso, signal_trampoline),
#endif
	.setup_rt_frame	= setup_rt_frame,
    .rt_signal_return_offset = offsetof(struct mips_vdso, rt_signal_trampoline),
	.restart	= __NR_restart_syscall
};

static int handle_signal(unsigned long sig, siginfo_t *info,
	struct k_sigaction *ka, sigset_t *oldset, struct pt_regs *regs)
{
	int ret;
    struct mips_abi *abi = current->thread.abi;
    void *vdso = current->mm->context.vdso;

    if (regs->regs[0]) {
      switch(regs->regs[2]) {
	case ERESTART_RESTARTBLOCK:
	case ERESTARTNOHAND:
		regs->regs[2] = EINTR;
		break;
	case ERESTARTSYS:
		if (!(ka->sa.sa_flags & SA_RESTART)) {
			regs->regs[2] = EINTR;
			break;
		}
	/* fallthrough */
      case ERESTARTNOINTR:
		regs->regs[7] = regs->regs[26];
            regs->regs[2] = regs->regs[0];
            regs->cp0_epc -= 4;
	}

	regs->regs[0] = 0;		/* Don't deal with this again.  */
    }

	if (sig_uses_siginfo(ka))
      ret = abi->setup_rt_frame(vdso + abi->rt_signal_return_offset,
                                ka, regs, sig, oldset, info);
	else
      ret = abi->setup_frame(vdso + abi->signal_return_offset,
                             ka, regs, sig, oldset);

    if (ret)
      return ret;

	spin_lock_irq(&current->sighand->siglock);
	sigorsets(&current->blocked, &current->blocked, &ka->sa.sa_mask);
	if (!(ka->sa.sa_flags & SA_NODEFER))
		sigaddset(&current->blocked, sig);
	recalc_sigpending();
	spin_unlock_irq(&current->sighand->siglock);

	return ret;
}

static void do_signal(struct pt_regs *regs)
{
	struct k_sigaction ka;
	sigset_t *oldset;
	siginfo_t info;
	int signr;

	/*
	 * We want the common case to go fast, which is why we may in certain
	 * cases get here from kernel mode. Just return without doing anything
	 * if so.
	 */
	if (!user_mode(regs))
		return;

	if (test_thread_flag(TIF_RESTORE_SIGMASK))
		oldset = &current->saved_sigmask;
	else
		oldset = &current->blocked;

	signr = get_signal_to_deliver(&info, &ka, regs, NULL);
	if (signr > 0) {
		/* Whee!  Actually deliver the signal.  */
		if (handle_signal(signr, &info, &ka, oldset, regs) == 0) {
			/*
			 * A signal was successfully delivered; the saved
			 * sigmask will have been stored in the signal frame,
			 * and will be restored by sigreturn, so we can simply
			 * clear the TIF_RESTORE_SIGMASK flag.
			 */
			if (test_thread_flag(TIF_RESTORE_SIGMASK))
				clear_thread_flag(TIF_RESTORE_SIGMASK);
		}

		return;
	}

	if (regs->regs[0]) {
		switch (regs->regs[2]) {
		case ERESTARTNOHAND:
		case ERESTARTSYS:
		case ERESTARTNOINTR:
			regs->regs[2] = regs->regs[0];
			regs->regs[7] = regs->regs[26];
			regs->cp0_epc -= 4;
			break;
		case ERESTART_RESTARTBLOCK:
			regs->regs[2] = current->thread.abi->restart;
			regs->regs[7] = regs->regs[26];
			regs->cp0_epc -= 4;
			break;
		}
		regs->regs[0] = 0;	/* Don't deal with this again.  */
	}

	/*
	 * If there's no signal to deliver, we just put the saved sigmask
	 * back
	 */
	if (test_thread_flag(TIF_RESTORE_SIGMASK)) {
		clear_thread_flag(TIF_RESTORE_SIGMASK);
		sigprocmask(SIG_SETMASK, &current->saved_sigmask, NULL);
	}
}

/*
 * notification of userspace execution resumption
 * - triggered by the TIF_WORK_MASK flags
 */
asmlinkage void do_notify_resume(struct pt_regs *regs, void *unused,
	__u32 thread_info_flags)
{
	local_irq_enable();

	/* deal with pending signal delivery */
	if (thread_info_flags & (_TIF_SIGPENDING | _TIF_RESTORE_SIGMASK))
		do_signal(regs);
}
