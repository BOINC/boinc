/*
Copyright (c) 1986, 1993, 1995 by University of Toronto.
Written by Henry Spencer.  Not derived from licensed software.

Permission is granted to anyone to use this software for any
purpose on any computer system, and to redistribute it in any way,
subject to the following restrictions:

1. The author is not responsible for the consequences of use of
	this software, no matter how awful, even if they arise
	from defects in it.

2. The origin of this software must not be misrepresented, either
	by explicit claim or by omission.

3. Altered versions must be plainly marked as such, and must not
	be misrepresented (by explicit claim or omission) as being
	the original software.
 *** THIS IS AN ALTERED VERSION.  It was altered by Benoit
 *** Goudreault-Emond, bge@videotron.ca, on 2002/01/03, to add \< and
 *** \> for word matching, as well as pseudo character classes.  It
 *** also handles wide characters quite well now. Also, the API was
 *** changed so an arbitrary number of parenthesized expressions can
 *** be used (and so were the internals of the regexp engine).
 ***
 *** In general, the new API should be more re-entrant.

4. This notice must not be removed or altered.
*/

/*
 * regcomp and regexec -- regsub and regerror are elsewhere
 */
#include <stdio.h>

#include "regexp_int.h"
#include "regexp_custom.h"
#include "regmagic.h"

/* FORWARDING FUNCTIONS for macros in ctype */
static int isalnum_f(CHAR_TYPE c)
{
    return cisalnum(c);
}

static int isalpha_f(CHAR_TYPE c)
{
    return cisalpha(c);
}

static int isblank_f(CHAR_TYPE c)
{
#ifdef _WIN32
    return (c==' ') || (c=='\t');
#else
    return cisblank(c);
#endif
}

static int iscntrl_f(CHAR_TYPE c)
{
    return ciscntrl(c);
}

static int isdigit_f(CHAR_TYPE c)
{
    return cisdigit(c);
}

static int isgraph_f(CHAR_TYPE c)
{
    return cisgraph(c);
}

static int islower_f(CHAR_TYPE c)
{
    return cislower(c);
}

static int isprint_f(CHAR_TYPE c)
{
    return cisprint(c);
}

static int ispunct_f(CHAR_TYPE c)
{
    return cispunct(c);
}

static int isspace_f(CHAR_TYPE c)
{
    return cisspace(c);
}

static int isupper_f(CHAR_TYPE c)
{
    return cisupper(c);
}

static int isxdigit_f(CHAR_TYPE c)
{
    return cisxdigit(c);
}

static int isword_f(CHAR_TYPE c)
{
    return cisalnum(c) || c == LIT('_');
}

/* character class table */
static const CHAR_TYPE class_table[] = LIT("mabcdglpnsuxw");
typedef int (*cclass_t)(CHAR_TYPE);
static const cclass_t class_table_f[] = {
    isalnum_f, isalpha_f, isblank_f, iscntrl_f,
    isdigit_f, isgraph_f, islower_f, isprint_f,
    ispunct_f, isspace_f, isupper_f, isxdigit_f,
    isword_f
};

/*
 * The "internal use only" fields in regexp.h are present to pass info from
 * compile to execute that permits the execute phase to run lots faster on
 * simple cases.  They are:
 *
 * regstart	char that must begin a match; '\0' if none obvious
 * reganch	is the match anchored (at beginning-of-line only)?
 * regmust	string (pointer into program) that match must include, or NULL
 * regmlen	length of regmust string
 *
 * Regstart and reganch permit very fast decisions on suitable starting points
 * for a match, cutting down the work a lot.  Regmust permits fast rejection
 * of lines that cannot possibly match.  The regmust tests are costly enough
 * that regcomp() supplies a regmust only if the r.e. contains something
 * potentially expensive (at present, the only such thing detected is * or +
 * at the start of the r.e., which can involve a lot of backup).  Regmlen is
 * supplied because the test in regexec() needs it and regcomp() is computing
 * it anyway.
 */

/*
 * Structure for regexp "program".  This is essentially a linear encoding
 * of a nondeterministic finite-state machine (aka syntax charts or
 * "railroad normal form" in parsing technology).  Each node is an opcode
 * plus a "next" pointer, possibly plus an operand.  "Next" pointers of
 * all nodes except BRANCH implement concatenation; a "next" pointer with
 * a BRANCH on both ends of it is connecting two alternatives.  (Here we
 * have one of the subtle syntax dependencies:  an individual BRANCH (as
 * opposed to a collection of them) is never concatenated with anything
 * because of operator precedence.)  The operand of some types of node is
 * a literal string; for others, it is a node leading into a sub-FSM.  In
 * particular, the operand of a BRANCH node is the first node of the branch.
 * (NB this is *not* a tree structure:  the tail of the branch connects
 * to the thing following the set of BRANCHes.)  The opcodes are:
 */

#define NSUBEXPS 10

/* definition	number	opnd?	meaning */
#define	END	0	/* no	End of program. */
#define	BOL	1	/* no	Match beginning of line. */
#define	EOL	2	/* no	Match end of line. */
#define	ANY	3	/* no	Match any character. */
#define	ANYOF	4	/* str	Match any of these. */
#define	ANYBUT	5	/* str	Match any but one of these. */
#define	BRANCH	6	/* node	Match this, or the next..\&. */
#define	BACK	7	/* no	"next" ptr points backward. */
#define	EXACTLY	8	/* str	Match this string. */
#define	NOTHING	9	/* no	Match empty string. */
#define	STAR	10	/* node	Match this 0 or more times. */
#define	PLUS	11	/* node	Match this 1 or more times. */
#define CCLASS  12  /* chr  Match character class. */
#define WORDA   13  /* no   Match beginning of word */
#define WORDZ   14  /* no   Match end of word */
#define	OPEN	20	/* lvl	Sub-RE starts here. */
                    /* the level follows the OPEN opcode. (NEW) */
#define	CLOSE	30	/* lvl	Analogous to OPEN. */

/* character classes */

/*
 * Opcode notes:
 *
 * BRANCH	The set of branches constituting a single choice are hooked
 *		together with their "next" pointers, since precedence prevents
 *		anything being concatenated to any individual branch.  The
 *		"next" pointer of the last BRANCH in a choice points to the
 *		thing following the whole choice.  This is also where the
 *		final "next" pointer of each individual branch points; each
 *		branch starts with the operand node of a BRANCH node.
 *
 * BACK		Normal "next" pointers all implicitly point forward; BACK
 *		exists to make loop structures possible.
 *
 * STAR,PLUS	'?', and complex '*' and '+', are implemented as circular
 *		BRANCH structures using BACK.  Simple cases (one character
 *		per match) are implemented with STAR and PLUS for speed
 *		and to minimize recursive plunges.
 *
 * OPEN,CLOSE	...are numbered at compile time, but stored separately
 *              so we can get a more-or-less unlimited amount of
 *              subexpressions.
 */

/*
 * A node is one char of opcode followed by two chars of "next" pointer.
 * "Next" pointers are stored as two 8-bit pieces, high order first.  The
 * value is a positive offset from the opcode of the node containing it.
 * An operand, if any, simply follows the node.  (Note that much of the
 * code generation knows about this implicit relationship.)
 *
 * Using two bytes for the "next" pointer is vast overkill for most things,
 * but allows patterns to get big without disasters.
 */
#define	OP(p)		(*(p))
#define	NEXT(p)		(((*((p)+1)&0177)<<8) + (*((p)+2)&0377))
#define	OPERAND(p)	((p) + 3)

/*
 * See regmagic.h for one further detail of program structure.
 */


/*
 * Utility definitions.
 */
#define	FAIL(m, code)		{ re_report(m); return code; }
#define FAIL2(m, code)      { re_report(m); if(errp) *errp = code; return NULL; }
#define	ISREPN(c)	((c) == LIT('*') || (c) == LIT('+') || (c) == LIT('?'))
#define	META		LIT("^$.[()|?+*\\")

/*
 * Flags to be passed up and down.
 */
#define	HASWIDTH	01	/* Known never to match null string. */
#define	SIMPLE		02	/* Simple enough to be STAR/PLUS operand. */
#define	SPSTART		04	/* Starts with * or +. */
#define	WORST		0	/* Worst case. */

/*
 * Work-variable struct for regcomp().
 */
struct comp {
	CHAR_TYPE *regparse;		/* Input-scan pointer. */
	int regnpar;		/* () count. */
	CHAR_TYPE *regcode;		/* Code-emit pointer; &regdummy = don't. */
	CHAR_TYPE regdummy[3];	/* NOTHING, 0 next ptr */
	long regsize;		/* Code size. */
};
#define	EMITTING(cp)	((cp)->regcode != (cp)->regdummy)

/*
 * Forward declarations for regcomp()'s friends.
 */
static CHAR_TYPE *reg(struct comp *cp, int paren, int *flagp, int *errp);
static CHAR_TYPE *regbranch(struct comp *cp, int *flagp, int* errp);
static CHAR_TYPE *regpiece(struct comp *cp, int *flagp, int* errp);
static CHAR_TYPE *regatom(struct comp *cp, int *flagp, int* errp);
static CHAR_TYPE *regnode(struct comp *cp, int op);
static CHAR_TYPE *regnext(CHAR_TYPE *node);
static void regc(struct comp *cp, int c);
static void reginsert(struct comp *cp, int op, CHAR_TYPE *opnd);
static void regtail(struct comp *cp, CHAR_TYPE *p, CHAR_TYPE *val);
static void regoptail(struct comp *cp, CHAR_TYPE *p, CHAR_TYPE *val);

/*
 - regcomp - compile a regular expression into internal code
 *
 * We can't allocate space until we know how big the compiled form will be,
 * but we can't compile it (and thus know how big it is) until we've got a
 * place to put the code.  So we cheat:  we compile it twice, once with code
 * generation turned off and size counting turned on, and once "for real".
 * This also means that we don't allocate space until we are sure that the
 * thing really will compile successfully, and we never have to move the
 * code and thus invalidate pointers into it.  (Note that it has to be in
 * one piece because free() must be able to free it all.)
 *
 * Beware that the optimization-preparation code in here knows about some
 * of the structure of the compiled regexp.
 *
 * Note: cflags is for future extensions (such as case insensitive search,
 * not supported yet)
 */
int re_comp_w(regexp** rpp, const CHAR_TYPE* exp)
{
	register CHAR_TYPE *scan;
	int flags;
	struct comp co;
    int error = 0;

    if(!rpp)
        FAIL("Invalid out regexp pointer", REGEXP_BADARG);
    {
        register regexp* r;
        
        if (exp == NULL)
            FAIL("Invalid expression", REGEXP_BADARG);
        
        /* First pass: determine size, legality. */
        co.regparse = (CHAR_TYPE *)exp;
        co.regnpar = 1;
        co.regsize = 0L;
        co.regdummy[0] = NOTHING;
        co.regdummy[1] = co.regdummy[2] = 0;
        co.regcode = co.regdummy;
        regc(&co, MAGIC);
        if (reg(&co, 0, &flags, &error) == NULL)
            return error;

        /* Small enough for pointer-storage convention? */
        if (co.regsize >= 0x7fffL)	/* Probably could be 0xffffL. */
            FAIL("regexp too big", REGEXP_ESIZE);

        /* Allocate space. */
        r = (regexp *)re_malloc(sizeof(regexp) + (size_t)co.regsize*sizeof(CHAR_TYPE));
        if (r == NULL)
            FAIL("out of space", REGEXP_ESPACE);

        /* Second pass: emit code. */
        co.regparse = (CHAR_TYPE *)exp;
        co.regnpar = 1;
        co.regcode = r->program;
        regc(&co, MAGIC);
        if(reg(&co, 0, &flags, &error) == NULL)
        {
            re_cfree(r);
            return error;
        }

        /* Dig out information for optimizations. */
        r->regstart = LIT('\0');		/* Worst-case defaults. */
        r->reganch = 0;
        r->regmust = NULL;
        r->regmlen = 0;
        scan = r->program+1;		/* First BRANCH. */
        if (OP(regnext(scan)) == END) {	/* Only one top-level choice. */
            scan = OPERAND(scan);

            /* Starting-point info. */
            if (OP(scan) == EXACTLY)
                r->regstart = *OPERAND(scan);
            else if (OP(scan) == BOL)
                r->reganch = 1;
            
            /*
             * If there's something expensive in the r.e., find the
             * longest literal string that must appear and make it the
             * regmust.  Resolve ties in favor of later strings, since
             * the regstart check works with the beginning of the r.e.
             * and avoiding duplication strengthens checking.  Not a
             * strong reason, but sufficient in the absence of others.
             */
            if (flags&SPSTART) {
                register CHAR_TYPE *longest = NULL;
                register size_t len = 0;
                
                for (; scan != NULL; scan = regnext(scan))
                    if (OP(scan) == EXACTLY && cstrlen(OPERAND(scan)) >= len) {
                        longest = OPERAND(scan);
                        len = cstrlen(OPERAND(scan));
                    }
                r->regmust = longest;
                r->regmlen = (int)len;
            }
        }
        
        r->regnsubexp = co.regnpar;

        *rpp = r;
        return 0;
    }
}

/*
 - reg - regular expression, i.e. main body or parenthesized thing
 *
 * Caller must absorb opening parenthesis.
 *
 * Combining parenthesis handling with the base level of regular expression
 * is a trifle forced, but the need to tie the tails of the branches to what
 * follows makes it hard to avoid.
 */
static CHAR_TYPE *
reg(struct comp* cp, int paren, int* flagp, int* errp)
{
	register CHAR_TYPE *ret;
	register CHAR_TYPE *br;
	register CHAR_TYPE *ender;
	register int parno;
	int flags;

	*flagp = HASWIDTH;	/* Tentatively. */

	if (paren) {
		/* Make an OPEN node. */
		if (cp->regnpar >= REGEXP_MAXEXP)
        {
            re_report("Too many ()");
            *errp = REGEXP_EEND;
            return NULL;
        }
        
		parno = cp->regnpar;
		cp->regnpar++;
        if(parno > NSUBEXPS)
        {
            ret = regnode(cp, OPEN);
            regc(cp, parno);
        }
        else
            ret = regnode(cp, OPEN + parno);
	}

	/* Pick up the branches, linking them together. */
	br = regbranch(cp, &flags, errp);
	if (br == NULL)
		return(NULL);
	if (paren)
		regtail(cp, ret, br);	/* OPEN -> first. */
	else
		ret = br;
	*flagp &= ~(~flags&HASWIDTH);	/* Clear bit if bit 0. */
	*flagp |= flags&SPSTART;
	while (*cp->regparse == LIT('|')) {
		cp->regparse++;
		br = regbranch(cp, &flags, errp);
		if (br == NULL)
			return(NULL);
		regtail(cp, ret, br);	/* BRANCH -> BRANCH. */
		*flagp &= ~(~flags&HASWIDTH);
		*flagp |= flags&SPSTART;
	}

	/* Make a closing node, and hook it on the end. */
    if(paren)
    {
        if(parno > NSUBEXPS)
        {
            ender = regnode(cp, CLOSE);
            regc(cp, parno);
        }
        else
            ender = regnode(cp, CLOSE + parno);
    }
    else
        ender = regnode(cp, END);
	regtail(cp, ret, ender);

	/* Hook the tails of the branches to the closing node. */
	for (br = ret; br != NULL; br = regnext(br))
		regoptail(cp, br, ender);

	/* Check for proper termination. */
	if (paren && *cp->regparse++ != LIT(')')) {
		FAIL2("unterminated ()", REGEXP_EPAREN);
	} else if (!paren && *cp->regparse != LIT('\0')) {
		if (*cp->regparse == LIT(')')) {
			FAIL2("unmatched ()", REGEXP_EPAREN);
		} else
			FAIL2("internal error: junk on end", REGEXP_EEND);
		/* NOTREACHED */
	}

	return(ret);
}

/*
 - regbranch - one alternative of an | operator
 *
 * Implements the concatenation operator.
 */
static CHAR_TYPE *
regbranch(struct comp* cp, int* flagp, int* errp)
{
	register CHAR_TYPE *ret;
	register CHAR_TYPE *chain;
	register CHAR_TYPE *latest;
	int flags;
	register int c;

	*flagp = WORST;				/* Tentatively. */

	ret = regnode(cp, BRANCH);
	chain = NULL;
	while ((c = *cp->regparse) != LIT('\0') && c != LIT('|') && c != LIT(')')) {
		latest = regpiece(cp, &flags, errp);
		if (latest == NULL)
			return(NULL);
		*flagp |= flags&HASWIDTH;
		if (chain == NULL)		/* First piece. */
			*flagp |= flags&SPSTART;
		else
			regtail(cp, chain, latest);
		chain = latest;
	}
	if (chain == NULL)			/* Loop ran zero times. */
		(void) regnode(cp, NOTHING);

	return(ret);
}

/*
 - regpiece - something followed by possible [*+?]
 *
 * Note that the branching code sequences used for ? and the general cases
 * of * and + are somewhat optimized:  they use the same NOTHING node as
 * both the endmarker for their branch list and the body of the last branch.
 * It might seem that this node could be dispensed with entirely, but the
 * endmarker role is not redundant.
 */
static CHAR_TYPE *regpiece(struct comp* cp, int* flagp, int* errp)
{
	register CHAR_TYPE *ret;
	register CHAR_TYPE op;
	register CHAR_TYPE *next;
	int flags;

	ret = regatom(cp, &flags, errp);
	if (ret == NULL)
		return(NULL);

	op = *cp->regparse;
	if (!ISREPN(op)) {
		*flagp = flags;
		return(ret);
	}

	if (!(flags&HASWIDTH) && op != LIT('?'))
		FAIL2("*+ operand could be empty", REGEXP_BADRPT);
	switch (op) {
	case LIT('*'):	*flagp = WORST|SPSTART;			break;
	case LIT('+'):	*flagp = WORST|SPSTART|HASWIDTH;	break;
	case LIT('?'):	*flagp = WORST;				break;
	}

	if (op == LIT('*') && (flags&SIMPLE))
		reginsert(cp, STAR, ret);
	else if (op == LIT('*')) {
		/* Emit x* as (x&|), where & means "self". */
		reginsert(cp, BRANCH, ret);		/* Either x */
		regoptail(cp, ret, regnode(cp, BACK));	/* and loop */
		regoptail(cp, ret, ret);		/* back */
		regtail(cp, ret, regnode(cp, BRANCH));	/* or */
		regtail(cp, ret, regnode(cp, NOTHING));	/* null. */
	} else if (op == LIT('+') && (flags&SIMPLE))
		reginsert(cp, PLUS, ret);
	else if (op == LIT('+')) {
		/* Emit x+ as x(&|), where & means "self". */
		next = regnode(cp, BRANCH);		/* Either */
		regtail(cp, ret, next);
		regtail(cp, regnode(cp, BACK), ret);	/* loop back */
		regtail(cp, next, regnode(cp, BRANCH));	/* or */
		regtail(cp, ret, regnode(cp, NOTHING));	/* null. */
	} else if (op == LIT('?')) {
		/* Emit x? as (x|) */
		reginsert(cp, BRANCH, ret);		/* Either x */
		regtail(cp, ret, regnode(cp, BRANCH));	/* or */
		next = regnode(cp, NOTHING);		/* null. */
		regtail(cp, ret, next);
		regoptail(cp, ret, next);
	}
	cp->regparse++;
	if (ISREPN(*cp->regparse))
		FAIL2("nested *?+", REGEXP_BADRPT);

	return(ret);
}

/*
 - regatom - the lowest level
 *
 * Optimization:  gobbles an entire sequence of ordinary characters so that
 * it can turn them into a single node, which is smaller to store and
 * faster to run.  Backslashed characters are exceptions, each becoming a
 * separate node; the code is simpler that way and it's not worth fixing.
 */
static CHAR_TYPE *regatom(struct comp* cp, int* flagp, int* errp)
{
	register CHAR_TYPE *ret;
	int flags;

	*flagp = WORST;		/* Tentatively. */

	switch (*cp->regparse++) {
	case LIT('^'):
		ret = regnode(cp, BOL);
		break;
	case LIT('$'):
		ret = regnode(cp, EOL);
		break;
	case LIT('.'):
		ret = regnode(cp, ANY);
		*flagp |= HASWIDTH|SIMPLE;
		break;
	case LIT('['): {
		register int range;
		register int rangeend;
		register int c;

		if (*cp->regparse == LIT('^')) {	/* Complement of range. */
			ret = regnode(cp, ANYBUT);
			cp->regparse++;
		} else
			ret = regnode(cp, ANYOF);
		if ((c = *cp->regparse) == LIT(']') || c == LIT('-')) {
			regc(cp, c);
			cp->regparse++;
		}
		while ((c = *cp->regparse++) != LIT('\0') && c != LIT(']')) {
			if (c != LIT('-'))
				regc(cp, c);
			else if ((c = *cp->regparse) == LIT(']') || c == LIT('\0'))
				regc(cp, LIT('-'));
			else {
				range = (UCHAR_TYPE)*(cp->regparse-2);
				rangeend = (UCHAR_TYPE)c;
				if (range > rangeend)
					FAIL2("invalid [] range", REGEXP_ERANGE);
				for (range++; range <= rangeend; range++)
					regc(cp, range);
				cp->regparse++;
			}
		}
		regc(cp, LIT('\0'));
		if (c != LIT(']'))
			FAIL2("unmatched []", REGEXP_EBRACK);
		*flagp |= HASWIDTH|SIMPLE;
		break;
		}
	case LIT('('):
		ret = reg(cp, 1, &flags, errp);
		if (ret == NULL)
			return(NULL);
		*flagp |= flags&(HASWIDTH|SPSTART);
		break;
	case LIT('\0'):
	case LIT('|'):
	case LIT(')'):
		/* supposed to be caught earlier */
		FAIL2("internal error: \\0|) unexpected", REGEXP_EEND);
		break;
	case LIT('?'):
	case LIT('+'):
	case LIT('*'):
		FAIL2("?+* follows nothing", REGEXP_BADRPT);
		break;
	case LIT('\\'):
		if (*cp->regparse == LIT('\0'))
			FAIL2("trailing \\", REGEXP_EESCAPE);
        /* check for match in char class */
        {
            const CHAR_TYPE* c;
            c = cstrchr(class_table, *cp->regparse);
            if(c != NULL)
            {
                ret = regnode(cp, CCLASS);
                regc(cp, c - class_table);
            }
            else if((c = cstrchr(class_table, ctolower(*cp->regparse))) != NULL)
            {
                ret = regnode(cp, CCLASS);
                /* negative char class */
                regc(cp, -(class_table - c + 1));
            }
            else if(*cp->regparse == L'<')
            {
                ret = regnode(cp, WORDA);
            }
            else if(*cp->regparse == L'>')
            {
                ret = regnode(cp, WORDZ);
            }
            else
            {
                ret = regnode(cp, EXACTLY);
                regc(cp, *cp->regparse);
                regc(cp, LIT('\0'));
            }
            cp->regparse++;
            *flagp |= HASWIDTH|SIMPLE;
        }
		break;
	default: {
		register size_t len;
		register CHAR_TYPE ender;

		cp->regparse--;
		len = cstrcspn(cp->regparse, META);
		if (len == 0)
			FAIL2("internal error: strcspn 0", REGEXP_EEND);
		ender = *(cp->regparse+len);
		if (len > 1 && ISREPN(ender))
			len--;		/* Back off clear of ?+* operand. */
		*flagp |= HASWIDTH;
		if (len == 1)
			*flagp |= SIMPLE;
		ret = regnode(cp, EXACTLY);
		for (; len > 0; len--)
			regc(cp, *cp->regparse++);
		regc(cp, LIT('\0'));
		break;
		}
	}

	return(ret);
}

/*
 - regnode - emit a node; returns Location.
 */
static CHAR_TYPE *regnode(struct comp* cp, int op)
{
	register CHAR_TYPE *const ret = cp->regcode;
	register CHAR_TYPE *ptr;

	if (!EMITTING(cp)) {
		cp->regsize += 3;
		return(ret);
	}

	ptr = ret;
	*ptr++ = op;
	*ptr++ = LIT('\0');		/* Null next pointer. */
	*ptr++ = LIT('\0');
	cp->regcode = ptr;

	return(ret);
}

/*
 - regc - emit (if appropriate) a byte of code
 */
static void regc(struct comp* cp, int b)
{
	if (EMITTING(cp))
		*cp->regcode++ = b;
	else
		cp->regsize++;
}

/*
 - reginsert - insert an operator in front of already-emitted operand
 *
 * Means relocating the operand.
 */
static void reginsert(struct comp* cp, int op, CHAR_TYPE* opnd)
{
	register CHAR_TYPE *place;

	if (!EMITTING(cp)) {
		cp->regsize += 3;
		return;
	}

	(void) memmove(opnd+3, opnd, (size_t)(cp->regcode - opnd) * sizeof(CHAR_TYPE));
	cp->regcode += 3;

	place = opnd;		/* Op node, where operand used to be. */
	*place++ = op;
	*place++ = LIT('\0');
	*place++ = LIT('\0');
}

/*
 - regtail - set the next-pointer at the end of a node chain
 */
static void regtail(struct comp* cp, CHAR_TYPE* p, CHAR_TYPE* val)
{
	register CHAR_TYPE *scan;
	register CHAR_TYPE *temp;
	register int offset;

	if (!EMITTING(cp))
		return;

	/* Find last node. */
	for (scan = p; (temp = regnext(scan)) != NULL; scan = temp)
		continue;

	offset = (OP(scan) == BACK) ? scan - val : val - scan;
	*(scan+1) = (offset>>8)&0177;
	*(scan+2) = offset&0377;
}

/*
 - regoptail - regtail on operand of first argument; nop if operandless
 */
static void regoptail(struct comp* cp, CHAR_TYPE* p, CHAR_TYPE* val)
{
	/* "Operandless" and "op != BRANCH" are synonymous in practice. */
	if (!EMITTING(cp) || OP(p) != BRANCH)
		return;
	regtail(cp, OPERAND(p), val);
}


/*
 * regexec and friends
 */

/*
 * Work-variable struct for regexec().
 */
struct exec {
	CHAR_TYPE *reginput;		/* String-input pointer. */
	CHAR_TYPE *regbol;		/* Beginning of input, for ^ check. */
    regmatch* regmatchp;        /* match input/output array */
    int   regnsubexp;           /* number of elements in array */
};

/*
 * Forwards.
 */
static int regtry(struct exec *ep, const regexp *rp, CHAR_TYPE *string, int offset);
static int regmatch_(struct exec *ep, CHAR_TYPE *prog);
static size_t regrepeat(struct exec *ep, CHAR_TYPE *node);

/*
 - regexec - match a regexp against a string
 */
int
re_exec_w(const regexp* rp, const CHAR_TYPE* str, size_t nmatch, regmatch pmatch[])
{
	register CHAR_TYPE *string = (CHAR_TYPE *)str;	/* avert const poisoning */
	register CHAR_TYPE *s;
	struct exec ex;

	/* Be paranoid. */
	if (rp == NULL || string == NULL) {
		FAIL("NULL argument to regexec", REGEXP_BADARG);
	}

	/* Check validity of program. */
	if ((UCHAR_TYPE)*rp->program != MAGIC) {
        FAIL("corrupted regexp", REGEXP_BADARG);
	}

	/* If there is a "must appear" string, look for it. */
	if (rp->regmust != NULL && cstrstr(string, rp->regmust) == NULL)
		return(0);

	/* Mark beginning of line for ^ . */
	ex.regbol = string;
	ex.regmatchp = pmatch;
    ex.regnsubexp = nmatch;
        
	/* Simplest case:  anchored match need be tried only once. */
	if (rp->reganch)
		return(regtry(&ex, rp, string, 0));

	/* Messy cases:  unanchored match. */
	if (rp->regstart != LIT('\0')) {
		/* We know what char it must start with. */
		for (s = string; s != NULL; s = cstrchr(s+1, rp->regstart))
			if (regtry(&ex, rp, s, s - string) > 0)
				return(1);
		return(0);
	} else {
        int error = 1;
		/* We don't -- general case. */
		for (s = string; (error = regtry(&ex, rp, s, s - string)) == 0; s++)
			if (*s == LIT('\0'))
				return(0);        
		return(error);
	}
	/* NOTREACHED */
}

/*
 - regtry - try match at specific point
 */
static int regtry(struct exec* ep, const regexp* prog, CHAR_TYPE* string, int offset)
{
	register regmatch *stp;
    int error;

	ep->reginput = string;

    /* Prefill match array */
    if(ep->regmatchp)
    {
        for(stp = ep->regmatchp; stp - ep->regmatchp < ep->regnsubexp; ++stp)
        {
            stp->begin = -1;
            stp->end = -1;
        }
    }
	if ((error = regmatch_(ep, (CHAR_TYPE*)prog->program + 1)) > 0) {
        if(ep->regmatchp && ep->regnsubexp >= 1)
        {
            ep->regmatchp[0].begin = offset;
            ep->regmatchp[0].end   = offset + ep->reginput - string;
        }
        return(1);
	} else
		return(error);
}

/*
 - regmatch - main matching routine
 *
 * Conceptually the strategy is simple:  check to see whether the current
 * node matches, call self recursively to see whether the rest matches,
 * and then act accordingly.  In practice we make some effort to avoid
 * recursion, in particular by going through "ordinary" nodes (that don't
 * need to know whether the rest of the match failed) by a loop instead of
 * by recursion.
 */
static int regmatch_(struct exec* ep, CHAR_TYPE* prog)
{
	register CHAR_TYPE *scan;	/* Current node. */
	CHAR_TYPE *next;		/* Next node. */

	for (scan = prog; scan != NULL; scan = next) {
		next = regnext(scan);

		switch (OP(scan)) {
		case BOL:
			if (ep->reginput != ep->regbol)
				return(0);
			break;
		case EOL:
			if (*ep->reginput != LIT('\0'))
				return(0);
			break;
        case WORDA:
            /* must be in the word char class */
            if (!isword_f(*ep->reginput))
                return(0);
            /* previous must be BOL or nonword */
            if(ep->reginput > ep->regbol &&
               isword_f(*(ep->reginput - 1)))
                return(0);
            /* NOTE: no increment--first match is "pushed back"
             * (actually, never consumed) */
            break;
        case WORDZ:
            /* stops matching when non-word */
            if (isword_f(*ep->reginput))
                return(0);
            /* previous char is not important */
            /* NOTE: no increment--first match is "pushed back"
             * (actually, never consumed) */
            break;
		case ANY:
			if (*ep->reginput == LIT('\0'))
				return(0);
			ep->reginput++;
			break;
		case EXACTLY: {
			register size_t len;
			register CHAR_TYPE *const opnd = OPERAND(scan);

			/* Inline the first character, for speed. */
			if (*opnd != *ep->reginput)
				return(0);
			len = cstrlen(opnd);
			if (len > 1 && cstrncmp(opnd, ep->reginput, len) != 0)
				return(0);
			ep->reginput += len;
			break;
			}
        case CCLASS: {
            register CHAR_TYPE* const opnd = OPERAND(scan);
            if(opnd >= 0)
            {
                if (ep->reginput == LIT('\0') ||
                    !(*class_table_f[*opnd])(*ep->reginput))
                    return 0;
            }
            else
            {
                if (ep->reginput == LIT('\0') ||
                    !(*class_table_f[-*opnd - 1])(*ep->reginput))
                    return 0;
            }
            ep->reginput++;
            break;
            }
		case ANYOF:
			if (*ep->reginput == LIT('\0') ||
					cstrchr(OPERAND(scan), *ep->reginput) == NULL)
				return(0);
			ep->reginput++;
			break;
		case ANYBUT:
			if (*ep->reginput == LIT('\0') ||
					cstrchr(OPERAND(scan), *ep->reginput) != NULL)
				return(0);
			ep->reginput++;
			break;
		case NOTHING:
			break;
		case BACK:
			break;
		case BRANCH: {
			register CHAR_TYPE *const save = ep->reginput;

			if (OP(next) != BRANCH)		/* No choice. */
				next = OPERAND(scan);	/* Avoid recursion. */
			else {
				while (OP(scan) == BRANCH) {
					if (regmatch_(ep, OPERAND(scan)) > 0)
						return(1);
					ep->reginput = save;
					scan = regnext(scan);
				}
				return(0);
				/* NOTREACHED */
			}
			break;
			}
		case STAR: case PLUS: {
			register const CHAR_TYPE nextch =
				(OP(next) == EXACTLY) ? *OPERAND(next) : LIT('\0');
			register size_t no;
			register CHAR_TYPE *const save = ep->reginput;
			register const size_t min = (OP(scan) == STAR) ? 0 : 1;

			for (no = regrepeat(ep, OPERAND(scan)) + 1; no > min; no--) {
				ep->reginput = save + no - 1;
				/* If it could work, try it. */
				if (nextch == LIT('\0') || *ep->reginput == nextch)
					if (regmatch_(ep, next) > 0)
						return(1);
			}
			return(0);
			break;
			}
		case END:
			return(1);	/* Success! */
			break;

        case OPEN+1: case OPEN+2: case OPEN+3:
        case OPEN+4: case OPEN+5: case OPEN+6:
        case OPEN+7: case OPEN+8: case OPEN+9: {
            register const int no = OP(scan) - OPEN;
            register CHAR_TYPE *const input = ep->reginput;

            if (regmatch_(ep, next) > 0) {
                /*
                 * Don't set start if some later
                 * invocation of the same parentheses
                 * already has.
                 */
                if (ep->regmatchp &&
                    no < ep->regnsubexp &&
                    ep->regmatchp[no].begin == -1)
                    ep->regmatchp[no].begin = input - ep->regbol;
                return(1);
            } else
                return(0);
            break;
            }
            
        case OPEN: {
            register const int no = *OPERAND(scan);
            register CHAR_TYPE *const input = ep->reginput;
            
            if (regmatch_(ep, next) > 0) {
                /*
                 * Don't set start if some later
                 * invocation of the same parentheses
                 * already has.
                 */
                if (ep->regmatchp &&
                    no < ep->regnsubexp &&
                    ep->regmatchp[no].begin == -1)
                    ep->regmatchp[no].begin = input - ep->regbol;
                return(1);
            } else
                return(0);
            break;
            }
            
        case CLOSE+1: case CLOSE+2: case CLOSE+3:
        case CLOSE+4: case CLOSE+5: case CLOSE+6:
        case CLOSE+7: case CLOSE+8: case CLOSE+9: {
            register const int no = OP(scan) - CLOSE;
            register CHAR_TYPE *const input = ep->reginput;

            if (regmatch_(ep, next) > 0) {
                /*
                 * Don't set end if some later
                 * invocation of the same parentheses
                 * already has.
                 */
                if (ep->regmatchp &&
                    no < ep->regnsubexp &&
                    ep->regmatchp[no].end == -1)
                    ep->regmatchp[no].end = input - ep->regbol;
                return(1);
            } else
                return(0);
            break;
            }
            
        case CLOSE: {
            register const int no = *OPERAND(scan);
            register CHAR_TYPE *const input = ep->reginput;
                
            if (regmatch_(ep, next) > 0) {
                /*
                 * Don't set end if some later
                 * invocation of the same parentheses
                 * already has.
                 */
                if (ep->regmatchp &&
                    no < ep->regnsubexp &&
                    ep->regmatchp[no].end == -1)
                    ep->regmatchp[no].end = input - ep->regbol;
                return(1);
            } else
                return(0);
            break;
            }
            
		default:
			FAIL("regexp corruption", REGEXP_EEND);
			break;
		}
	}

	/*
	 * We get here only if there's trouble -- normally "case END" is
	 * the terminating point.
	 */
	FAIL("corrupted pointers", REGEXP_EEND);
}

/*
 - regrepeat - report how many times something simple would match
 */
static size_t regrepeat(struct exec* ep, CHAR_TYPE* node)
{
	register size_t count;
	register CHAR_TYPE *scan;
	register CHAR_TYPE ch;

	switch (OP(node)) {
	case ANY:
		return(cstrlen(ep->reginput));
		break;
	case EXACTLY:
		ch = *OPERAND(node);
		count = 0;
		for (scan = ep->reginput; *scan == ch; scan++)
			count++;
		return(count);
		break;
    case CCLASS:
        ch = *OPERAND(node);
        count = 0;
        if(ch >= 0)
        {
            for (scan = ep->reginput; (*class_table_f[ch])(*scan); scan++)
                count++;
        }
        else
        {
            ch = -ch + 1;
            for (scan = ep->reginput; !(*class_table_f[ch])(*scan); scan++)
                count++;
        }
        return(count);
        break;
	case ANYOF:
		return(cstrspn(ep->reginput, OPERAND(node)));
		break;
	case ANYBUT:
		return(cstrcspn(ep->reginput, OPERAND(node)));
		break;
	default:		/* Oh dear.  Called inappropriately. */
		re_report("internal error: bad call of regrepeat");
		return(0);	/* Best compromise. */
		break;
	}
	/* NOTREACHED */
}

/*
 - regnext - dig the "next" pointer out of a node
 */
static CHAR_TYPE *regnext(CHAR_TYPE* p)
{
	register const int offset = NEXT(p);

	if (offset == 0)
		return(NULL);

	return((OP(p) == BACK) ? p-offset : p+offset);
}

/*
 * re_nsubexp
 */
int re_nsubexp(const regexp* rp)
{
	/* Be paranoid. */
	if (rp == NULL)
		FAIL("NULL argument to re_nsubexp", REGEXP_BADARG);

	/* Check validity of program. */
	if ((UCHAR_TYPE)*rp->program != MAGIC)
		FAIL("corrupted regexp", REGEXP_BADARG);

    return rp->regnsubexp;
}

/*
 * re_free
 */
void re_free(void* object)
{
    re_cfree(object);
}
