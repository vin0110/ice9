"""
Parser for ice9 for csc512

VW Freeh copyright 2010
"""

#########
# Library imports
#########
import sys
from ast import Seq, StmList, ExpList, TypeList, VarList, \
    Program, DecList, ProcList, \
    Int, Str, Bool, Proc, Forward, \
    If, Do, Fa, Exit, Write, Break, Return, Assign, \
    Binop, Uniop, Call, Var, Sym, Read, Nop, Lval, Rval, Idx

#########
# Local imports
#########
from symbol import Sig, ArrSig, ListSig, ProcSig, Symbol

#########
# Exceptions
#########
from errors import ParseError, SemanticError

class ParseEOF(Exception):
    pass

#########
# Globals
#########
LookAhead=None
Debug = 0
Verbose = 0
InLoop = 0
Lexer = None
TypeCheck=True
ShowSymbolTables=False
LocalOffset=0
GlobalOffset=0

#########
# First sets
#########

FirstExp = ['LPAREN',"INT","SLIT","READ","MINUS","QUEST","ID", "TRUE","FALSE"]
FirstStm_= ['IF','DO',"FA","BREAK","EXIT","RETURN","WRITE","WRITES","SEMI"] 
FirstStm = FirstStm_ + FirstExp

class iLitList(object):
    def __init__(self, ilit):
        self.ilit = ilit
        self.next = None

#########
# Grammar rules
#########

def R_program(L):
    '''
program:  'type' type program
	| 'forward' forward program
	| 'proc' proc program
	| 'var' var program
	| stms
	;
        '''

    debug(R_program)
    if LookAhead.type == 'TYPE':
        consume()
        R_type()
        return R_program(L)
    elif LookAhead.type == 'FORWARD':
        consume()
        R_forward()
        return R_program(L)
    elif LookAhead.type == 'PROC':
        consume()
        proc = R_proc()
        L.append(proc)
        return R_program(L)
    elif LookAhead.type == 'VAR':
        consume()
        R_var()
        return R_program(L)
    else:
        if ShowSymbolTables:
            Symbols.vars.show()
            Symbols.types.show()
            Symbols.procs.show()
        
        stms = R_ostms()
        if LookAhead.type is not "EOF":
            raise ParseError(LookAhead, 'syntax error near %s', LookAhead.type)
        L.append(stms)
        return L

def R_ostms():
    '''
stms:	  stm stmsx
	|
	;
    '''
    debug(R_ostms)
    return R_stmsx(None)

def R_stms():
    debug(R_ostms)
    n = R_stm()
    return R_stmsx(StmList(LookAhead, n))

def R_stmsx(s):
    debug(R_stmsx)
    if LookAhead.type in FirstStm:
        n = R_stm()
        if s:
            s.join(n)
        else:
            s = StmList(LookAhead, n)
        return R_stmsx(s)
    else:
        return s

def R_stm():
    '''
stm:	 'if' if
	| 'do' do
	| 'fa' fa
	| 'break' ';'
	| 'exit' ';'
	| 'return' optexp ';'
	| 'write' exp ';'
	| 'writes' exp ';'
	| id idx ';'
	| exp ';'
	| ';'
	;
    '''
    debug(R_stm)
    if LookAhead.type == "IF":
        consume()
        return R_if()
    elif LookAhead.type == "DO":
        consume()
        return R_do()
    elif LookAhead.type == "FA":
        consume()
        return R_fa()
    elif LookAhead.type in ["BREAK", "EXIT"]:
        e = consume()
        consume("SEMI")
        if e.type == "BREAK":
            if TypeCheck:
                if not InLoop:
                    raise SemanticError(LookAhead, 
                                        'break statement outside of loop')
            return Break(LookAhead)
        else:
            return Exit(LookAhead)
    elif LookAhead.type == "RETURN":
        consume()
        e = R_optexp()
        if TypeCheck and Symbols.procs.level() < 2:
            raise SemanticError(LookAhead, 'return outside of proc')
        return Return(LookAhead, e)
    elif LookAhead.type in ["WRITE", "WRITES"]:
        w = consume()
        e = R_exp()
        nl = w.value != 'writes'
        w = Write(LookAhead, mkRval(e), nl)
        consume("SEMI")
        return w
    elif LookAhead.type == "ID":
        name = consume()
        e = R_idx(name)
        consume("SEMI")
        return mkRval(e)
    elif LookAhead.type in FirstExp:
        e = R_exp()
        consume("SEMI")
        return mkRval(e)
    else:
        consume("SEMI")
        return Nop(LookAhead)

def mkcaller(name, args):
    if TypeCheck:
        try:
            sym = Symbols.procs.lookup(name.value)
        except AttributeError:
            raise SemanticError(LookAhead, 'proc "%s" not found', name.value)
        # do not check param types here; check in semantics.py
        sig = sym.sig.returns
    else:
        sym = name.value
        sig = None
    c = Call(LookAhead, sym, args)
    c.sig = sig
    return c

def R_idx(name):
    '''
idx:	  '(' explist ')' idx2
	| idx2
	| index idx3
        | ':=' exp
	;
    '''
    debug(R_idx)
    if LookAhead.type == 'LPAREN':
        consume()
        args = R_explist()
        c = mkcaller(name, args)
        consume("RPAREN")
        return R_idx2(c)
    elif LookAhead.type == 'ASSIGN':
        consume()
        e = R_exp()
        e = mkRval(e)
        if TypeCheck:
            try:
                var = Symbols.vars.lookup(name.value)
            except AttributeError:
                raise SemanticError(LookAhead, 'variable "%s" not found',
                                    name.value)
        else:
            var = Var(LookAhead, Sym(LookAhead, name.value))
        return Assign(LookAhead, Lval(LookAhead,var), e)
    elif LookAhead.type == 'LBRACK':
        if TypeCheck:
            var = Symbols.vars.lookup(name.value)
        else:
            sym = Sym(LookAhead, name.value)
            var = Var(LookAhead, sym)
        n = R_index(var)
        return R_idx3(n)
    else:
        # return a var
        if TypeCheck:
            try:
                v = Symbols.vars.lookup(name.value)
            except AttributeError:
                raise SemanticError(LookAhead,'variable "%s" not found',
                                 name.value)
        else:
            v = Var(LookAhead, Sym(LookAhead, name.value))
        return R_idx2(v)

def R_idx2(v):
    '''
idx2:	termx sexpx expx
	;
    '''
    debug(R_idx2)
    t = R_termx(v)
    s = R_sexpx(t)
    return R_expx(s)

def R_idx3(v):
    '''
idx3:	  ':=' exp
	| idx2
	;
    '''
    debug(R_idx3)
    if LookAhead.type == 'ASSIGN':
        consume()
        e = R_exp()
        return Assign(LookAhead, Lval(LookAhead,v), mkRval(e))
    else:
        return R_idx2(v)

def R_optexp():
    '''
optexp:	  exp
	|
	;
    '''
    debug(R_optexp)
    if LookAhead.type in FirstExp:
        return R_exp()
    else:
        return None

def R_if():
    '''
if:	  exp '->' stms elses 'fi'
	;
    '''
    debug(R_if)
    test = R_exp()
    consume("ARROW")
    then = R_stms()
    i = If(LookAhead, test, then, None)
    R_elses(i)
    consume("FI")

    return i

def R_elses(parent):
    '''
elses:	  '[]' elsesx
	|
	;
    '''
    debug(R_elses)
    if LookAhead.type == "BOX":
        consume()
        return R_elsesx(parent)
    return parent

def R_elsesx(parent):
    '''
elsesx:	  exp '->' stms elses
	| 'else'  '->' stms
	;
    '''
    debug(R_elsesx)
    if LookAhead.type in FirstStm:
        test = R_exp()
        consume('ARROW')
        then = R_stms()
        this = If(LookAhead, test, then, None)
        R_elses(this)
        if parent:
            parent.elze = this
            return parent
        else:
            return this
    else:
        consume('ELSE')
        consume('ARROW')
        elze = R_stms()
        parent.elze = elze
        return parent

def R_do():
    '''
do:	  exp '->' stms 'od'
	;
    '''
    global InLoop
    debug(R_do)
    test = R_exp()
    consume('ARROW')
    InLoop += 1
    body = R_stms()
    InLoop -= 1
    consume('OD')
    return Do(LookAhead, test, body)

def R_fa():
    '''
fa:	  id ':=' exp 'to' exp '->' stms 'af'
	;
    '''
    global InLoop
    debug(R_fa)
    name = consume("ID")
    consume('ASSIGN')
    start = mkRval(R_exp())
    consume('TO')
    end = mkRval(R_exp())
    consume('ARROW')
    # add itervar AFTER the expressions
    Symbols.vars.push()
    if TypeCheck:
        itervar = Var(LookAhead, \
                  Symbol(Symbols.vars.level(),nextOffset(),Sig('int'), \
                             name=name.value))
        itervar.sym.assignable = False
        Symbols.vars.insert(itervar.sym.name, itervar)
    else:
        itervar = Sym(LookAhead, name.value)
    InLoop += 1
    body = R_stms() 
    InLoop -= 1
    if ShowSymbolTables:
        Symbols.vars.show()
    Symbols.vars.pop()

    consume('AF')
    return Fa(LookAhead, itervar, start, end, body)

def R_proc():
    '''
proc:	 id '(' declist ')' returns body 'end'
	;
    '''
    debug(R_proc)

    # consume declaration (name, params, return type)
    fname = consume("ID")
    consume('LPAREN')
    params = R_declist()
    consume('RPAREN')
    returns = R_returns()

    # do some semantic stuff
    Symbols.types.push()
    Symbols.vars.push()
    if TypeCheck:
        # lookup fname in outermost scope
        try:
            sym = Symbols.procs.lookup(fname.value)
            if sym.__dict__.has_key('forward') and sym.forward:
                forwarded = True
            else:
                raise SemanticError(LookAhead, \
                                       'name clash; proc "%s" already defined',\
                                        fname.value)
        except AttributeError:
            # name is not used
            forwarded = False

        clearLocalOffset()
        if returns:
            try:
                returns_sig = Symbols.types.lookup(returns.value)
            except AttributeError:
                raise SemanticError(LookAhead, 'invalid return type for proc')
            r = Symbol(Symbols.vars.level(),nextOffset(),returns_sig,
                       name=fname.value)
            Symbols.vars.insert(fname.value, Var(LookAhead, r))
        else:
            returns_sig = None

        params_sig = ListSig(None)
        if params:
            for p in params.kids:
                params_sig.append(p.sig)
                try:
                    Symbols.vars.lookup(p.sym.name, local=True)
                    raise SemanticError(LookAhead, 
                                        'redfinition of formal argument "%s"',
                                        p.sym.name)
                except AttributeError:
                    pass
                psym = Symbol(Symbols.vars.level(),nextOffset(),p.sig, 
                              name=p.sym.name)
                Symbols.vars.insert(p.sym.name, Var(LookAhead, psym))

        if forwarded:
            try:
                params_sig.check(LookAhead, sym.sig.params)
            except SigError:
                raise SemanticError(LookAhead, \
                     'parameter mismatch between proc and forward declarations')
            if returns_sig:
                try:
                    returns_sig.check(LookAhead,sym.sig.returns)
                except SigError:
                        raise SemanticError(LookAhead, 'return type conflict between proc and forward declarations')
            else:
                if sym.sig.returns:
                    raise SemanticError(LookAhead, 'return type conflict between proc and forward declarations')
            sym.forward = False
        else:
            sym = Symbol(Symbols.procs.level(), 
                         None,
                         ProcSig(params_sig, returns_sig),
                         name=fname.value)
            Symbols.procs.insert(fname.value, sym)
    else:
        # create dummy symbol
        sym = Symbol(1, None, None)

    # comsume body
    sym.body = R_body()
    
    # cleanup
    if TypeCheck:
        if ShowSymbolTables:
            Symbols.vars.show()
            Symbols.types.show()
        if params:
            k = len(params.kids)
        else:
            k = 0
        if returns:
            k += 1
        psize = (LocalOffset, k)
        Symbols.vars.pop()
        Symbols.types.pop()
    else:
        psize = (0,0)
    
    consume('END')
    p = Proc(fname, fname.value, returns, params, sym, psize)
    return p

def R_returns():
    '''
returns: ':' typeid
	|
	;
    '''
    debug(R_returns)
    if LookAhead.type == "COLON":
        consume()
        typeid = consume("ID")
        return typeid
    else:
        return None


def R_body():
    '''
body:	  'type' type body
	| 'var' var body
	| stms
	|
	;
    '''
    debug(R_body)
    if LookAhead.type == "TYPE":
        consume()
        R_type()
        return R_body()
    elif LookAhead.type == "VAR":
        consume()
        R_var()
        return R_body()
    elif LookAhead.type in FirstStm:
        return R_stms()
    else:
        pass

def R_forward():
    '''
forward:  id '(' declist ')' returns
	;
    '''
    debug(R_forward)
    fname = consume("ID")
    consume('LPAREN')
    params = R_declist()
    consume('RPAREN')
    returns = R_returns()

    if TypeCheck:
        # lookup fname in outermost scope
        try:
            Symbols.procs.lookup(fname.value)
            raise SemanticError(LookAhead, 
                                'name clash; proc "%s" already defined', 
                                fname.value)
        except AttributeError:
            # name is not used
            pass

        params_sig = ListSig(None)
        if params:
            for p in params.kids:
                params_sig.append(p.sig)

        if returns:
            try:
                returns_sig = Symbols.types.lookup(returns.value)
            except AttributeError:
                raise SemanticError(LookAhead, 'invalid return type for proc')
            r = Symbol(Symbols.vars.level(),None,returns_sig,name=fname.value)
            Symbols.vars.insert(fname.value, Var(LookAhead, r))
        else:
            returns_sig = None
        sym = Symbol(Symbols.procs.level(), 
                     None,
                     ProcSig(params_sig, returns_sig),
                     name=fname.value)
        sym.forward = True
        Symbols.procs.insert(fname.value, sym)
        

def R_var():
    '''
var:	  idlist ':' typeid array varx ';'
	;

    This defines a variable. Puts it in the top-level scope. Error if 
    name is already defined.
    '''
    debug(R_var)
    il = R_idlist()
    consume('COLON')
    t = consume("ID")       # typeid
    a = R_array()
    if TypeCheck:
        try:
            g = Symbols.types.lookup(t.value)
        except AttributeError:
            raise SemanticError(LookAhead, 'type not found "%s"', t.value)
        while a:
            if a.ilit.value < 1:
                raise SemanticError(LookAhead, \
                          'cannot define variable with non-positive array size')
            g1 = ArrSig(g, a.ilit.value)
            g = g1
            a = a.next
        
        level = Symbols.vars.level()
        size = g.space()
        for k in il.kids:
            try:
                Symbols.vars.lookup(k.sym.name, local=True)
                raise SemanticError(LookAhead,
                                    'local parameter "%s" re-definition',
                                    k.sym.name)
            except AttributeError:
                pass
            k.sym = Symbol(level, nextOffset(size=size), g, name=k.sym.name)
            Symbols.vars.insert(k.sym.name, k)
    if LookAhead.type == "COMMA":
        consume()
        R_var()
    else:
        consume("SEMI")

def R_varx():
    '''
varx:	  ',' var varx
	|
	;
    '''
    debug(R_varx)
    if LookAhead.type == "COMMA":
        consume()
        R_var()
        R_varx()
    else:
        pass

def R_array():
    '''
array:	  '[' int ']' array
	|
	;

    '''
    debug(R_array)
    if LookAhead.type == "LBRACK":
        consume()
        i = consume("INT")
        consume("RBRACK")
        a = iLitList(Int(i,i.value))
        a.next = R_array()
        return a
    else:
        return None

def R_idlist():
    '''
idlist:	  id idlistx
	;

    '''
    debug(R_idlist)
    name = consume("ID")
    s = VarList(LookAhead, Var(LookAhead,Sym(LookAhead,name.value)))
    return R_idlistx(s)

def R_idlistx(s):
    '''
idlistx:  ',' id idlistx
	|
	;
    '''
    debug(R_idlistx)
    if LookAhead.type == "COMMA":
        consume()
        name = consume("ID")
        v = Var(LookAhead, Sym(LookAhead,name.value))
        if s:
            s.append(v)
        else:
            s = VarList(LookAhead, v)
        return R_idlistx(s)
    else:
        return s

def R_type():
    '''
type:	  id '=' typeid array
	;
    '''
    debug(R_type)
    new = consume("ID")
    consume("EQ")
    t = consume("ID")   # typeid
    a = R_array()
    if TypeCheck:
        # lookup rhs type
        try:
            g = Symbols.types.lookup(t.value)
        except AttributeError:
            raise SemanticError(LookAhead, 'type not found "%s"', t.value)
        # see if lhs type is already defined
        try:
            g = Symbols.types.lookup(new.value, local=True)
            raise SemanticError(LookAhead, 'type clash; "%s" already defined', 
                                new.value)
        except AttributeError:
            while a:
                if a.ilit.value < 1:
                    raise SemanticError(LookAhead, \
                             'cannot define type with non-positive array size')
                g1 = ArrSig(g, a.ilit.value)
                g = g1
                a = a.next
            Symbols.types.insert(new.value, g)
    consume("SEMI")
    return Nop(LookAhead)

def declister():
    il = R_idlist()
    consume('COLON')
    n = consume("ID")   # typeid
    if TypeCheck:
        try:
            g = Symbols.types.lookup(n.value)
        except AttributeError:
            raise SemanticError(LookAhead, 'type not found "%s"', n.value)
    else:
        g = None
    il.type = "Dec" # change idlist into dec list
    for param in il.kids:
        param.sig = g
    return il

def R_declist():
    '''
declist:  idlist ':' typeid declistx
	|
	;
    '''
    debug(R_declist)
    if LookAhead.type == "ID":
        il = declister()
        extra = R_declistx(None)
        if extra:
            for d in extra.kids:
                il.append(d)
        return il
    return None

def R_declistx(dl):
    '''
declistx: ',' idlist ':' typeid declistx
	|
	;
    '''
    debug(R_declistx)
    if LookAhead.type == "COMMA":
        consume()
        il = declister()
        if dl:
            for i in il.kids:
                dl.append(i)
        else:
            dl = il
        return R_declistx(dl)
    else:
        return dl

def R_exp():
    '''
exp:	  sexp expx
	;
    '''
    debug(R_exp)
    e = R_sexp()
    return R_expx(e)


Logical_operators = ['EQ',"NEQ","GT","LT","GE","LE"]

def R_expx(e):
    '''
expx:	  '=' sexp
	| '!=' sexp
	| '>' sexp
	| '<' sexp
	| '>=' sexp
	| '<=' sexp
	|
	;
    '''
    #debug(R_expx, str(e), fmt="%s: %s")
    if LookAhead.type in Logical_operators:
        op = consume()
        e1 = R_sexp()
        return Binop(LookAhead, op.value, e, e1)
    else:
        return e

def R_sexp():
    '''
sexp:	  term sexpx
	;
    '''
    debug(R_sexp)
    t = R_term()
    return R_sexpx(t)

def R_sexpx(t):
    '''
sexpx:	| '+' sexp sexpx
	| '-' sexp sexpx
	|
	;
    '''
    debug(R_sexpx)
    if LookAhead.type in ["PLUS", "MINUS"]:
        op = consume()
        t1 = R_sexp()
        s = Binop(LookAhead, op.value,t,t1)
        return R_sexpx(s)
    else:
        return t

def R_term():
    '''
term:	  factor termx
	;
    '''
    debug(R_term)
    f = R_factor()
    return R_termx(f)

def R_termx(f):
    '''
termx:	  '*' term termx
	| '/' term termx
	| '%' term termx
	|
	;
    '''
    debug(R_termx)
    if LookAhead.type in ["STAR", "SLASH", "MOD"]:
        op = consume()
        f1 = R_term()
        s = Binop(LookAhead, op.value, f, f1)
        return R_termx(s)
    else:
        return f

def R_factor():
    '''
factor:   '(' exp ')'
	| int
	| string
	| 'read'
	| '-' factor
	| '?' factor
        | 'true'
        | 'false'
	| id factorx
	;
    '''
    debug(R_factor)
    if LookAhead.type == "LPAREN":
        consume()
        n = R_exp()
        consume("RPAREN")
        return mkRval(n)
    elif LookAhead.type == "INT":
        i = consume()
        return Int(LookAhead, int(i.value))
    elif LookAhead.type == "SLIT":
        s = consume()
        return Str(LookAhead, s.value.strip('"\''))
    elif LookAhead.type == "READ":
        consume()
        return Read(LookAhead)
    elif LookAhead.type == "TRUE":
        consume()
        return Bool(LookAhead, True)
    elif LookAhead.type == "FALSE":
        consume()
        return Bool(LookAhead, False)
    elif LookAhead.type == "MINUS":
        consume()
        n = R_factor()
        return Uniop(LookAhead, '-',n)
    elif LookAhead.type == "QUEST":
        consume()
        n = R_factor()
        return Uniop(LookAhead, '?',n)
    elif LookAhead.type == "ID":
        i = consume()
        e = R_factorx(i)
        return mkRval(e)
    else:
        raise ParseError(LookAhead, 'syntax error near %s', LookAhead.type)

def R_factorx(name):
    '''
factorx:  '(' explist ')'
	| index
	;
    '''
    debug(R_factorx, name, fmt="%s: %s")
    if LookAhead.type == "LPAREN":
        consume()
        args = R_explist()
        consume("RPAREN")
        c = mkcaller(name, args)
        return c
    else:
        if TypeCheck:
            try:
                sym = Symbols.vars.lookup(name.value)
            except AttributeError, e:
                raise SemanticError(LookAhead, str(e))
            v = sym
            v.sig = sym.sig
        else:
            v = Var(LookAhead, Sym(LookAhead, name.value))
        return R_index(v)

def R_index(n):
    '''
index:	  '[' exp ']' index
	|
	;
    '''
    debug(R_index)
    if LookAhead.type == "LBRACK":
        consume()
        new = Idx(LookAhead,mkRval(R_exp()),n)
        consume("RBRACK")
        return R_index(new)
    else:
        return n

def R_explist():
    '''
explist:  exp explistx
	|
	;
    '''
    debug(R_explist)
    if LookAhead.type in FirstExp:
        e = R_exp()
        s = ExpList(LookAhead, e)
        e1 = R_explistx()
        s.join(e1)
    else:
        s = None
    return s

def R_explistx():
    '''
explistx: ',' exp explistx
	|
	;
        '''
    debug(R_explistx)
    if LookAhead.type == "COMMA":
        consume()
        e = R_exp()
        s = ExpList(LookAhead, e)
        e1 = R_explistx()
        s.join(e1)
    else:
        s = None
    return s

#########
# Support routines
#########

def debug(func, *args, **kwargs):
    fmt=kwargs.get('fmt', "%s")
    val=kwargs.get('val', 1)
    if Debug > val:
        print fmt % ((func.func_name,) + tuple(args))

def EOF(): pass
EOF.type = "EOF"

def consume(tok=None):
    global LookAhead
    if tok and tok != LookAhead.type:
        raise ParseError(LookAhead, "unexpected token; expected %s, got %s",
                         tok,LookAhead.type)
    prev = LookAhead
    LookAhead = Lexer.token()
    if not LookAhead:
        debug(consume, fmt="%s: end of input", val=3)
        EOF.lineno = Lexer.lineno
        LookAhead = EOF
        return prev
    debug(consume, str(LookAhead), fmt="%s: %s", val=2)
    return prev         # @@@ should this return or just set LA?

def clearLocalOffset():
    global LocalOffset
    LocalOffset = 0
def nextOffset(size=1):
    global LocalOffset, GlobalOffset
    if Symbols.vars.level() > 1:
        r = LocalOffset
        LocalOffset += size
    else:
        r = GlobalOffset
        GlobalOffset += size
    return r
def getGlobalOffset():
    '''
    called only by code gen
    '''
    return GlobalOffset

def mkRval(e):
    if e.type in ['Var', 'Idx']:
        return Rval(e.token, e)
    else:
        return e

def parse(lexer=None, debug=0, verbose=0, source=sys.stdin, nosemantics=False,
          showsym=False, symbols=None):
    global Lexer, Debug, Verbose, TypeCheck, ShowSymbolTables, Symbols, \
        BoolT, BoolF
    '''
    Parser for ice9
    '''
    Lexer = lexer
    Debug = debug
    Verbose = verbose
    TypeCheck = not nosemantics
    Symbols = symbols
    ShowSymbolTables = TypeCheck and showsym

    # set LookAhead
    consume()
    return R_program(Seq(LookAhead,None,None))

if __name__ == "__main__":
    pass
