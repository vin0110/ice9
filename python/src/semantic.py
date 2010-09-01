#!/usr/bin/python
"""
semantic analysis for ice9

VW Freeh copyright 2010

CSC 512
"""

#########
# Library imports
#########

#########
# Local imports
#########
from ast import Seq, StmList, ExpList, TypeList, IdxList, VarList, \
    Program, DecList,\
    Int, Str, Bool, Proc, Forward, \
    If, Do, Fa, Exit, Write, Break, Return, Assign, \
    Binop, Uniop, Call, Var, Sym, Read, Nop

from symbol import Sig

#########
# Globals
#########
Debug=0
Verbose=0
from errors import SemanticError

# @@@ should get same object as parser
SigI = Sig('int')
SigB = Sig('bool')
SigS = Sig('string')
SigN = Sig('nil')

def propagateSigs(n):
    if n is None:
        return
    t = n.type
    if t in ["Seq", "SeqStm", "SeqExp", "IdxList", "SeqVar", \
                 "SeqType", "SeqDec"]:
        # @@@ should this be done for Type, Dec?
        for k in n.kids:
            propagateSigs(k)
        n.sig = SigN
    elif t == "Program":
        ProgagateSigs(n.body)
        n.sig = SigN
    elif t == "Proc":
        ProgagateSigs(n.body)
        if n.returns:
            n.sig = n.returns.sig
        else:
            n.sig = SigN
    elif t == "Int":
        n.sig = SigI
    elif t == "Str":
        n.sig = SigB
    elif t == "Bool":
        n.sig = SigB
    elif t == "Forward":
        pass
    elif t == "If":
        propagateSigs(n.test)
        propagateSigs(n.then)
        propagateSigs(n.elze)
        n.sig = SigN
    elif t == "Do":
        propagateSigs(n.test)
        propagateSigs(n.body)
        n.sig = SigN
    elif t == "Fa":
        propagateSigs(n.var)
        propagateSigs(n.test)
        propagateSigs(n.body)
        n.sig = SigN
    elif t == "Exit":
        n.sig = SigN
    elif t == "Write":
        propagateSigs(n.exp)
        n.sig = SigN
    elif t == "Break":
        n.sig = SigN
    elif t == "Return":
        propagateSigs(n.exp)
        n.sig = SigN
    elif t == "Assign":
        propagateSigs(n.var)
        propagateSigs(n.exp)
        n.sig = SigN
    elif t == "Binop":
        propagateSigs(n.left)
        propagateSigs(n.right)
        n.sig = checkBinop(n.op, n.left.sig, n.right.sig)
    elif t == "Uniop":
        propagateSigs(n.exp)
        sig = n.exp.sig
        if n.op == '-' and (sig.check(SigI) or sig.check(SigB)):
            n.sig = sig
        elif n.op == '?' and sig.check(SigB):
            n.sig = SigI
        else:
            raise ValueError('invalid type of uniop ' + n.op)
    elif t == "Call":
        propagateSigs(n.args)
        if n.returns:
            propagateSigs(n.returns)
            n.sig = n.returns.sig
    elif t == "Var":
        n.sig = n.sym.sig
    elif t == "Sym":
        pass
    elif t == "Read":
        n.sig = SigI
    elif t == "Nop":
        n.sig = SigN
    else:
        raise ValueError("propogateSigs: invalid type %s", t)

def checkBinop(op, l, r):
    if op in ['*', '+']:
        # must be same, int or bool
        if l.check(r) and (l.check(SigI) or l.check(SigB)):
            return l
    elif op in ['=', '!=']:
        # must be same, int or bool, result bool
        if l.check(r) and (l.check(SigI) or l.check(SigB)):
            return SigB
    elif op in [">", "<", ">=", "<="]:
        # must be same, int, result bool
        if l.check(r) and l.check(SigI):
            return SigB
    elif op in ['-', '/', '%']:
        # must be same, must be int
        if l.check(r) and l.check(SigI):
            return l
    else:
        raise ValueError('invalid binop ' + op)
    raise ValueError('invalid types for binop ' + n.op)

def doSemantics(ast, debug, verbose):
    global Debug, Verbose

    Debug = debug
    Verbose = verbose
    propagateSigs(ast)
    # @@@ any other semantic checks? 

def main():
    pass

if __name__ == "__main__":
    main()
