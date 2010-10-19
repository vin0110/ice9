#!/usr/bin/python
"""
codegen for ice9

VW Freeh copyright 2010

CSC 512
"""

"""
REGISTERS
0:
1:
2:
3:
4:
5: GP - top of memory
6: SP
7: PC
8+: temp registers

Data Memory

0:   +--------------+
     | static data  |
     +--------------+
     |		    |
	   ...
     |		    |
     +--------------+
     | stack frames |
     +--------------+
     |   globals    |
top: +--------------+

AR (all components are optional:
     +--------------+
     | return val   |
     +--------------+
     |   params     |
     +--------------+
     |   locals     |
     +--------------+ 
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

from symbol import SigI, SigB, SigS
from parser import getGlobalOffset

from emit import emitFile, emitSkip, emitBackPatch, emit, emitComment, \
    emitData, emitSData, emitDataOffset

#########
# Globals
#########
ArrayBoundCall = None
WriteStringCall = None
reg_gp = 5
reg_sp = 6
reg_pc = 7
from errors import CodeGenError, CompilerError, SigError
__register = 8

def nextReg():
    global __register
    __register += 1
    return __register

def gen(n):
    if n is None:
        return
    t = n.type
    if t in ["Seq", "SeqStm", "IdxList", "SeqVar", \
                 "SeqType", "SeqDec", "SeqExp"]:
        for k in n.kids:
            gen(k)
    elif t == "Program":
        gen(n.body)
    elif t == "Proc":
        raise CompilerError('proc not implemented yet')
        gen(n.body)
    elif t == "Int":
        n.reg = nextReg()
        emit("LDC",n.reg,n.value,0,'int literal')
    elif t == "Str":
        s = emitData(len(n.value), 'string literal - length')
        emitSData(n.value, 'string literal')
        n.reg = nextReg()
        emit("LDC", n.reg, s, 0, 'load addr of string lit')
    elif t == "Bool":
        n.reg = nextReg()
        if n.value:
            emit("LDC",n.reg,1,0,'bool literal - true')
        else:
            emit("LDC",n.reg,0,0,'bool literal - false')
    elif t == "Forward":
        pass
    elif t == "If":
        emitComment("start of IF TEST")
        gen(n.test)
        jumpelse = emitSkip(1)
        emitComment("start of IF THEN")
        gen(n.then)
        jumpend = emitSkip(1)
        emitComment("start of IF ELSE")
        gen(n.elze)
        end_pc = emitSkip(0)
        emitBackPatch(jumpelse,"JEQ",n.test.reg,jumpend-jumpelse,reg_pc,
                      'jump to else')
        emitBackPatch(jumpend,"LDA",reg_pc,end_pc-jumpend,reg_pc, 'jump to end')
    elif t == "Do":
        test_pc = emitSkip(0)
        emitComment("start of DO TEST")
        gen(n.test)
        jumpout = emitSkip(1)
        emitComment("start of DO BODY")
        gen(n.body)
        jumpback = emitSkip(1)
        end_pc = jumpback+1
        emitBackPatch(jumpout, "JEQ", n.test.reg, end_pc-jumpout, reg_pc,
                      'jump out')
        emitBackPatch(jumpback, "LDA", reg_pc, test_pc-jumpback-1, reg_pc,
                      'jump to test')
    elif t == "Fa":
        #gen(n.var)
        emitComment("start of FA: get addr of itervar")
        varAddr(n.var)
        emitComment("calculate start value")
        gen(n.start)
        emit("ST",n.start.reg,0,n.var.reg,'store initial itervar')
        emitComment("calculate end value")
        top = emitSkip(0)
        gen(n.end)
        emit("LD",1,0,n.var.reg,'load itervar')
        emit('SUB', 1, n.end.reg, 1, 'fa:  test')
        jumpout = emitSkip(1)
        gen(n.body)
        emitComment('increment itervar in FA')
        emit("LD",1,0,n.var.reg, 'load itervar')
        emit("LDA", 1, 1, 1, '++itervar')
        emit("ST",1, 0, n.var.reg, 'store itervar')
        emit('LDA', reg_pc, top-emitSkip(0)-1, reg_pc, 'fa:  jump to test exp')
        emitBackPatch(jumpout, "JLT", 1, emitSkip(0)-jumpout-1, reg_pc, 
                      'fa:  jump out')
    elif t == "Exit":
        emit("HALT", 0,0,0,'exit')
    elif t == "Write":
        gen(n.exp)
        try:
            SigI.check(None,n.exp.sig)
            emit("OUT", n.exp.reg, 0, 0, None)
        except SigError:
            try:
                SigS.check(None,n.exp.sig)
                saddr = n.exp.reg
                # use r0 for return pc, r1 for len, r2 for sp
                emit("LD",1,0,saddr, 'write(str): r1 = len')
                emit("LDA",2,1,saddr, 'write(str): r2 = sp')
                emit("LDC",0,emitSkip(0)+2,0, 'store return pc')
                emit("LDA", reg_pc, WriteStringCall-emitSkip(0), reg_pc,
                     'jump to string call')
            except SigError:
                raise CompilerError('FATAL ERROR')
        if n.nl:
            emit("OUTNL", 1,0,0, 'output \\n')
    elif t == "Break":
        raise CompilerError('break not implemented yet')
    elif t == "Return":
        raise CompilerError('return not implemented yet')
    elif t == "Assign":
        gen(n.exp)
        n.lval.reg = n.exp.reg
        gen(n.lval)
    elif t == "Binop":
        gen(n.left)
        gen(n.right)
        op = n.op
        n.reg = nextReg()
        if op == '*':
            emit('MUL',n.reg,n.left.reg,n.right.reg,'binop: *')
        elif op == '+':
            emit('ADD',n.reg,n.left.reg,n.right.reg,'binop: +')
        elif op == '-':
            emit('SUB',n.reg,n.left.reg,n.right.reg,'binop: -')
        elif op == '/':
            emit('DIV',n.reg,n.left.reg,n.right.reg,'binop: *')
        elif op == '%':
            emit('DIV',n.reg,n.left.reg,r.right.reg,'binop: % (i)')
            emit('MUL',n.reg,n.reg,n.right.reg,'binop: % (ii)')
            emit('SUB',n.reg,n.left.reg,n.reg,'binop: % (iii)')
        elif op in ['=','!=','>','<','>=','<=']:
            genLogicalBinOp(n)
    elif t == "Uniop":
        gen(n.exp)
        if n.op == '?':
            n.reg = n.exp.reg
        else:
            n.reg = nextReg()
            try:
                n.sigCheck(SigI)
                # integer uniop minus
                emit("LDC",n.reg,0,0,'uniop int - (i)')
                emit("SUB",n.reg,n.reg,n.exp.reg, 'uniop int - (ii)')
            except SigError:
                # binop uniop minus
                emit("LDC",n.reg,1,0,'uniop bool - (i)')
                emit("SUB",n.reg,n.reg,n.exp.reg, 'uniop bool - (ii)')
    elif t == "Call":
        raise CompilerError('call not implemented yet')
    elif t == "Idx":
        gen(n.exp)
        gen(n.under)
        n.reg = nextReg()
        # check array bounds
        emit("LDA",1,0,n.exp.reg, 'copy index')
        emit("LDC",2,n.size,0, 'load ub')
        emit("LDC",0,emitSkip(0)+2,0, 'load return pc')
        emit("LDA", reg_pc, ArrayBoundCall-emitSkip(0)-1, reg_pc, 
             'jump to bound check')
     
        if n.sig.space() > 1:
            emit("LDC",n.reg,n.sig.space(),0, 'load array stride')
            emit("MUL",n.reg,n.reg,n.exp.reg, 'stride * index')
            emit("SUB",n.reg, n.under.reg, n.reg,'adjust addr')
        else:
            emit("SUB",n.reg, n.under.reg, n.exp.reg,'adjust addr')
    elif t == "Var":
        varAddr(n)
    elif t == 'Rval':
        gen(n.var)
        n.reg = nextReg()
        emit('LD', n.reg, 0, n.var.reg, 'load variable')
    elif t == 'Lval':
        gen(n.var)
        emit('ST', n.reg, 0, n.var.reg, 'store variable')
    elif t == "Read":
        n.reg = nextReg()
        emit("IN",n.reg,0,0,"read value")
    elif t == "Nop":
        pass
    else:
        raise CodeGenError(n.token, "gen: invalid type %s", t)

def varAddr(n):
    try:
        return n.reg
    except AttributeError:
        n.reg = nextReg()
        if n.sym.level < 3:
            # global
            emit('LDA', n.reg, -n.sym.location, reg_gp, 
                 'load base addr of global')
        else:
            raise CompilerError("local vars not handled yet")

    """
    if n.under:
        try:
            n.under.reg
        except AttributeError:
            n.under.reg = nextReg()
            if n.sym.level < 3:
                # global
                emit('LDA', n.under.reg, -n.sym.location, reg_gp, 
                     'load base addr of global array')
            else:
                raise CompilerError("local vars not handled yet")

        try:
            n.reg
        except AttributeError:
            n.reg = nextReg()

        for k in reversed(n.under.kids):
            gen(k)
            try:
                stride = k.sig.under.space()
                if stride == 1: raise AttributeError
                emit("LDC",1,stride,0,'outer stride')
                emit("MUL",1,k.reg,1,'outer index * stride')
                emit("ADD",n.reg,n.reg,1, 'accumulate array offset')
            except AttributeError:
                # stride size is 1 => innermost index
                emit("LDA",n.reg,0,k.reg, 'innermost index: stride=1')
        emit('SUB', n.reg, n.under.reg,n.reg, 
             'subtract array offset from base addr')
    else:
        try:
            # only need to calculate this once if scalar
            n.reg
            return
        except AttributeError:
            if n.sym.level < 3:
                # global
                emit('LDA', n.reg, -n.sym.location, reg_gp, 
                     'load addr of global var')
            else:
                raise CompilerError("local vars not handled yet")
    """
def genLogicalBinOp(n):
    op = n.op
    
    emit('SUB',n.reg, n.left.reg, n.right.reg, 'Logical binop (i)')
    if op == '=':
        inst = 'JEQ'
    elif op == '!=':
        inst = 'JNE'
    elif op == '>':
        inst = 'JGT'
    elif op == '<':
        inst = 'JLT'
    elif op == '>=':
        inst = 'JGE'
    elif op == '<=':
        inst = 'JLE'
    else:
        raise CompilerError("FATAL: invalid binop: " + op)

    emit(inst, n.reg, 2, reg_pc, 'Logical binop (ii) test')
    emit("LDC", n.reg, 0, 0, 'Logical binop (iii) set false')
    emit("LDA", reg_pc, 1, reg_pc, 'Logical binop (iv) jump over true')
    emit("LDC", n.reg, 1, 0, 'Logical binop (v) set true')

def codegen(ast, out, options):
    global ArrayBoundCall, WriteStringCall
    emitFile(out)

    # preamble
    emitComment('set regiseters')
    emit("LD",reg_gp,0,0, 'load memory size into GP')
    emit("LDA",reg_sp,-getGlobalOffset(),reg_gp, 'set stack pointer')
    jump = emitSkip(1)

    # runtime
    emitComment('begin runtime')

    # string print routine
    emitComment('begin write string')
    # input: r0 for return pc r1 for len, r2 for sp -- do not change r0,r1
    # uses r3 for temp values
    WriteStringCall = emit("JEQ", 1, 5, reg_pc, 'write(str): test len == 0')
    emit("LD", 3,0,2, 'r3 = *sp')
    emit("OUTC", 3,3,3, 'out *sp')
    emit("LDA", 2,1,2, '++sp')
    emit("LDA",1,-1,1, '--len')
    emit("LDA",reg_pc,WriteStringCall-emitSkip(0)-1,reg_pc, 'goto top')
    emit("LDA", reg_pc,0,0,'jump back')
    emitComment('end write string')

    # array bounds error
    emitComment('begin array bounds')
    msg_txt = "ERROR: Array index out of bounds"
    msg = emitSData(msg_txt, 'string literal')
    
    #
    error = emit("LDC",1,len(msg_txt),0, 'load size of error message')
    emit("LDC", 2, msg, 0, "load location of error message")
    emit("LDC",0,emitSkip(0)+2,0, 'load return pc (halt inst below)')
    emit("LDA", reg_pc, WriteStringCall-emitSkip(0), reg_pc, 
         'jump to string call')
    emit("OUTNL", 0,0,0,'')
    emit("HALT",0,0,0, 'exit after printing error message')
    # r0 return pc, r1 index, r2 upper
    ArrayBoundCall = emit("JLT",1,error-emitSkip(0)-1,reg_pc, 'test lb')
    emit("SUB",1,2,1, "test ub")
    emit("JLE",1,error-emitSkip(0)-1,reg_pc, 'test ub')
    emit("LDA",7,0,0, "return")
    emitComment('end array bounds')

    emitComment('end runtime')

    emitBackPatch(jump, "LDA", reg_pc, emitSkip(0)-jump-1, reg_pc,
                  'jump over preamble')
    gen(ast)

    # epilogue
