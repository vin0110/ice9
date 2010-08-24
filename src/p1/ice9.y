%{

#include <stdio.h>
#include "ice9.tab.h"

extern int yynewlines;
extern char *yytext;

int yylex(void); /* function prototype */

void yyerror(char *s)
{
  if ( *yytext == '\0' )
    fprintf(stderr, "line %d: %s near end of file\n", 
	    yynewlines,s);
  else
    fprintf(stderr, "line %d: %s near %s\n",
	    yynewlines, s, yytext);
}

%}


%token TK_IF
%token TK_FI
%token TK_ELSE
%token TK_DO
%token TK_OD
%token TK_FA
%token TK_AF
%token TK_TO
%token TK_PROC
%token TK_END
%token TK_RETURN
%token TK_FORWARD
%token TK_VAR
%token TK_TYPE
%token TK_BREAK
%token TK_EXIT
%token TK_TRUE
%token TK_FALSE
%token TK_WRITE
%token TK_WRITES
%token TK_READ
%token TK_BOX
%token TK_ARROW
%token TK_LPAREN
%token TK_RPAREN
%token TK_LBRACK
%token TK_RBRACK
%token TK_COLON
%token TK_SEMI
%token TK_ASSIGN
%token TK_QUEST
%token TK_COMMA
%token TK_PLUS
%token TK_MINUS
%token TK_STAR
%token TK_SLASH
%token TK_MOD
%token TK_EQ
%token TK_NEQ
%token TK_GT
%token TK_LT
%token TK_GE
%token TK_LE

%token TK_SLIT
%token TK_INT
%token TK_ID

%left TK_SEMI
%nonassoc TK_LBRACK
%nonassoc TK_ASSIGN

%nonassoc TK_EQ TK_NEQ TK_GT TK_LT TK_GE TK_LE
%left TK_PLUS TK_MINUS
%left TK_STAR TK_SLASH TK_MOD
%nonassoc UMINUS TK_QUEST

%start program

%%
program:  decl ostms
	;

// program declarations
decl:	  decl var	
	| decl type
	| decl forward
	| decl proc
	| /* empty */
	;

var:	  TK_VAR varlist
	;

idlist:	  TK_ID TK_COMMA idlist
	| TK_ID
	;

varlist:  varlist TK_COMMA idlist TK_COLON typeid arraydecl
	| idlist TK_COLON typeid arraydecl
	;

arraydecl: arraydecl TK_LBRACK TK_INT TK_RBRACK
	|		/* empty */
	;

typeid:	  TK_ID 
	;

type:	  TK_TYPE TK_ID TK_EQ typeid arraydecl
	;

forward:  TK_FORWARD TK_ID TK_LPAREN declist TK_RPAREN TK_COLON typeid
	| TK_FORWARD TK_ID TK_LPAREN declist TK_RPAREN
	;

// proc decls
pdecl:	  pdecl var
	| pdecl type
	| /* empty */
	;

proc:	  TK_PROC TK_ID TK_LPAREN declist TK_RPAREN TK_COLON typeid 
	  pdecl ostms TK_END		
	| TK_PROC TK_ID TK_LPAREN declist TK_RPAREN
	  pdecl ostms TK_END		
	;

declist:  declst2
	| /* empty */
	;

declst2:  idlist TK_COLON typeid TK_COMMA declst2
	| idlist TK_COLON typeid
	;

// optional stms
ostms:	  stms
	| /* empty */
	;

// one or more stms
stms:	  stm stms
	| stm
	;

stm:      TK_IF exp TK_ARROW stms elses TK_FI   // if
        | TK_DO exp TK_ARROW stms TK_OD                 // do
        | TK_FA TK_ID TK_ASSIGN exp TK_TO exp TK_ARROW stms TK_AF
        | exp TK_SEMI
        | TK_BREAK TK_SEMI
        | TK_EXIT TK_SEMI
        | TK_RETURN TK_SEMI
	| TK_WRITE exp TK_SEMI
	| TK_WRITES exp TK_SEMI
        | lvalue TK_ASSIGN exp TK_SEMI
        | TK_SEMI
	;

elses:	  TK_BOX exp TK_ARROW stms elses
	| TK_BOX TK_ELSE TK_ARROW stms
	|
	;

lvalue:	  lvalue TK_LBRACK exp TK_RBRACK
	| TK_ID	
	;

exp:	  lvalue
	| TK_INT
	| TK_TRUE
	| TK_FALSE
	| TK_SLIT
	| TK_READ
	| TK_MINUS exp %prec UMINUS
	| TK_QUEST exp
	| TK_ID TK_LPAREN TK_RPAREN
	| TK_ID TK_LPAREN explist TK_RPAREN
	| exp TK_PLUS exp
	| exp TK_MINUS exp
	| exp TK_STAR exp
	| exp TK_SLASH exp
	| exp TK_MOD exp
	| exp TK_EQ exp
	| exp TK_NEQ exp
	| exp TK_GT exp
	| exp TK_LT exp
	| exp TK_GE exp
	| exp TK_LE exp
	| TK_LPAREN exp TK_RPAREN

explist:  exp TK_COMMA explist
	| exp
	;

%%

int main(int argc, char *argv[])
{
  yyparse();
  return 0;
}
