/* $Header: /usr/local/dslrepos/uClinux-dist/user/tcsh/sh.dol.c,v 1.1.1.1 2003/08/18 05:40:15 kaohj Exp $ */
/*
 * sh.dol.c: Variable substitutions
 */
/*-
 * Copyright (c) 1980, 1991 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include "sh.h"

RCSID("$Id: sh.dol.c,v 1.1.1.1 2003/08/18 05:40:15 kaohj Exp $")

/*
 * C shell
 */

/*
 * These routines perform variable substitution and quoting via ' and ".
 * To this point these constructs have been preserved in the divided
 * input words.  Here we expand variables and turn quoting via ' and " into
 * QUOTE bits on characters (which prevent further interpretation).
 * If the `:q' modifier was applied during history expansion, then
 * some QUOTEing may have occurred already, so we dont "trim()" here.
 */

static int Dpeekc, Dpeekrd;	/* Peeks for DgetC and Dreadc */
static Char *Dcp, **Dvp;	/* Input vector for Dreadc */

#define	DEOF	-1

#define	unDgetC(c)	Dpeekc = c

#define QUOTES		(_QF|_QB|_ESC)	/* \ ' " ` */

/*
 * The following variables give the information about the current
 * $ expansion, recording the current word position, the remaining
 * words within this expansion, the count of remaining words, and the
 * information about any : modifier which is being applied.
 */
#define MAXWLEN (BUFSIZE - 4)
#ifndef COMPAT
#define MAXMOD MAXWLEN		/* This cannot overflow	*/
#endif /* COMPAT */
static Char *dolp;		/* Remaining chars from this word */
static Char **dolnxt;		/* Further words */
static int dolcnt;		/* Count of further words */
#ifdef COMPAT
static Char dolmod;		/* : modifier character */
#else
static Char dolmod[MAXMOD];	/* : modifier character */
static int dolnmod;		/* Number of modifiers */
#endif /* COMPAT */
static int dolmcnt;		/* :gx -> 10000, else 1 */
static int dolwcnt;		/* :ax -> 10000, else 1 */

static	void	 Dfix2		__P((Char **));
static	Char 	*Dpack		__P((Char *, Char *));
static	int	 Dword		__P((void));
static	void	 dolerror	__P((Char *));
static	int	 DgetC		__P((int));
static	void	 Dgetdol	__P((void));
static	void	 fixDolMod	__P((void));
static	void	 setDolp	__P((Char *));
static	void	 unDredc	__P((int));
static	int	 Dredc		__P((void));
static	void	 Dtestq		__P((int));

/*
 * Fix up the $ expansions and quotations in the
 * argument list to command t.
 */
void
Dfix(t)
    register struct command *t;
{
    register Char **pp;
    register Char *p;

    if (noexec)
	return;
    /* Note that t_dcom isn't trimmed thus !...:q's aren't lost */
    for (pp = t->t_dcom; (p = *pp++) != NULL;) {
	for (; *p; p++) {
#ifdef DSPMBYTE
	    if (Ismbyte1(*p) && *(p + 1))
		p ++;
	    else
#endif DSPMBYTE
	    if (cmap(*p, _DOL | QUOTES)) {	/* $, \, ', ", ` */
		Dfix2(t->t_dcom);	/* found one */
		blkfree(t->t_dcom);
		t->t_dcom = gargv;
		gargv = 0;
		return;
	    }
	}
    }
}

/*
 * $ substitute one word, for i/o redirection
 */
Char   *
Dfix1(cp)
    register Char *cp;
{
    Char   *Dv[2];

    if (noexec)
	return (0);
    Dv[0] = cp;
    Dv[1] = NULL;
    Dfix2(Dv);
    if (gargc != 1) {
	setname(short2str(cp));
	stderror(ERR_NAME | ERR_AMBIG);
    }
    cp = Strsave(gargv[0]);
    blkfree(gargv), gargv = 0;
    return (cp);
}

/*
 * Subroutine to do actual fixing after state initialization.
 */
static void
Dfix2(v)
    Char  **v;
{
    ginit();			/* Initialize glob's area pointers */
    Dvp = v;
    Dcp = STRNULL;		/* Setup input vector for Dreadc */
    unDgetC(0);
    unDredc(0);			/* Clear out any old peeks (at error) */
    dolp = 0;
    dolcnt = 0;			/* Clear out residual $ expands (...) */
    while (Dword())
	continue;
}

/*
 * Pack up more characters in this word
 */
static Char *
Dpack(wbuf, wp)
    Char   *wbuf, *wp;
{
    register int c;
    register int i = MAXWLEN - (int) (wp - wbuf);
#if defined(DSPMBYTE)
    int mbytepos = 1;
#endif /* DSPMBYTE */

    for (;;) {
	c = DgetC(DODOL);
#if defined(DSPMBYTE)
	if (mbytepos == 1 && Ismbyte1(c)) {
	    /* An MB1 byte that may be followed by a MB2 byte */
	    mbytepos = 2;
	}
	else {
	    /* check if MB1 byte followed by an MB2 byte */
	    if (mbytepos == 2 && Ismbyte2(c)) {
		/* MB1 + MB2 make the character */
		mbytepos = 1; /* reset */
		goto mbyteskip;
	    }
	    mbytepos = 1; /* reset */
	    /* wasn't followed, so the two bytes make two characters */
	}
#endif /* DSPMBYTE */
	if (c == '\\') {
	    c = DgetC(0);
	    if (c == DEOF) {
		unDredc(c);
		*wp = 0;
		Gcat(STRNULL, wbuf);
		return (NULL);
	    }
	    if (c == '\n')
		c = ' ';
	    else
		c |= QUOTE;
	}
	if (c == DEOF) {
	    unDredc(c);
	    *wp = 0;
	    Gcat(STRNULL, wbuf);
	    return (NULL);
	}
	if (cmap(c, _SP | _NL | _QF | _QB)) {	/* sp \t\n'"` */
	    unDgetC(c);
	    if (cmap(c, QUOTES))
		return (wp);
	    *wp++ = 0;
	    Gcat(STRNULL, wbuf);
	    return (NULL);
	}
#if defined(DSPMBYTE)
mbyteskip:
#endif /* DSPMBYTE */
	if (--i <= 0)
	    stderror(ERR_WTOOLONG);
	*wp++ = (Char) c;
    }
}

/*
 * Get a word.  This routine is analogous to the routine
 * word() in sh.lex.c for the main lexical input.  One difference
 * here is that we don't get a newline to terminate our expansion.
 * Rather, DgetC will return a DEOF when we hit the end-of-input.
 */
static int
Dword()
{
    register int c, c1;
    Char    wbuf[BUFSIZE];
    register Char *wp = wbuf;
    register int i = MAXWLEN;
    register bool dolflg;
    bool    sofar = 0, done = 0;

    while (!done) {
	done = 1;
	c = DgetC(DODOL);
	switch (c) {

	case DEOF:
	    if (sofar == 0)
		return (0);
	    /* finish this word and catch the code above the next time */
	    unDredc(c);
	    /*FALLTHROUGH*/

	case '\n':
	    *wp = 0;
	    Gcat(STRNULL, wbuf);
	    return (1);

	case ' ':
	case '\t':
	    done = 0;
	    break;

	case '`':
	    /* We preserve ` quotations which are done yet later */
	    *wp++ = (Char) c, --i;
	    /*FALLTHROUGH*/
	case '\'':
	case '"':
	    /*
	     * Note that DgetC never returns a QUOTES character from an
	     * expansion, so only true input quotes will get us here or out.
	     */
	    c1 = c;
	    dolflg = c1 == '"' ? DODOL : 0;
	    for (;;) {
		c = DgetC(dolflg);
		if (c == c1)
		    break;
		if (c == '\n' || c == DEOF)
		    stderror(ERR_UNMATCHED, c1);
		if ((c & (QUOTE | TRIM)) == ('\n' | QUOTE)) {
		    if ((wp[-1] & TRIM) == '\\')
			--wp;
		    ++i;
		}
		if (--i <= 0)
		    stderror(ERR_WTOOLONG);
		switch (c1) {

		case '"':
		    /*
		     * Leave any `s alone for later. Other chars are all
		     * quoted, thus `...` can tell it was within "...".
		     */
		    *wp++ = c == '`' ? '`' : c | QUOTE;
		    break;

		case '\'':
		    /* Prevent all further interpretation */
		    *wp++ = c | QUOTE;
		    break;

		case '`':
		    /* Leave all text alone for later */
		    *wp++ = (Char) c;
		    break;

		default:
		    break;
		}
	    }
	    if (c1 == '`')
		*wp++ = '`' /* i--; eliminated */;
	    sofar = 1;
	    if ((wp = Dpack(wbuf, wp)) == NULL)
		return (1);
	    else {
#ifdef masscomp
    /*
     * Avoid a nasty message from the RTU 4.1A & RTU 5.0 compiler concerning
     * the "overuse of registers". According to the compiler release notes,
     * incorrect code may be produced unless the offending expression is
     * rewritten. Therefore, we can't just ignore it, DAS DEC-90.
     */
		i = MAXWLEN;
		i -= (int) (wp - wbuf);
#else /* !masscomp */
		i = MAXWLEN - (int) (wp - wbuf);
#endif /* masscomp */
		done = 0;
	    }
	    break;

	case '\\':
	    c = DgetC(0);	/* No $ subst! */
	    if (c == '\n' || c == DEOF) {
		done = 0;
		break;
	    }
	    c |= QUOTE;
	    break;

	default:
	    break;
	}
	if (done) {
	    unDgetC(c);
	    sofar = 1;
	    if ((wp = Dpack(wbuf, wp)) == NULL)
		return (1);
	    else {
#ifdef masscomp
    /*
     * Avoid a nasty message from the RTU 4.1A & RTU 5.0 compiler concerning
     * the "overuse of registers". According to the compiler release notes,
     * incorrect code may be produced unless the offending expression is
     * rewritten. Therefore, we can't just ignore it, DAS DEC-90.
     */
		i = MAXWLEN;
		i -= (int) (wp - wbuf);
#else /* !masscomp */
		i = MAXWLEN - (int) (wp - wbuf);
#endif /* masscomp */
		done = 0;
	    }
	}
    }
    /* Really NOTREACHED */
    return (0);
}


/*
 * Get a character, performing $ substitution unless flag is 0.
 * Any QUOTES character which is returned from a $ expansion is
 * QUOTEd so that it will not be recognized above.
 */
static int
DgetC(flag)
    register int flag;
{
    register int c;

top:
    if ((c = Dpeekc) != 0) {
	Dpeekc = 0;
	return (c);
    }
    if (lap) {
	c = *lap++ & (QUOTE | TRIM);
	if (c == 0) {
	    lap = 0;
	    goto top;
	}
quotspec:
	if (cmap(c, QUOTES))
	    return (c | QUOTE);
	return (c);
    }
    if (dolp) {
	if ((c = *dolp++ & (QUOTE | TRIM)) != 0)
	    goto quotspec;
	if (dolcnt > 0) {
	    setDolp(*dolnxt++);
	    --dolcnt;
	    return (' ');
	}
	dolp = 0;
    }
    if (dolcnt > 0) {
	setDolp(*dolnxt++);
	--dolcnt;
	goto top;
    }
    c = Dredc();
    if (c == '$' && flag) {
	Dgetdol();
	goto top;
    }
    return (c);
}

static Char *nulvec[] = { NULL };
static struct varent nulargv = {nulvec, STRargv, VAR_READWRITE, 
				{ NULL, NULL, NULL }, 0 };

static void
dolerror(s)
    Char   *s;
{
    setname(short2str(s));
    stderror(ERR_NAME | ERR_RANGE);
}

/*
 * Handle the multitudinous $ expansion forms.
 * Ugh.
 */
static void
Dgetdol()
{
    register Char *np;
    register struct varent *vp = NULL;
    Char    name[4 * MAXVARLEN + 1];
    int     c, sc;
    int     subscr = 0, lwb = 1, upb = 0;
    bool    dimen = 0, bitset = 0, length = 0;
    char    tnp;
    Char    wbuf[BUFSIZE];
    static Char *dolbang = NULL;

#ifdef COMPAT
    dolmod = dolmcnt = dolwcnt = 0;
#else
    dolnmod = dolmcnt = dolwcnt = 0;
#endif /* COMPAT */
    c = sc = DgetC(0);
    if (c == '{')
	c = DgetC(0);		/* sc is { to take } later */
    if ((c & TRIM) == '#')
	dimen++, c = DgetC(0);	/* $# takes dimension */
    else if (c == '?')
	bitset++, c = DgetC(0);	/* $? tests existence */
    else if (c == '%')
	length++, c = DgetC(0); /* $% returns length in chars */
    switch (c) {

    case '!':
	if (dimen || bitset || length)
	    stderror(ERR_SYNTAX);
	if (backpid != 0) {
	    if (dolbang) 
		xfree((ptr_t) dolbang);
	    setDolp(dolbang = putn(backpid));
	}
	goto eatbrac;

    case '$':
	if (dimen || bitset || length)
	    stderror(ERR_SYNTAX);
	setDolp(doldol);
	goto eatbrac;

#ifdef COHERENT
    /* Coherent compiler doesn't allow case-labels that are not 
       constant-expressions */
#ifdef SHORT_STRINGS
    case 0100074:
#else /* !SHORT_STRINGS */
    case 0274:
#endif
#else /* !COHERENT */
    case '<'|QUOTE:
#endif
	if (bitset)
	    stderror(ERR_NOTALLOWED, "$?<");
	if (dimen)
	    stderror(ERR_NOTALLOWED, "$#<");
	if (length)
	    stderror(ERR_NOTALLOWED, "$%<");
	{
#ifdef BSDSIGS
	    sigmask_t omask = sigsetmask(sigblock(0) & ~sigmask(SIGINT));
#else /* !BSDSIGS */
	    (void) sigrelse(SIGINT);
#endif /* BSDSIGS */
	    for (np = wbuf; force_read(OLDSTD, &tnp, 1) == 1; np++) {
		*np = (unsigned char) tnp;
		if (np >= &wbuf[BUFSIZE - 1])
		    stderror(ERR_LTOOLONG);
		if (tnp == '\n')
		    break;
	    }
	    *np = 0;
#ifdef BSDSIGS
	    (void) sigsetmask(omask);
#else /* !BSDSIGS */
	    (void) sighold(SIGINT);
#endif /* BSDSIGS */
	}

#ifdef COMPAT
	/*
	 * KLUDGE: dolmod is set here because it will cause setDolp to call
	 * domod and thus to copy wbuf. Otherwise setDolp would use it
	 * directly. If we saved it ourselves, no one would know when to free
	 * it. The actual function of the 'q' causes filename expansion not to
	 * be done on the interpolated value.
	 */
	/* 
	 * If we do that, then other modifiers don't work.
	 * in addition, let the user specify :q if wanted
	 * [christos]
	 */
/*old*/	dolmod = 'q';
/*new*/	dolmod[dolnmod++] = 'q';
	dolmcnt = 10000;
#endif /* COMPAT */

	fixDolMod();
	setDolp(wbuf);
	goto eatbrac;

    case '*':
	(void) Strcpy(name, STRargv);
	vp = adrof(STRargv);
	subscr = -1;		/* Prevent eating [...] */
	break;

    case DEOF:
    case '\n':
	np = dimen ? STRargv : (bitset ? STRstatus : NULL);
	if (np) {
	    bitset = 0;
	    (void) Strcpy(name, np);
	    vp = adrof(np);
	    subscr = -1;		/* Prevent eating [...] */
	    unDredc(c);
	    break;
	}
	else
	    stderror(ERR_SYNTAX);
	/*NOTREACHED*/

    default:
	np = name;
	if (Isdigit(c)) {
	    if (dimen)
		stderror(ERR_NOTALLOWED, "$#<num>");
	    subscr = 0;
	    do {
		subscr = subscr * 10 + c - '0';
		c = DgetC(0);
	    } while (Isdigit(c));
	    unDredc(c);
	    if (subscr < 0) {
		dolerror(vp->v_name);
		return;
	    }
	    if (subscr == 0) {
		if (bitset) {
		    dolp = dolzero ? STR1 : STR0;
		    goto eatbrac;
		}
		if (ffile == 0)
		    stderror(ERR_DOLZERO);
		if (length) {
		    Char *cp;
		    length = Strlen(ffile);
		    cp = putn(length);
		    addla(cp);
		    xfree((ptr_t) cp);
		}
		else {
		    fixDolMod();
		    setDolp(ffile);
		}
		goto eatbrac;
	    }
#if 0
	    if (bitset)
		stderror(ERR_NOTALLOWED, "$?<num>");
	    if (length)
		stderror(ERR_NOTALLOWED, "$%<num>");
#endif
	    vp = adrof(STRargv);
	    if (vp == 0) {
		vp = &nulargv;
		goto eatmod;
	    }
	    break;
	}
	if (!alnum(c)) {
	    np = dimen ? STRargv : (bitset ? STRstatus : NULL);
	    if (np) {
		bitset = 0;
		(void) Strcpy(name, np);
		vp = adrof(np);
		subscr = -1;		/* Prevent eating [...] */
		unDredc(c);
		break;
	    }
	    else
		stderror(ERR_VARALNUM);
	}
	for (;;) {
	    *np++ = (Char) c;
	    c = DgetC(0);
	    if (!alnum(c))
		break;
	    if (np >= &name[MAXVARLEN])
		stderror(ERR_VARTOOLONG);
	}
	*np++ = 0;
	unDredc(c);
	vp = adrof(name);
    }
    if (bitset) {
	dolp = (vp || getenv(short2str(name))) ? STR1 : STR0;
	goto eatbrac;
    }
    if (vp == 0) {
	np = str2short(getenv(short2str(name)));
	if (np) {
	    fixDolMod();
	    setDolp(np);
	    goto eatbrac;
	}
	udvar(name);
	/* NOTREACHED */
    }
    c = DgetC(0);
    upb = blklen(vp->vec);
    if (dimen == 0 && subscr == 0 && c == '[') {
	np = name;
	for (;;) {
	    c = DgetC(DODOL);	/* Allow $ expand within [ ] */
	    if (c == ']')
		break;
	    if (c == '\n' || c == DEOF)
		stderror(ERR_INCBR);
	    if (np >= &name[sizeof(name) / sizeof(Char) - 2])
		stderror(ERR_VARTOOLONG);
	    *np++ = (Char) c;
	}
	*np = 0, np = name;
	if (dolp || dolcnt)	/* $ exp must end before ] */
	    stderror(ERR_EXPORD);
	if (!*np)
	    stderror(ERR_SYNTAX);
	if (Isdigit(*np)) {
	    int     i;

	    for (i = 0; Isdigit(*np); i = i * 10 + *np++ - '0')
		continue;
	    if ((i < 0 || i > upb) && !any("-*", *np)) {
		dolerror(vp->v_name);
		return;
	    }
	    lwb = i;
	    if (!*np)
		upb = lwb, np = STRstar;
	}
	if (*np == '*')
	    np++;
	else if (*np != '-')
	    stderror(ERR_MISSING, '-');
	else {
	    register int i = upb;

	    np++;
	    if (Isdigit(*np)) {
		i = 0;
		while (Isdigit(*np))
		    i = i * 10 + *np++ - '0';
		if (i < 0 || i > upb) {
		    dolerror(vp->v_name);
		    return;
		}
	    }
	    if (i < lwb)
		upb = lwb - 1;
	    else
		upb = i;
	}
	if (lwb == 0) {
	    if (upb != 0) {
		dolerror(vp->v_name);
		return;
	    }
	    upb = -1;
	}
	if (*np)
	    stderror(ERR_SYNTAX);
    }
    else {
	if (subscr > 0) {
	    if (subscr > upb)
		lwb = 1, upb = 0;
	    else
		lwb = upb = subscr;
	}
	unDredc(c);
    }
    if (dimen) {
	Char   *cp = putn(upb - lwb + 1);

	/* this is a kludge. It prevents Dgetdol() from */
	/* pushing erroneous ${#<error> values into the labuf. */
	if (sc == '{') {
	    c = Dredc();
	    if (c != '}')
	    {
		xfree((ptr_t) cp);
		stderror(ERR_MISSING, '}');
	        return;
	    }
	    unDredc(c);
	}
	addla(cp);
	xfree((ptr_t) cp);
    }
    else if (length) {
	int i;
	Char   *cp;
	for (i = lwb - 1, length = 0; i < upb; i++)
	    length += Strlen(vp->vec[i]);
#ifdef notdef
	/* We don't want that, since we can always compute it by adding $#xxx */
	length += i - 1;	/* Add the number of spaces in */
#endif
	cp = putn(length);
	addla(cp);
	xfree((ptr_t) cp);
    }
    else {
eatmod:
	fixDolMod();
	dolnxt = &vp->vec[lwb - 1];
	dolcnt = upb - lwb + 1;
    }
eatbrac:
    if (sc == '{') {
	c = Dredc();
	if (c != '}')
	    stderror(ERR_MISSING, '}');
    }
}

static void
fixDolMod()
{
    register int c;

    c = DgetC(0);
    if (c == ':') {
#ifndef COMPAT
	do {
#endif /* COMPAT */
	    c = DgetC(0), dolmcnt = 1, dolwcnt = 1;
	    if (c == 'g' || c == 'a') {
		if (c == 'g')
		    dolmcnt = 10000;
		else
		    dolwcnt = 10000;
		c = DgetC(0);
	    }
	    if ((c == 'g' && dolmcnt != 10000) || 
		(c == 'a' && dolwcnt != 10000)) {
		if (c == 'g')
		    dolmcnt = 10000;
		else
		    dolwcnt = 10000;
		c = DgetC(0); 
	    }

	    if (c == 's') {	/* [eichin:19910926.0755EST] */
		int delimcnt = 2;
		int delim = DgetC(0);
		dolmod[dolnmod++] = (Char) c;
		dolmod[dolnmod++] = (Char) delim;
		
		if (!delim || letter(delim)
		    || Isdigit(delim) || any(" \t\n", delim)) {
		    seterror(ERR_BADSUBST);
		    break;
		}	
		while ((c = DgetC(0)) != (-1)) {
		    dolmod[dolnmod++] = (Char) c;
		    if(c == delim) delimcnt--;
		    if(!delimcnt) break;
		}
		if(delimcnt) {
		    seterror(ERR_BADSUBST);
		    break;
		}
		continue;
	    }
	    if (!any("luhtrqxes", c))
		stderror(ERR_BADMOD, c);
#ifndef COMPAT
	    dolmod[dolnmod++] = (Char) c;
#else
	    dolmod = (Char) c;
#endif /* COMPAT */
	    if (c == 'q')
		dolmcnt = 10000;
#ifndef COMPAT
	}
	while ((c = DgetC(0)) == ':');
	unDredc(c);
#endif /* COMPAT */
    }
    else
	unDredc(c);
}

static void
setDolp(cp)
    register Char *cp;
{
    register Char *dp;
#ifndef COMPAT
    int i;
#endif /* COMPAT */

#ifdef COMPAT
    if (dolmod == 0 || dolmcnt == 0) {
#else
    if (dolnmod == 0 || dolmcnt == 0) {
#endif /* COMPAT */
	dolp = cp;
	return;
    }
#ifdef COMPAT
    dp = domod(cp, dolmod);
#else
    dp = cp = Strsave(cp);
    for (i = 0; i < dolnmod; i++) {
	/* handle s// [eichin:19910926.0510EST] */
	if(dolmod[i] == 's') {
	    int delim;
	    Char *lhsub, *rhsub, *np;
	    size_t lhlen = 0, rhlen = 0;
	    int didmod = 0;
		
	    delim = dolmod[++i];
	    if (!delim || letter(delim)
		|| Isdigit(delim) || any(" \t\n", delim)) {
		seterror(ERR_BADSUBST);
		break;
	    }
	    lhsub = &dolmod[++i];
	    while(dolmod[i] != delim && dolmod[++i]) {
		lhlen++;
	    }
	    dolmod[i] = 0;
	    rhsub = &dolmod[++i];
	    while(dolmod[i] != delim && dolmod[++i]) {
		rhlen++;
	    }
	    dolmod[i] = 0;

	    do {
		strip(lhsub);
		strip(cp);
		dp = Strstr(cp, lhsub);
		if (dp) {
		    np = (Char *) xmalloc((size_t)
					  ((Strlen(cp) + 1 - lhlen + rhlen) *
					  sizeof(Char)));
		    (void) Strncpy(np, cp, (size_t) (dp - cp));
		    (void) Strcpy(np + (dp - cp), rhsub);
		    (void) Strcpy(np + (dp - cp) + rhlen, dp + lhlen);

		    xfree((ptr_t) cp);
		    dp = cp = np;
		    didmod = 1;
		} else {
		    /* should this do a seterror? */
		    break;
		}
	    }
	    while (dolwcnt == 10000);
	    /*
	     * restore dolmod for additional words
	     */
	    dolmod[i] = rhsub[-1] = (Char) delim;
	    if (didmod)
		dolmcnt--;
#ifdef notdef
	    else
		break;
#endif
        } else {
	    int didmod = 0;

	    do {
		if ((dp = domod(cp, dolmod[i])) != NULL) {
		    didmod = 1;
		    if (Strcmp(cp, dp) == 0) {
			xfree((ptr_t) cp);
			cp = dp;
			break;
		    }
		    else {
			xfree((ptr_t) cp);
			cp = dp;
		    }
		}
		else
		    break;
	    }
	    while (dolwcnt == 10000);
	    dp = cp;
	    if (didmod)
		dolmcnt--;
#ifdef notdef
	    else
		break;
#endif
	}
    }
#endif /* COMPAT */

    if (dp) {
#ifdef COMPAT
	dolmcnt--;
#endif /* COMPAT */
	addla(dp);
	xfree((ptr_t) dp);
    }
#ifndef COMPAT
    else
	addla(cp);
#endif /* COMPAT */

    dolp = STRNULL;
    if (seterr)
	stderror(ERR_OLD);
}

static void
unDredc(c)
    int     c;
{

    Dpeekrd = c;
}

static int
Dredc()
{
    register int c;

    if ((c = Dpeekrd) != 0) {
	Dpeekrd = 0;
	return (c);
    }
    if (Dcp && (c = *Dcp++))
	return (c & (QUOTE | TRIM));
    if (*Dvp == 0) {
	Dcp = 0;
	return (DEOF);
    }
    Dcp = *Dvp++;
    return (' ');
}

static void
Dtestq(c)
    register int c;
{

    if (cmap(c, QUOTES))
	gflag = 1;
}

/*
 * Form a shell temporary file (in unit 0) from the words
 * of the shell input up to EOF or a line the same as "term".
 * Unit 0 should have been closed before this call.
 */
void
heredoc(term)
    Char   *term;
{
    int c;
    Char   *Dv[2];
    Char    obuf[BUFSIZE], lbuf[BUFSIZE], mbuf[BUFSIZE];
    int     ocnt, lcnt, mcnt;
    register Char *lbp, *obp, *mbp;
    Char  **vp;
    bool    quoted;
    char   *tmp;
#ifndef WINNT_NATIVE
    struct timeval tv;

again:
#endif /* WINNT_NATIVE */
    tmp = short2str(shtemp);
#ifndef O_CREAT
# define O_CREAT 0
    if (creat(tmp, 0600) < 0)
	stderror(ERR_SYSTEM, tmp, strerror(errno));
#endif
    (void) close(0);
#ifndef O_TEMPORARY
# define O_TEMPORARY 0
#endif
#ifndef O_EXCL
# define O_EXCL 0
#endif
    if (open(tmp, O_RDWR|O_CREAT|O_EXCL|O_TEMPORARY) == -1) {
	int oerrno = errno;
#ifndef WINNT_NATIVE
	if (errno == EEXIST) {
	    if (unlink(tmp) == -1) {
		(void) gettimeofday(&tv, NULL);
		shtemp = Strspl(STRtmpsh, putn((((int)tv.tv_sec) ^ 
		    ((int)tv.tv_usec) ^ ((int)getpid())) & 0x00ffffff));
	    }
	    goto again;
	}
#endif /* WINNT_NATIVE */
	(void) unlink(tmp);
	errno = oerrno;
 	stderror(ERR_SYSTEM, tmp, strerror(errno));
    }
    (void) unlink(tmp);		/* 0 0 inode! */
    Dv[0] = term;
    Dv[1] = NULL;
    gflag = 0;
    trim(Dv);
    rscan(Dv, Dtestq);
    quoted = gflag;
    ocnt = BUFSIZE;
    obp = obuf;
    inheredoc = 1;
#ifdef WINNT_NATIVE
    __dup_stdin = 1;
#endif /* WINNT_NATIVE */
    for (;;) {
	/*
	 * Read up a line
	 */
	lbp = lbuf;
	lcnt = BUFSIZE - 4;
	for (;;) {
	    c = readc(1);	/* 1 -> Want EOF returns */
	    if (c < 0 || c == '\n')
		break;
	    if ((c &= TRIM) != 0) {
		*lbp++ = (Char) c;
		if (--lcnt < 0) {
		    setname("<<");
		    stderror(ERR_NAME | ERR_OVERFLOW);
		}
	    }
	}
	*lbp = 0;

	/*
	 * Check for EOF or compare to terminator -- before expansion
	 */
	if (c < 0 || eq(lbuf, term)) {
	    (void) write(0, short2str(obuf), (size_t) (BUFSIZE - ocnt));
	    (void) lseek(0, (off_t) 0, L_SET);
	    inheredoc = 0;
	    return;
	}

	/*
	 * If term was quoted or -n just pass it on
	 */
	if (quoted || noexec) {
	    *lbp++ = '\n';
	    *lbp = 0;
	    for (lbp = lbuf; (c = *lbp++) != 0;) {
		*obp++ = (Char) c;
		if (--ocnt == 0) {
		    (void) write(0, short2str(obuf), BUFSIZE);
		    obp = obuf;
		    ocnt = BUFSIZE;
		}
	    }
	    continue;
	}

	/*
	 * Term wasn't quoted so variable and then command expand the input
	 * line
	 */
	Dcp = lbuf;
	Dvp = Dv + 1;
	mbp = mbuf;
	mcnt = BUFSIZE - 4;
	for (;;) {
	    c = DgetC(DODOL);
	    if (c == DEOF)
		break;
	    if ((c &= TRIM) == 0)
		continue;
	    /* \ quotes \ $ ` here */
	    if (c == '\\') {
		c = DgetC(0);
		if (!any("$\\`", c))
		    unDgetC(c | QUOTE), c = '\\';
		else
		    c |= QUOTE;
	    }
	    *mbp++ = (Char) c;
	    if (--mcnt == 0) {
		setname("<<");
		stderror(ERR_NAME | ERR_OVERFLOW);
	    }
	}
	*mbp++ = 0;

	/*
	 * If any ` in line do command substitution
	 */
	mbp = mbuf;
	if (Strchr(mbp, '`') != NULL) {
	    /*
	     * 1 arg to dobackp causes substitution to be literal. Words are
	     * broken only at newlines so that all blanks and tabs are
	     * preserved.  Blank lines (null words) are not discarded.
	     */
	    vp = dobackp(mbuf, 1);
	}
	else
	    /* Setup trivial vector similar to return of dobackp */
	    Dv[0] = mbp, Dv[1] = NULL, vp = Dv;

	/*
	 * Resurrect the words from the command substitution each separated by
	 * a newline.  Note that the last newline of a command substitution
	 * will have been discarded, but we put a newline after the last word
	 * because this represents the newline after the last input line!
	 */
	for (; *vp; vp++) {
	    for (mbp = *vp; *mbp; mbp++) {
		*obp++ = *mbp & TRIM;
		if (--ocnt == 0) {
		    (void) write(0, short2str(obuf), BUFSIZE);
		    obp = obuf;
		    ocnt = BUFSIZE;
		}
	    }
	    *obp++ = '\n';
	    if (--ocnt == 0) {
		(void) write(0, short2str(obuf), BUFSIZE);
		obp = obuf;
		ocnt = BUFSIZE;
	    }
	}
	if (pargv)
	    blkfree(pargv), pargv = 0;
    }
}
