"""
AST for ice9

VW Freeh copyright 2010

CSC 512
"""

from errors import SemanticError

INDENT="  "
def indent(i):
    return INDENT*i

class Node(object):
    '''
    Base tree class
    
    the register field is used in register allocation. It is non-null only for
    scalar values stored in registers.
    '''
    def __init__(self, tok, name='Node', sig=None):
        self.type = name
        self.sig = sig
        self.token = tok
        self.register = None
    def __str__(self):
        return self.type
    def show(self, level=0):
        s = indent(level)+self.__str__()
        if self.sig:
            s += ': ' + self.sig.__str__()
        print s
    def sigCheck(self, sig):
        try:
            self.sig.check(self.token,sig)
        except AttributeError:
            raise SemanticError(self.token, "type mismatch: %s != %s",
                                self.sig, sig)

#########        
# list nodes
#########        
class Seq(Node):
    '''
    Sequence.
    Holds a list of like nodes. This is subtyped in several cases.
    '''
    def __init__(self, tok, name, left, **kwargs):
        if name is None:
            super(Seq,self).__init__(tok, 'Seq', **kwargs)
        else:
            super(Seq,self).__init__(tok, 'Seq'+ name, **kwargs)
        if left:
            self.kids = [left]
        else:
            self.kids = []
    def show(self, level=0):
        super(Seq, self).show(level)
        level += 1
        for k in self.kids:
            k.show(level)
    def append(self, n):
        if n:
            self.kids.append(n)
    def join(self, rest):
        if rest:
            if rest.type.startswith("Seq"):
                self.kids += rest.kids
            else:
                self.kids.append(rest)
        return self

class StmList(Seq):
    '''
    List of statements
    '''
    def __init__(self, tok, left, **kwargs):
        super(StmList,self).__init__(tok, 'Stm', left, **kwargs)

class ExpList(Seq):
    '''
    List of statements
    '''
    def __init__(self, tok, left, **kwargs):
        super(ExpList,self).__init__(tok, 'Exp', left, **kwargs)

class TypeList(Seq):
    '''
    List of types
    '''
    def __init__(self, tok, left, **kwargs):
        super(TypeList,self).__init__(tok, 'Type', left, **kwargs)

class VarList(Seq):
    '''
    List of vars
    '''
    def __init__(self, tok, left, **kwargs):
        super(VarList,self).__init__(tok, 'Var', left, **kwargs)

class DecList(Seq):
    '''
    List of decs
    '''
    def __init__(self, tok, left, **kwargs):
        super(DecList,self).__init__(tok, 'Dec', left, **kwargs)

class ProcList(Seq):
    '''
    List of procs
    '''
    def __init__(self, tok, left, **kwargs):
        super(ProcList,self).__init__(tok, 'Proc', left, **kwargs)

#########
# program
#########
class Program(Node):
    '''
    Top level node.  Exactly one per program.
    One child: body (stmlist)
    '''
    def __init__(self, tok, body, **kwargs):
        super(Program,self).__init__(tok, 'Program',**kwargs)
        self.body = body
    def show(self, level=0):
        s = "%s%s" % (indent(level), self.type)
        if self.sig:
            s += ': ' + self.sig.__str__()
        print s
        level += 1
        self.body.show(level)

#########
# literals
#########
from symbol import Sig
IntSig = Sig('int')
BoolSig = Sig('bool')
StringSig = Sig('string')

class Int(Node):
    '''
    Integer literal
    '''
    def __init__(self, tok, value, **kwargs):
        super(Int,self).__init__(tok, 'Int',**kwargs)
        self.value = value
        self.sig = IntSig
    def show(self, level=0):
        s = "%s%s(%d)" % (indent(level), self.type, self.value)
        if self.sig:
            s += ': ' + self.sig.__str__()
        print s
        
class Str(Node):
    '''
    String literal
    '''
    def __init__(self, tok, value, **kwargs):
        super(Str,self).__init__(tok, 'Str',**kwargs)
        self.value = value
        self.sig = StringSig
    def show(self, level=0):
        s = "%s%s(%s)" % (indent(level), self.type, self.value)
        if self.sig:
            s += ': ' + self.sig.__str__()
        print s

class Bool(Node):
    '''
    Boolean literal
    '''
    def __init__(self, tok, value, **kwargs):
        super(Bool,self).__init__(tok, 'Bool',**kwargs)
        self.value = value
        self.sig = BoolSig
    def show(self, level=0):
        s = "%s%s(%s)" % (indent(level), self.type, str(self.value))
        if self.sig:
            s += ': ' + self.sig.__str__()
        print s

#########
# procedure
#########
class Proc(Node):
    '''
    procedure has 4 children
    name 	: id|sym
    returns 	: typeid
    params 	: typelist
    body	: stmList
    '''
    def __init__(self, tok, name, returns, params, sym, size, **kwargs):
        super(Proc,self).__init__(tok, 'Proc',**kwargs)
        self.name = name
        self.returns = returns
        self.params = params
        self.sym = sym
        self.size = size
        self.body = sym.body
        self.forward = False

    def show(self, level=0):
        s = "%s%s(%s)" % (indent(level), self.type, self.name)
        if self.returns:
            s +=  ' : ' + self.returns.value
        if self.sig:
            s += ': ' + self.sig.__str__()
        print s
        level += 1
        if self.params:
            self.params.show(level)
        else:
            print '%s[]' % (indent(level))
        if self.body:           # forward has no body
            self.body.show(level)

class Forward(Proc):
    '''
    forward is a procedure with no body
    '''
    def __init__(self, tok, name, returns, params, **kwargs):
        super(Forward,self).__init__(tok, 'Forward',name,returns,params,None,
                                     **kwargs)

#########
# Statements
#########
class If(Node):
    '''
    three children: test, then, else (else can be empty)
    no elif; collapse everything into if.
    '''
    def __init__(self, tok, test, then, elze, **kwargs):
        super(If,self).__init__(tok, 'If',**kwargs)
        self.test = test
        self.then = then
        self.elze = elze
    def show(self, level=0):
        s = indent(level) + self.__str__()
        if self.sig:
            s += ': ' + self.sig.__str__()
        print s
        level += 1
        self.test.show(level)
        self.then.show(level)
        if self.elze:
            self.elze.show(level)

class Do(Node):
    '''
    two children: test and body
    '''
    def __init__(self, tok, test, body, **kwargs):
        super(Do,self).__init__(tok, 'Do',**kwargs)
        self.test = test
        self.body = body
    def show(self, level=0):
        s = self.__str__()
        if self.sig:
            s += ': ' + self.sig.__str__()
        print s
        level += 1
        self.test.show(level)
        self.body.show(level)

class Fa(Node):
    '''
    '''
    def __init__(self, tok, var, start, end, body, **kwargs):
        super(Fa,self).__init__(tok, 'Fa',**kwargs)
        self.var = var
        self.start = start
        self.end = end
        self.body = body
    def show(self, level=0):
        s = indent(level) + self.__str__()
        if self.sig:
            s += ': ' + self.sig.__str__()
        print s
        level += 1
        print indent(level) + str(self.var)
        self.start.show(level)
        self.end.show(level)
        self.body.show(level)

class Exit(Node):
    def __init__(self, tok, **kwargs):
        super(Exit,self).__init__(tok,'Exit',**kwargs)

class Write(Node):
    def __init__(self, tok, exp, nl, **kwargs):
        super(Write,self).__init__(tok,'Write',**kwargs)
        self.exp = exp
        self.nl = nl
    def show(self, level=0):
        super(Write, self).show(level)
        self.exp.show(level+1)

class Break(Node):
    def __init__(self, tok, **kwargs):
        super(Break,self).__init__(tok,'Break',**kwargs)

class Return(Node):
    def __init__(self, tok, exp, **kwargs):
        super(Return,self).__init__(tok,'Return',**kwargs)
        self.exp = exp
    def show(self, level=0):
        super(Return, self).show(level)
        if self.exp:
            self.exp.show(level+1)

class Assign(Node):
    def __init__(self, tok, val, exp, **kwargs):
        super(Assign,self).__init__(tok,'Assign',**kwargs)
        self.lval = val
        self.exp = exp
    def show(self, level=0):
        super(Assign, self).show(level)
        level += 1
        self.lval.show(level)
        self.exp.show(level)

#########
# Expressions
#########
class Lval(Node):
    def __init__(self, tok, var, **kwargs):
        super(Lval,self).__init__(tok,'Lval',**kwargs)
        self.var = var
    def show(self, level=0):
        s = indent(level) + self.__str__()
        if self.sig:
            s += ": " + self.sig.__str__()
        print s
        level += 1
        self.var.show(level)

class Rval(Node):
    def __init__(self, tok, var, **kwargs):
        super(Rval,self).__init__(tok,'Rval',**kwargs)
        self.var = var
    def show(self, level=0):
        s = indent(level) + self.__str__()
        if self.sig:
            s += ": " + self.sig.__str__()
        print s
        level += 1
        self.var.show(level)

class Binop(Node):
    def __init__(self, tok, op, left, right, **kwargs):
        super(Binop,self).__init__(tok,'Binop',**kwargs)
        self.op = op
        self.left = left
        self.right = right
    def show(self, level=0):
        s = "%s%s(%s)" % (indent(level), self.__str__(), self.op)
        if self.sig:
            s += ": " + self.sig.__str__()
        print s
        level += 1
        self.left.show(level)
        self.right.show(level)

class Uniop(Node):
    def __init__(self, tok, op, exp, **kwargs):
        super(Uniop,self).__init__(tok,'Uniop',**kwargs)
        assert op in "-?"
        self.op = op 
        self.exp = exp
    def show(self, level=0):
        s = "%s%s(%s)" % (indent(level), self.__str__(), self.op)
        if self.sig:
            s += ": " + self.sig.__str__()
        print s
        self.exp.show(level+1)

class Idx(Node):
    '''
    Index of a variable.  
    '''
    def __init__(self, tok, exp, under, **kwargs):
        super(Idx,self).__init__(tok,'Idx',**kwargs)
        self.exp = exp
        self.under = under
        self.var = None
        self.depth = 0
    def show(self, level=0):
        s = "%s%s" % (indent(level), self.type)
        if self.sig:
            s += ': ' + self.sig.__str__()
        print s
        self.exp.show(level+1)
        if self.under:
            self.under.show(level+1)
            
    def __str__(self):
        try:
            return "%s(%s)" %(self.type, self.sym.name,)
        except AttributeError:
            return super(Idx,self).__str__()
    

class Var(Node):
    '''
    Variable.  
    '''
    def __init__(self, tok, sym, **kwargs):
        super(Var,self).__init__(tok,'Var',**kwargs)
        self.sym = sym
    def show(self, level=0):
        s = "%s%s(%s)" % (indent(level), self.type, self.sym.name)
        if self.sig:
            s += ': ' + self.sig.__str__()
        print s
    def __str__(self):
        try:
            return "%s(%s)" %(self.type, self.sym.name,)
        except AttributeError:
            return super(Var,self).__str__()

class Sym(Node):
    '''
    Fake node for use when not doing symantic analysis
    '''
    def __init__(self, tok, name, **kwargs):
        super(Sym,self).__init__(tok,'Sym',**kwargs)
        self.name = name
    def __str__(self):
        return 'Sym(%s)' % (self.name,)
    def show(self, level=0):
        s = indent(level) + self.__str__()
        if self.sig:
            s += ": " + self.sig.__str__()
        print s

class Call(Node):
    def __init__(self, tok, sym, args, **kwargs):
        super(Call,self).__init__(tok,'Call',**kwargs)
        self.sym = sym
        self.args = args
    def show(self, level=0):
        s = "%s%s(%s)" % (indent(level), self.__str__(), self.sym)
        if self.sig:
            s += ": " + self.sig.__str__()
        print s
        if self.args:
            self.args.show(level+1)

class Read(Node):
    def __init__(self, tok, **kwargs):
        super(Read,self).__init__(tok,'Read',**kwargs)
        self.sig = IntSig

class Nop(Node):
    def __init__(self, tok, **kwargs):
        super(Nop,self).__init__(tok,'Nop',**kwargs)


if __name__ == "__main__":
    pass
