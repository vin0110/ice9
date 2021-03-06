#!/usr/bin/python
"""
symbol table for ice9

VW Freeh copyright 2010

CSC 512
"""

#########
# Library imports
#########

#########
# Local imports
#########
from errors import SigError

#########
# Globals
#########

#########
# symbols
#########
class Symbol(object):
    '''
    symbol can be a variable for a procedure.

    location for variable is stack offset, for proc it is instruction pointer
    '''
    def __init__(self, level, location, sig, name='undef'):
        self.name = name        # just for printing; real name is table
        self.level = level
        self.location = location
        self.sig = sig
        self.assignable = True
        
    def __str__(self):
        return "Sym(%s, %d): %s" % (self.name, self.level, self.sig,)

class Table(object):
    def __init__(self, name):
        self.name = name
        self.tab = [dict()]
        
    def __str__(self):
        return 'Table(%s)' % self.name

    def level(self):
        return len(self.tab)

    def push(self):
        self.tab.insert(0,dict())
    def pop(self):
        return self.tab.pop(0)

    def insert(self, name, sym):
        if self.tab[0].has_key(sym):
            raise SymbolError('tab(%s.%d): symbol %s already exists',
                              self.name, len(self.tab), name)
        else:
            self.tab[0][name] = sym

    def lookup(self, name, local=False):
        if self.tab[0].has_key(name):
            return self.tab[0][name]
        elif local:
            raise AttributeError(self.__str__() + ': local name "' +\
                                     name + '" not found')
        else:
            for i in range(1,len(self.tab)):
                if self.tab[i].has_key(name):
                    return self.tab[i][name]
            raise AttributeError(self.__str__() + ': name "' +\
                                     name + '" not found')

    def show(self):
        print self.__str__()
        t = self.tab
        L = len(t)
        for i in range(L):
            print ' ', "Level", L-i
            keys = t[i].keys()
            keys.sort()
            for k in keys:
                print '   ', k, ':', t[i][k]

    def items(self):
        I = []
        for t in self.tab:
            I += t.items()
        return I

#########
# Type signatures
#########
class Sig(object):
    def __init__(self, typ):
        self.type = typ

    def __str__(self):
        return '%s' % (self.type,)

    def show(self):
        return 'Sig(%s)' % (self.type,)

    def check(self, token, sig):
        try:
            if self.type == sig.type:
                return True
        except AttributeError:
            pass
        raise SigError(token, "sig mismatch: %s != %s", self.type, sig.type)

    def space(self):
        return 1

class ArrSig(Sig):
    def __init__(self, under, size):
        self.type = 'ArrSig'
        self.under = under
        self.size = size

    def __str__(self):
        s = '[%s]' % self.size
        u = self.under
        while u.type == ArrSig:
            s += '[%d]' % u.size
            u = u.under
        return u.__str__() + s

    def check(self, token, sig):
        print 'sss', self.size, sig.size
        if self.type == sig.type and self.size == sig.size:
            try:
                return self.under.check(sig.under)
            except AttributeError:
                pass
        raise SigError(token, "sig mismatch: %s != %s", self.type, sig.type)

    def space(self):
        return self.size * self.under.space()

class ListSig(Sig):
    def __init__(self, this):
        self.type = 'ListSig'
        if this:
            self.kids = [this]
        else:
            self.kids = []

    def __str__(self):
        return '[' + ','.join([k.__str__() for k in self.kids]) + ']'

    def append(self, that):
        self.kids.append(that)

    def check(self, token, sig):
        if self.type == sig.type:
            if len(self.kids) == len(sig.kids):
                for i in range(len(self.kids)):
                    if not self.kids[i].check(token,sig.kids[i]):
                        raise SigError(token, "sig mismatch: %s != %s", 
                                       self.type, sig.type)

                return True
        raise SigError(token, "sig mismatch: %s != %s", self.type, sig.type)

    def space(self):
        size = 0
        for k in self.kids:
            size += k.size
        return size

class ProcSig(Sig):
    def __init__(self, params, returns):
        self.type = 'ProcSig'
        self.params = params
        self.returns = returns

    def __str__(self):
        s = '%s(%s)' % (self.type, self.params.__str__(),)
        if self.returns:
            return s + ': ' + self.returns.__str__()
        else:
            return s

    def check(self, token, sig):
        if self.type == sig.type:
            if self.returns == self.returns:
                return self.params.check(sig.params)
        raise SigError(token, "sig mismatch: %s != %s", self.type, sig.type)

#########
class Symbols(object):
    def __init__(self):
        # init vars
        v = Table('vars')
        self.vars = v

        # init procs
        p = Table('procs')
        self.procs = p
        int_sym = Symbol(p.level(),
                         None, # @@@ need to set location for CG
                         ProcSig(ListSig(Sig('string')),Sig('int')),
                         name='int')
        p.insert('int', int_sym)
        p.push()

        # init types
        t = Table('types')
        self.types = t
        t.insert('int',Sig('int'))
        t.insert('bool',Sig('bool'))
        t.insert('string',Sig('string'))
        t.push()

    def show(self):
        self.vars.show()
        self.types.show()
        self.procs.show()


#########
# Externals
#########
SigI = Sig('int')
SigB = Sig('bool')
SigS = Sig('string')
    
def main():
    pass

if __name__ == "__main__":
    main()
