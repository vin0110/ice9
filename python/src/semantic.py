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
from ast import Seq, StmList, ExpList, TypeList, VarList, \
    Program, DecList,\
    Int, Str, Bool, Proc, Forward, \
    If, Do, Fa, Exit, Write, Break, Return, Assign, \
    Binop, Uniop, Call, Var, Sym, Read, Nop, Idx, Rval, Lval

from symbol import Sig, ListSig

#########
# Globals
#########
Debug=0
Verbose=0
EmitLibs=""
from errors import SemanticError, SigError

# @@@ should get same object as parser
SigI = Sig('int')
SigB = Sig('bool')
SigS = Sig('string')
SigN = Sig('nil')

def propagateSigs(n):
    if n is None:
        return
    t = n.type
    if t in ["Seq", "SeqStm", "IdxList", "SeqVar", \
                 "SeqType", "SeqDec"]:
        # @@@ should this be done for Type, Dec?
        for k in n.kids:
            propagateSigs(k)
        n.sig = SigN
    elif t == "SeqExp":
        n.sig = ListSig(None)
        for k in n.kids:
            propagateSigs(k)
            n.sig.append(k.sig)
    elif t == "Program":
        propagateSigs(n.body)
        n.sig = SigN
    elif t == "Proc":
        propagateSigs(n.body)
        if n.returns:
            n.sig = n.returns
        else:
            n.sig = SigN
    elif t == "Int":
        n.sig = SigI
    elif t == "Str":
        emitLib("S")
        n.sig = SigS
    elif t == "Bool":
        n.sig = SigB
    elif t == "Forward":
        pass
    elif t == "If":
        propagateSigs(n.test)
        n.test.sigCheck(SigB)
        propagateSigs(n.then)
        propagateSigs(n.elze)
        n.sig = SigN
    elif t == "Do":
        propagateSigs(n.test)
        n.test.sigCheck(SigB)
        propagateSigs(n.body)
        n.sig = SigN
    elif t == "Fa":
        #propagateSigs(n.var)
        propagateSigs(n.start)
        n.start.sigCheck(SigI)
        propagateSigs(n.end)
        n.end.sigCheck(SigI)
        propagateSigs(n.body)
        n.sig = SigN
    elif t == "Exit":
        n.sig = SigN
    elif t == "Write":
        propagateSigs(n.exp)
        try:
            n.exp.sigCheck(SigI)
        except SigError:
            try:
                n.exp.sigCheck(SigS)
            except SigError:
                raise SigError(n.token, "write requires int or string")
        n.sig = SigN
    elif t == "Break":
        n.sig = SigN
    elif t == "Return":
        propagateSigs(n.exp)
        n.sig = SigN
    elif t == "Assign":
        propagateSigs(n.lval)
        propagateSigs(n.exp)
        '''
        sig = n.var.sig
        for idx in n.var.under.kids:
            try:
                sig = sig.under
            except AttributeError:
                raise SemanticError(idx.token, 'invalid array index')
            if not idx.sig.check(SigI):
                raise SemanticError(idx.token, 
                                    'array index must be int, not %s',
                                    idx.sig)
        try:
            sig.under
            raise SemanticError(n.token, 'cannot assign into an array')
        except AttributeError:
            pass
        try:
            if sig.check(n.exp.sig):
                n.sig = SigN
            else:
                raise SemanticError(n.token, 'type mismatch %s != %s',
                                    sig, n.exp.sig)
        except AttributeError:
            raise SemanticError(n.token, 'invalid assignment')
        '''
        n.lval.sigCheck(n.exp.sig)
        try:
            n.lval.sig.under
            raise SemanticError(n.token, 'cannot assign arrays')
        except AttributeError:
            pass
        n.sig = None

    elif t == "Binop":
        propagateSigs(n.left)
        propagateSigs(n.right)
        try:
            n.sig = checkBinop(n)
        except ValueError, e:
            raise SemanticError(n.token, str(e))
    elif t == "Uniop":
        propagateSigs(n.exp)
        if n.op == '-':
            # can be int or bool
            try:
                n.exp.sigCheck(SigI)
            except SigError:
                try:
                    n.exp.sigCheck(SigB)
                except SigError:
                    raise SigError(n.token, "uniop %s requires int or bool", op)
            n.sig = n.exp.sig
        elif n.op == '?':
            n.exp.sigCheck(SigB)
            n.sig = SigI
        else:
            raise SemanticError(n.token, 'invalid type of uniop ' + n.op)
    elif t == "Call":
        propagateSigs(n.args)
        # compare param types
        if not n.args:
            args_sig = ListSig(None)
        else:
            args_sig = n.args.sig
        n.sym.sig.params.check(n.token,args_sig)
    elif t == "Idx":
        emitLib("A")
        propagateSigs(n.exp)
        n.exp.sigCheck(SigI)
        propagateSigs(n.under)
        n.size = arrSize(n.under)
        n.sig = n.under.sig.under
    elif t == "Var":
        """
        sig = n.sym.sig
        if n.under:
            print 'varrr', n.under
            for k in n.under.kids:
                if k.type == 'Int':
                    print 'kk', k, k.value
                propagateSigs(k)
                try:
                    if not k.sig.check(SigI):
                        raise SemanticError(n.token, 
                                            "array index expression is not an int: %s", k.sig)
                except AttributeError:
                    raise SemanticError(n.token, 
                                        "invalid array index expression")
                k.sig = sig
                sig = sig.under
        """
        n.sig = n.sym.sig
    elif t =="Lval":
        propagateSigs(n.var)
        n.sig = n.var.sig
        v = n.var
        while v.type == "Idx":
            v = v.under
        if not v.sym.assignable:
            raise SemanticError(n.token, '%s cannot be an l-value', \
                                    v.sym.name)
    elif t =="Rval":
        propagateSigs(n.var)
        n.sig = n.var.sig
    elif t == "Sym":
        pass
    elif t == "Read":
        n.sig = SigI
    elif t == "Nop":
        n.sig = SigN
    else:
        raise SemanticError(n.token, "propogateSigs: invalid type %s", t)

def checkBinop(n):
    op = n.op
    if op in ['*', '+']:
        # must be same
        n.left.sigCheck(n.right.sig)
        # must be int or bool
        try:
            n.left.sigCheck(SigI)
        except SigError:
            try:
                n.left.sigCheck(SigB)
            except SigError:
                raise SigError(n.token, "binop %s requires int or bool", op)
        return n.left.sig
    elif op in ['=', '!=']:
        # must be same
        n.left.sigCheck(n.right.sig)
        # must be int or bool
        try:
            n.left.sigCheck(SigI)
        except SigError:
            try:
                n.left.sigCheck(SigB)
            except SigError:
                raise SigError(n.token, "binop %s requires int or bool", op)
        # result bool
        return SigB
    elif op in [">", "<", ">=", "<="]:
        # must be same
        n.left.sigCheck(n.right.sig)
        # must be int
        n.left.sigCheck(SigI)
        # result bool
        return SigB
    elif op in ['-', '/', '%']:
        # must be same
        n.left.sigCheck(n.right.sig)
        # must be int
        n.left.sigCheck(SigI)
        # return int
        return SigI
    else:
        raise ValueError('invalid binop ' + op)
    raise ValueError('invalid types for binop ' + op)

def arrSize(n):
    def _arrBot(n,k):
        try:
            (m, i) = _arrBot(n.under,k+1)
            return (m, i)
        except AttributeError:
            return (n.sig, k)
    def _arrDim(s):
        try:
            return 1 + _arrDim(s.under)
        except AttributeError:
            return 0
    s, k = _arrBot(n,0)
    u = _arrDim(s)
    for i in range(u-k-1):
        s = s.under
    return s.size
    
def doSemantics(ast, debug, verbose):
    global Debug, Verbose

    Debug = debug
    Verbose = verbose
    propagateSigs(ast)

    # @@@ any other semantic checks? 
    

def emitLib(s=None):
    global EmitLibs
    if s and s not in EmitLibs:
        EmitLibs += s
    return EmitLibs

def main():
    pass

if __name__ == "__main__":
    main()
