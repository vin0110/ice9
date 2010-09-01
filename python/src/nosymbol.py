#!/usr/bin/python
"""
dummy symbol table for ice9

VW Freeh copyright 2010

CSC 512
"""

#########
# Library imports
#########

#########
# Local imports
#########

#########
# Globals
#########

#########
# symbols
#########
class Symbol(object):
    def __init__(self, level, location, sig, name='undef'):
        pass
    def __str__(self):
        return "dummy"

class Table(object):
    def __init__(self, name):
        pass

    def __str__(self):
        return 'dummy'

    def push(self):
        pass
    def pop(self):
        return None

    def insert(self, name, sym):
        pass

    def lookup(self, name, local=False):
        return None

#########
# Type signatures
#########
class Sig(object):
    def __init__(self, typ):
        pass

    def __str__(self):
        return 'dummy'

    def check(self, sig):
        return True

class ArrSig(Sig):
    def __init__(self, under):
        pass

class ListSig(Sig):
    def __init__(self, under):
        pass

class ProcSig(Sig):
    def __init__(self, under):
        pass

#########
class Symbols(object):
    def __init__(self):
        v = Table('vars')
        self.vars = v
        p = Table('procs')
        self.procs = p
        t = Table('types')
        self.types = t
    
def main():
    pass

if __name__ == "__main__":
    main()
