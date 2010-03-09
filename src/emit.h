/******************************************************************************
 *
 *  File Name........: emit.h
 *
 *  Description......:
 *
 *  Created by vin on 03/08/10
 *
 *  Revision History.:
 *
 *****************************************************************************/

#ifndef EMIT_H
#define EMIT_H

typedef enum
{
    /* RR instructions */
    HALT,		/* RR     halt, operands are ignored */
    IN,		      /* RR     read integer into reg(r); s and t are ignored */
    INB,		/* RR     read bool into reg(r); s and t are ignored */
    OUT,	/* RR     write integer from reg(r), s and t are ignored */
    OUTB,		/* RR     write bool from reg(r), s and t are ignored */
    OUTNL,		/* RR     write newline regs r, s and t are ignored */
    ADD,		/* RR     reg(r) = reg(s)+reg(t) */
    SUB,		/* RR     reg(r) = reg(s)-reg(t) */
    MUL,		/* RR     reg(r) = reg(s)*reg(t) */
    DIV,		/* RR     reg(r) = reg(s)/reg(t) */
    RRLim,		/* limit of RR opcodes */

    /* RM instructions */
    LD,			/* RM     reg(r) = mem(d+reg(s)) */
    ST,			/* RM     mem(d+reg(s)) = reg(r) */
    RMLim,		/* Limit of RM opcodes */

    /* RA instructions */
    LDA,		/* RA     reg(r) = d+reg(s) */
    LDC,		/* RA     reg(r) = d ; reg(s) is ignored */
    JLT,		/* RA     if reg(r)<0 then reg(7) = d+reg(s) */
    JLE,		/* RA     if reg(r)<=0 then reg(7) = d+reg(s) */
    JGT,		/* RA     if reg(r)>0 then reg(7) = d+reg(s) */
    JGE,		/* RA     if reg(r)>=0 then reg(7) = d+reg(s) */
    JEQ,		/* RA     if reg(r)==0 then reg(7) = d+reg(s) */
    JNE,		/* RA     if reg(r)!=0 then reg(7) = d+reg(s) */
    RALim,		/* Limit of RA opcodes */

} Inst;

void emitFile(FILE *);
int emitSkip(int);
int emitBackPatch(int, Inst, int, int, int, char *);
int emit(Inst, int, int, int, char *);
int emitComment(char *);
int emitData(int, char *);
int emitSData(char *, char *);
int emitDataOffset();

#endif /* EMIT_H */

/*........................ end of emit.h ....................................*/
