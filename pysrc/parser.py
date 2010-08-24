"""
Parser for ice9 for csc512

VW Freeh copyright 2010
"""

#########
# Library imports
#########
import sys

#########
# Local imports
#########
from scanner import lexer, tokens

#########
# Exceptions
#########
class ParseEOF(Exception):
    pass

class ParseError(Exception):
    def __init__(self, la, fmt, *args):
        Exception.__init__(self, *args)
        self.la = la
        self.fmt = fmt
        self.args = args

    def __str__(self):
        return 'ParseError: line %d: ' % (self.la.lineno) \
            + self.fmt % tuple(self.args)
    pass

#########
# Globals
#########
LookAhead=None
Debug = 0
Verbose = 0
Lexer = None

#########
# First sets
#########

FirstExp = ['LPAREN',"INT","STRING","READ","MINUS","QUEST","ID"]
FirstStm = ['IF','DO',"FA","BREAK","EXIT","RETURN",
             "WRITE","WRITES","SEMI"] + FirstExp


#########
# Grammar rules
#########

def R_program():
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
        R_program()
    elif LookAhead.type == 'FORWARD':
        consume()
        R_forward()
        R_program()
    elif LookAhead.type == 'PROC':
        consume()
        R_proc()
        R_program()
    elif LookAhead.type == 'VAR':
        consume()
        R_var()
        R_program()
    else:
        R_stms()

    try:
        consume()
        raise ParseError(LookAhead, 'additional tokens')
    except ParseEOF:
        pass

def R_stms():
        '''
stms:	  stm stms
	|
	;
        '''
        debug(R_stms)
        if LookAhead.type in FirstStm:
            R_stm()
        else:
            pass

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
            R_if()
        elif LookAhead.type == "DO":
            consume()
            R_do()
        elif LookAhead.type == "FA":
            consume()
            R_fa()
        elif LookAhead.type in ["BREAK", "EXIT"]:
            consume()
            consume("SEMI")
        elif LookAhead.type == "RETURN":
            consume()
            R_optexp()
        elif LookAhead.type in ["WRITE", "WRITES"]:
            consume()
            R_exp()
            consume("SEMI")
        elif LookAhead.type == "ID":
            consume()
            R_idx()
            consume("SEMI")
        elif LookAhead.type in FirstExp:
            R_exp()
        else:
            consume("SEMI")
            

def R_idx():
        '''
idx:	  '(' explist ')' termx sexpx expx
	| index optassign
	;
        '''
        debug(R_idx)
        if LookAhead.type == 'LPAREN':
            consume()
            R_explist()
            consume("RPAREN")
            R_termx()
            R_sexpx()
            R_expx()
        else:
            R_index()
            R_optassign()

def R_optassign():
        '''
optassign: ':=' exp
	| termx sexpx expx
	;
        '''
        debug(R_optassign)
        if LookAhead.type == "ASSIGN":
            consume()
            R_exp()
        else:
            R_termx()
            R_sexpx()
            R_expx()

def R_optexp():
        '''
optexp:	  exp
	|
	;
        '''
        debug(R_optexp)
        if LookAhead.type in FirstExp:
            R_exp()
        else:
            pass

def R_if():
        '''
if:	  exp '->' stms elses 'fi'
	;
        '''
        debug(R_if)
        R_exp()
        consume("ARROW")
        R_stms()
        R_elses()
        consume("FI")

def R_elses():
        '''
elses:	  '[]' elsesx
	|
	;
        '''
        debug(R_elses)
        if LookAhead.type == "BOX":
            consume()
            R_elsesx()

def R_elsesx():
        '''
elsesx:	  exp '->' stms elses
	| 'else'  '->' stms
	;
        '''
        debug(R_elsesx)
        if LookAhead.type in FirstStm:
            R_exp()
            consume('ARROW')
            R_stms()
            R_elses()
        else:
            consume('else')
            consume('ARROW')
            R_stms()

def R_do():
        '''
do:	  exp '->' stms 'od'
	;
        '''
        debug(R_do)
        R_exp()
        consume('ARROW')
        R_stms()
        consume('OD')

def R_fa():
        '''
fa:	  id ':=' exp 'to' exp '->' stms 'af'
	;
        '''
        debug(R_fa)
        itervar = consume("ID")
        consume('ASSIGN')
        R_exp()
        consume('TO')
        R_exp()
        consume('ARROW')
        R_stms() 
        consume('AF')

def R_proc():
        '''
proc:	 id '(' declist ')' returns body 'end'
	;
        '''
        debug(R_proc)
        fname = consume("ID")
        consume('LPAREN')
        R_declist()
        consume('RPAREN')
        R_returns()
        R_body()
        consume('END')

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
        else:
            pass

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
            R_body()
        elif LookAhead.type == "VAR":
            consume()
            R_var()
            R_body()
        elif LookAhead.type in FirstStm:
            R_stms()
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
        R_declist()
        consume('RPAREN')
        R_returns()

def R_var():
        '''
var:	  idlist ':' typeid array varx
	;
        '''
        debug(R_var)
        R_idlist()
        consume('COLON')
        R_typeid()
        R_array()
        R_varx()

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
            R_int()
            consume("RBRACK")
            R_array()

def R_idlist():
        '''
idlist:	  id idlistx
	;

        '''
        debug(R_idlist)
        name = consume("ID")
        R_idlistx()

def R_idlistx():
        '''
idlistx:  ',' id idlistx
	|
	;
        '''
        debug(R_idlistx)
        if LookAhead.type == "COMMA":
            consume()
            n = consume("ID")
            R_idlistx()
        else:
            pass

def R_type():
        '''
type:	  id '=' typeid array
	;
        '''
        debug(R_type)
        n = consume("ID")
        consume("EQ")
        R_typeid()
        R_array()

def R_declist():
        '''
declist:  idlist ':' typeid declistx
	|
	;
        '''
        debug(R_declist)
        if LookAhead.type == "ID":
            R_idlist()
            consume('COLON')
            R_typeid()
            R_declistx()

def R_declistx():
        '''
declistx: ',' idlist ':' typeid declistx
	|
	;
        '''
        debug(R_declistx)
        if LookAhead.type == "COMMA":
            consume()
            R_idlist()
            consume('COLON')
            R_typeid()
            R_declistx()
        else:
            pass

def R_typeid():
        '''
typeid:   id
	;
        '''
        debug(R_typeid)
        n = consume("ID")

def R_exp():
        '''
exp:	  sexp expx
	;
        '''
        debug(R_exp)
        R_sexp()
        R_expx()

Exp_operators = ['EQ',"NEQ","GT","LT","GE","LE"]

def R_expx():
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
        debug(R_expx)
        if LookAhead.type in Exp_operators:
            op = consume()
            R_sexp()
        else:
            pass

def R_sexp():
        '''
sexp:	  term sexpx
	;
        '''
        debug(R_sexp)
        R_term()
        R_sexpx()

def R_sexpx():
        '''
sexpx:	| '+' sexp sexpx
	| '-' sexp sexpx
	|
	;
        '''
        debug(R_sexpx)
        if LookAhead.type in ["PLUS", "MINUS"]:
            op = consume()
            R_sexp()
            R_sexpx()
        else:
            pass

def R_term():
        '''
term:	  factor termx
	;
        '''
        debug(R_term)
        R_factor()
        R_termx()

def R_termx():
        '''
        termx:	  '*' term termx
	| '/' term termx
	|
	;
        '''
        debug(R_termx)
        if LookAhead.type in ["MINUS", "PLUS"]:
            consume()
            R_term()
            R_termx()
        else:
            pass

def R_factor():
        '''
factor:   '(' exp ')'
	| int
	| string
	| 'read'
	| '-' exp
	| '?' exp
	| id factorx
	;
        '''
        debug(R_factor)
        if LookAhead.type == "LPAREN":
            consume()
            R_exp()
            consume("RPAREN")
        elif LookAhead.type == "INT":
            i = consume()
        elif LookAhead.type == "STRING":
            s = consume()
        elif LookAhead.type == "READ":
            consume()
        elif LookAhead.type == "MINUS":
            consume()
            R_exp()
        elif LookAhead.type == "QUEST":
            consume()
            R_exp()
        elif LookAhead.type == "ID":
            n = consume()
            R_factorx()
        else:
            raise ParseError(LookAhead, 'factor: unexpected token %s', token.type)

def R_factorx():            

        '''
factorx:  '(' explist ')'
	| index
	;
        '''
        debug(R_factorx)
        if LookAhead.type == "LPAREN":
            consume()
            R_explist()
            consume("RPAREN")
        else:
            R_index()

def R_index():
        '''
index:	  '[' exp ']' index
	|
	;
        '''
        debug(R_index)
        if LookAhead.type == "LBRACK":
            consume()
            R_exp()
            consume("RBRACK")
        else:
            pass

def R_explist():
        '''
explist:  exp explistx
	|
	;
        '''
        debug(R_explist)
        if LookAhead.type in FirstExp:
            R_exp()
            R_explistx()
        else:
            pass

def R_explistx():
        '''
explistx: ',' exp explistx
	|
	;
        '''
        debug(R_explistx)
        if LookAhead.type == "COMMA":
            consume()
            R_exp()
            R_explistx()
        else:
            pass

#########
# Support routines
#########

def debug(func, *args, **kwargs):
    fmt=kwargs.get('fmt', "%s")
    val=kwargs.get('val', 1)
    if Debug > val:
        print fmt % ((func.func_name,) + tuple(args))

def consume(tok=None):
    global LookAhead
    if tok and tok != LookAhead.type:
        raise ParseError(LookAhead, "unexpected token; expected %s, got %s",
                         tok,LookAhead.type)
    LookAhead = Lexer.token()
    if not LookAhead:
        debug(consume, fmt="%s: end of input", val=3)
        raise ParseEOF
    debug(consume, str(LookAhead), fmt="%s: %s", val=2)
    return LookAhead         # @@@ should this return or just set LA?

def parse(lexer=None, debug=0, verbose=0):
    global Lexer, Debug, Verbose
    '''
    Parser for ice9
    '''
    Lexer = lexer
    Debug = debug
    Verbose = verbose

    try:
        consume()
        R_program()
    except ParseError, e:
        print e
    except ParseEOF:
        print 'ParseError: end of input'

if __name__ == "__main__":
    parse()
