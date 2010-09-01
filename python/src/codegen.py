#!/usr/bin/python
"""
codegen for ice9

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
from errors import CompilerError

class CodegenError(CompilerError):
    pass

def codegen(ast, out):
    pass

def main():
    pass

if __name__ == "__main__":
    main()
