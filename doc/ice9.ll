LL(1) Grammar for ice9

program:  'type' type program
	| 'forward' forward program
	| 'proc' proc program
	| 'var' var program
	| stms
	;

stms:	  stm stms
	|
	;

stm:	 'if' if
	| 'do' do
	| 'fa' fa
	| 'break' ';'
	| 'exit' ';'
	| 'return' optexp ';'
	| 'write' exp ';'
	| 'writes' exp ';'
	| 'id' idx
	| exp;
	| ';'
	;

idx:	  '(' explist ')' termx sexpx expx
	| index optassign
	;
optassign: ':=' exp ';'
	| termx sexpx expx
	;

optexp:	  exp
	|
	;

if:	  exp '->' stms elses 'fi'
	;
elses:	  '[]' elsesx
	|
	;
elsesx:	  exp '->' stms elses
	| 'else'  '->' stms
	;

do:	  exp '->' stms 'od'
	;

fa:	  id ':=' exp 'to' exp '->' stms 'af'
	;

proc:	 id '(' declist ')' returns body 'end'
	;
returns: ':' typeid
	|
	;
body:	  'type' type body
	| 'var' var body
	| stms
	|
	;
forward:  id '(' declist ')' returns
	;

var:	  idlist ':' typeid array varx
	;
varx:	  ',' var varx
	|
	;
array:	  '[' int ']' array
	|
	;


idlist:	  id idlistx
	;

idlistx:  ',' id idlistx
	|
	;

type:	  id '=' typeid array
	;

declist:  idlist ':' typeid declistx
	|
	;
declistx: ',' idlist ':' typeid declistx
	|
	;

typeid:   id
	;

exp:	  sexp expx
	;
expx:	  '=' sexp
	| '!=' sexp
	| '>' sexp
	| '<' sexp
	| '>=' sexp
	| '<=' sexp
	|
	;

sexp:	  term sexpx
	;
sexpx:	| '+' sexp sexpx
	| '-' sexp sexpx
	|
	;

term:	  factor termx
	;
termx:	  '*' term termx
	| '/' term termx
	|
	;

factor:   '(' exp ')'
	| int
	| string
	| 'read'
	| '-' factor
	| '?' factor
	| id factorx
	;
factorx:  '(' explist ')'
	| index
	;
index:	  '[' exp ']' index
	|
	;

explist:  exp explistx
	|
	;
explistx: ',' exp explistx
	|
	;
