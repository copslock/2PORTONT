
/********************************************
rexp2.c
copyright 1991, Michael D. Brennan

This is a source file for mawk, an implementation of
the AWK programming language.

Mawk is distributed without warranty under the terms of
the GNU General Public License, version 2, 1991.
********************************************/

/*$Log: rexp2.c,v $
/*Revision 1.1.1.1  2003/08/18 05:41:27  kaohj
/*initial import into CVS
/*
 * Revision 1.3  1993/07/24  17:55:12  mike
 * more cleanup
 *
 * Revision 1.2	 1993/07/23  13:21:44  mike
 * cleanup rexp code
 *
 * Revision 1.1.1.1  1993/07/03	 18:58:28  mike
 * move source to cvs
 *
 * Revision 3.8	 1992/12/24  00:36:44  mike
 * fixed major bozo for LMDOS when growing stack
 * fixed potential LMDOS bozo with M_STR+U_ON+END_ON
 * fixed minor bug in M_CLASS+U_ON+END_ON
 *
 * Revision 3.7	 92/01/21  17:33:15  brennan
 * added some casts so that character classes work with signed chars
 *
 * Revision 3.6	 91/10/29  10:54:03  brennan
 * SIZE_T
 *
 * Revision 3.5	 91/08/13  09:10:15  brennan
 * VERSION .9994
 *
 * Revision 3.4	 91/08/08  07:53:34  brennan
 * work around for turboC realloc() bug
 *
 * Revision 3.4	 91/08/07  07:10:47  brennan
 * work around for TurboC realloc() bug
 *
 * Revision 3.3	 91/08/04  15:45:57  brennan
 * minor change for large model dos
 *
 * Revision 3.2	 91/06/10  16:18:14  brennan
 * changes for V7
 *
 * Revision 3.1	 91/06/07  10:33:25  brennan
 * VERSION 0.995
 *
 * Revision 1.8	 91/06/05  09:01:33  brennan
 * changes to RE_new_run_stack
 *
 * Revision 1.7	 91/05/31  10:56:02  brennan
 * stack_empty hack for DOS large model
 *
*/



/*  test a string against a machine   */

#include "rexp.h"

#define	 STACKGROWTH	16

#ifdef	DEBUG
static RT_STATE *PROTO(slow_push, (RT_STATE *, STATE *, char *, int)) ;
#endif


RT_STATE *RE_run_stack_base ;
RT_STATE *RE_run_stack_limit ;

/* Large model DOS segment arithemetic breaks the current stack.
   This hack fixes it without rewriting the whole thing, 5/31/91 */
RT_STATE *RE_run_stack_empty ;

void
RE_run_stack_init()
{
   if (!RE_run_stack_base)
   {
      RE_run_stack_base = (RT_STATE *)
      RE_malloc(sizeof(RT_STATE) * STACKGROWTH) ;
      RE_run_stack_limit = RE_run_stack_base + STACKGROWTH ;
      RE_run_stack_empty = RE_run_stack_base - 1 ;
   }
}

/* sometimes during REmatch(), this stack can grow pretty large.
   In real life cases, the back tracking usually fails. Some
   work is needed here to improve the algorithm.
   I.e., figure out how not to stack useless paths.
*/

RT_STATE *
RE_new_run_stack()
{
   int oldsize = RE_run_stack_limit - RE_run_stack_base ;
   int newsize = oldsize + STACKGROWTH ;

#ifdef	LMDOS			/* large model DOS */
   /* have to worry about overflow on multiplication (ugh) */
   if (newsize >= 4096)	 RE_run_stack_base = (RT_STATE *) 0 ;
   else
#endif

      RE_run_stack_base = (RT_STATE *) realloc(RE_run_stack_base,
					newsize * sizeof(RT_STATE)) ;

   if (!RE_run_stack_base)
   {
      fprintf(stderr, "out of memory for RE run time stack\n") ;
      /* this is pretty unusual, I've only seen it happen on
       weird input to REmatch() under 16bit DOS , the same
       situation worked easily on 32bit machine.  */
      exit(100) ;
   }

   RE_run_stack_limit = RE_run_stack_base + newsize ;
   RE_run_stack_empty = RE_run_stack_base - 1 ;

   /* return the new stackp */
   return RE_run_stack_base + oldsize ;
}

#ifdef	DEBUG
static RT_STATE *
slow_push(sp, m, s, u)
   RT_STATE *sp ;
   STATE *m ;
   char *s ;
   int u ;
{
   if (sp == RE_run_stack_limit)  sp = RE_new_run_stack() ;
   sp->m = m ; sp->s = s ; sp->u = u ;
   return sp ;
}
#endif

#ifdef	 DEBUG
#define	 push(mx,sx,ux)	  stackp = slow_push(++stackp, mx, sx, ux)
#else
#define	 push(mx,sx,ux)	  if (++stackp == RE_run_stack_limit)\
				stackp = RE_new_run_stack() ;\
stackp->m=(mx);stackp->s=(sx);stackp->u=(ux)
#endif


#define	  CASE_UANY(x)	case  x + U_OFF :  case	 x + U_ON

/* test if str ~ /machine/
*/

int
REtest(str, machine)
   char *str ;
   PTR machine ;
{
   register STATE *m = (STATE *) machine ;
   register char *s = str ;
   register RT_STATE *stackp ;
   int u_flag ;
   char *str_end ;
   int t ;			 /*convenient temps */
   STATE *tm ;

   /* handle the easy case quickly */
   if ((m + 1)->type == M_ACCEPT && m->type == M_STR)
      return str_str(s, m->data.str, m->len) != (char *) 0 ;
   else
   {
      u_flag = U_ON ; str_end = (char *) 0 ;
      stackp = RE_run_stack_empty ;
      goto reswitch ;
   }

refill :
   if (stackp == RE_run_stack_empty)  return 0 ;
   m = stackp->m ;
   s = stackp->s ;
   u_flag = stackp--->u ;


reswitch  :

   switch (m->type + u_flag)
   {
      case M_STR + U_OFF + END_OFF:
	 if (strncmp(s, m->data.str, m->len))  goto refill ;
	 s += m->len ;	m++ ;
	 goto reswitch ;

      case M_STR + U_OFF + END_ON:
	 if (strcmp(s, m->data.str))  goto refill ;
	 s += m->len ;	m++ ;
	 goto reswitch ;

      case M_STR + U_ON + END_OFF:
	 if (!(s = str_str(s, m->data.str, m->len)))  goto refill ;
	 push(m, s + 1, U_ON) ;
	 s += m->len ; m++ ; u_flag = U_OFF ;
	 goto reswitch ;

      case M_STR + U_ON + END_ON:
	 if (!str_end)	str_end = s + strlen(s) ;
	 t = (str_end - s) - m->len ;
	 if (t < 0 || memcmp(s + t, m->data.str, m->len))
	    goto refill ;
	 s = str_end ; m++ ; u_flag = U_OFF ;
	 goto reswitch ;

      case M_CLASS + U_OFF + END_OFF:
	 if (!ison(*m->data.bvp, s[0]))	 goto refill ;
	 s++ ; m++ ;
	 goto reswitch ;

      case M_CLASS + U_OFF + END_ON:
	 if (s[1] || !ison(*m->data.bvp, s[0]))	 goto refill ;
	 s++ ; m++ ;
	 goto reswitch ;

      case M_CLASS + U_ON + END_OFF:
	 while (!ison(*m->data.bvp, s[0]))
	 {
	    if (s[0] == 0)  goto refill ;
	    else  s++ ;
	 }
	 s++ ;
	 push(m, s, U_ON) ;
	 m++ ; u_flag = U_OFF ;
	 goto reswitch ;

      case M_CLASS + U_ON + END_ON:
	 if (!str_end)	str_end = s + strlen(s) ;
	 if (s[0] == 0 || !ison(*m->data.bvp, str_end[-1]))
	    goto refill ;
	 s = str_end ; m++ ; u_flag = U_OFF ;
	 goto reswitch ;

      case M_ANY + U_OFF + END_OFF:
	 if (s[0] == 0)	 goto refill ;
	 s++ ; m++ ;
	 goto reswitch ;

      case M_ANY + U_OFF + END_ON:
	 if (s[0] == 0 || s[1] != 0)  goto refill ;
	 s++ ; m++ ;
	 goto reswitch ;

      case M_ANY + U_ON + END_OFF:
	 if (s[0] == 0)	 goto refill ;
	 s++ ;
	 push(m, s, U_ON) ;
	 m++ ; u_flag = U_OFF ;
	 goto reswitch ;

      case M_ANY + U_ON + END_ON:
	 if (s[0] == 0)	 goto refill ;
	 if (!str_end)	str_end = s + strlen(s) ;
	 s = str_end ; m++ ; u_flag = U_OFF ;
	 goto reswitch ;

      case M_START + U_OFF + END_OFF:
      case M_START + U_ON + END_OFF:
	 if (s != str)	goto refill ;
	 m++ ;	u_flag = U_OFF ;
	 goto reswitch ;

      case M_START + U_OFF + END_ON:
      case M_START + U_ON + END_ON:
	 if (s != str || s[0] != 0)  goto refill ;
	 m++ ; u_flag = U_OFF ;
	 goto reswitch ;

      case M_END + U_OFF:
	 if (s[0] != 0)	 goto refill ;
	 m++ ; goto reswitch ;

      case M_END + U_ON:
	 s += strlen(s) ;
	 m++ ; u_flag = U_OFF ;
	 goto reswitch ;

       CASE_UANY(M_U):
	 u_flag = U_ON ; m++ ;
	 goto reswitch ;

       CASE_UANY(M_1J):
	 m += m->data.jump ;
	 goto reswitch ;

       CASE_UANY(M_2JA):	/* take the non jump branch */
	 /* don't stack an ACCEPT */
	 if ((tm = m + m->data.jump)->type == M_ACCEPT)	 return 1 ;
	 push(tm, s, u_flag) ;
	 m++ ;
	 goto reswitch ;

       CASE_UANY(M_2JB):	/* take the jump branch */
	 /* don't stack an ACCEPT */
	 if ((tm = m + 1)->type == M_ACCEPT)  return 1 ;
	 push(tm, s, u_flag) ;
	 m += m->data.jump ;
	 goto reswitch ;

       CASE_UANY(M_ACCEPT):
	 return 1 ;

      default:
	 RE_panic("unexpected case in REtest") ;
   }
}



#ifdef	MAWK

char *
is_string_split(p, lenp)
   register STATE *p ;
   unsigned *lenp ;
{
   if (p[0].type == M_STR && p[1].type == M_ACCEPT)
   {
      *lenp = p->len ;
      return p->data.str ;
   }
   else	 return (char *) 0 ;
}
#else /* mawk provides its own str_str */

char *
str_str(target, key, klen)
   register char *target ;
   register char *key ;
   unsigned klen ;
{
   int c = key[0] ;

   switch (klen)
   {
      case 0:
	 return (char *) 0 ;

      case 1:
	 return strchr(target, c) ;

      case 2:
	 {
	    int c1 = key[1] ;

	    while (target = strchr(target, c))
	    {
	       if (target[1] == c1)  return target ;
	       else  target++ ;
	    }
	    break ;
	 }

      default:
	 klen-- ; key++ ;
	 while (target = strchr(target, c))
	 {
	    if (memcmp(target + 1, key, klen) == 0)  return target ;
	    else  target++ ;
	 }
	 break ;
   }
   return (char *) 0 ;
}


#endif /* MAWK */
