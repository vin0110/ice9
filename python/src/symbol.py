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

#########
# Globals
#########

#########
# symbols
#########
class Symbol(object):
    def __init__(self, level, location, sig, name='undef'):
        self.name = name        # just for printing; real name is table
        self.level = level
        self.location = location
        self.sig = sig
        
    def __str__(self):
        return "Sym(%s, %d) %s" % (self.name, self.level, self.sig,)

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
            raise AttributeError(self.__str__() + ': global name "' +\
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

#########
# Type signatures
#########
class Sig(object):
    def __init__(self, typ):
        self.type = typ

    def __str__(self):
        return 'Sig(%s)' % (self.type,)

    def check(self, sig):
        if self.type == sig.type:
            return True
        return False

class ArrSig(Sig):
    def __init__(self, under):
        self.type = 'ArrSig'
        self.under = under

    def __str__(self):
        s = '[]'
        u = self.under
        while u.type == ArrSig:
            s += '[]'
        return u.__str__() + s

    def check(self, sig):
        if self.type == sig.type:
            try:
                return self.under.check(sig.under)
            except AttributeError:
                pass
        return False

class ListSig(Sig):
    def __init__(self, this):
        self.type = 'ListSig'
        self.kids = [this]

    def __str__(self):
        return '[' + ','.join([k.__str__() for k in self.kids]) + ']'

    def append(self, that):
        self.kids.append(that)

    def check(self, sig):
        if self.type == sig.type:
            if len(self.kids) == len(sig.kids):
                for i in range(len(self.kids)):
                    if self.kids[i].type != sig.kids[i].type:
                        return False
                return True
        return False

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

    def check(self, sig):
        if self.type == sig.type:
            if self.returns == self.returns:
                return self.params.check(sig.params)
        return False

#########
class Symbols(object):
    def __init__(self):
        v = Table('vars')
        self.vars = v
        p = Table('procs')
        self.procs = p
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
    
def main():
    pass

if __name__ == "__main__":
    main()
