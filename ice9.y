%{
// $Log: ice9.y,v $
// Revision 1.4  2002/01/18 20:47:09  vin
// Three working phases: all the way to CG'g C code.
// Robustness not verified.
//
// Revision 1.3  2002/01/16 22:14:03  vin
// The basics of semantics are in and working.
//
// Revision 1.2  2002/01/10 00:56:24  vin
// symbol table files.
//
// Revision 1.1.1.1  2002/01/09 18:02:35  vin
// new ice9
//
// Revision 1.1.1.1  2000/01/13 20:57:25  vin
// Initial import
//

// $Id: ice9.y,v 1.4 2002/01/18 20:47:09 vin Exp $

#include <stdio.h>
#include <stdlib.h>
#include "ice9.h"
#include "list-pack.h"
#include "ast.h"
#include "symtab.h"
#include "type.h"
#include "yhelp.h"
#include "cg.h"

//#define YY_MAIN

extern char *FileName;
extern int yynewlines;
extern char *yytext;
//extern FILE *yyin;
extern lexfrom(FILE *);

//static ModuleSymbol *module = NULL;

int yylex(void); /* function prototype */
static Node Root = NULL;
Table Vars, Types, Procs;
Sig Gint, Gbool, Gstr, Gnil, Gerr;
int SigPrint = 0;

void yyerror(char *s)
{
  if ( *yytext == '\0' )
    fprintf(stderr, "%s: line %d: %s near end of file\n", FileName, 
	    yynewlines+1,s);
  else
    fprintf(stderr, "%s: line %d: %s near %s\n", FileName, 
	    yynewlines+1, s, yytext);
}

%}

%union {
  int intt;
  char *str;
  ylist list;
  Node node;
  Symbol sym;
  Sig sig;
}

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
%token TK_EQ
%token TK_NEQ
%token TK_GT
%token TK_LT
%token TK_GE
%token TK_LE

%token <str> TK_SLIT
//%token TK_SLIT
%token <intt> TK_INT
//%token TK_INT
%token <str> TK_ID
//%token TK_ID

// token type casts
%type <node> exp explist ostms stms stm elses lvalue typeid declist declst2
%type <node> decl type forward proc pdecl
%type <node> var varlist
%type <list> idlist arraydecl

%left TK_SEMI
%nonassoc TK_LBRACK
%right TK_ASSIGN

%nonassoc TK_EQ TK_NEQ TK_GT TK_LT TK_GE TK_LE
%left TK_PLUS TK_MINUS
%left TK_STAR TK_SLASH
%nonassoc UMINUS

%start program

%%
program:  decl ostms
		{ 
		  Root = mkSeq($1, $2);
		}
	;

// program declarations
decl:	  decl var	 { if ($1) $$ = mkSeq($1, $2); else $$ = $2; }
	| decl type	 { if ($1) $$ = mkSeq($1, $2); else $$ = $2; }
	| decl forward 	 { if ($1) $$ = mkSeq($1, $2); else $$ = $2; }
	| decl proc	 { if ($1) $$ = mkSeq($1, $2); else $$ = $2; }
	| /* empty */	 { $$ = NULL; }
	;

var:	  TK_VAR varlist  { $$ = $2; }
	;

idlist:	  TK_ID TK_COMMA idlist
		{ $$ = yList(symStr($1), $3); }
	| TK_ID		{ $$ = yList(symStr($1), NULL); }
	;

varlist:  varlist TK_COMMA idlist TK_COLON typeid arraydecl
		{ $$ = mkSeq($1, yInsertVar($3, $5, $6)); }
	| idlist TK_COLON typeid arraydecl
		{ $$ = yInsertVar($1, $3, $4); }
	;

arraydecl: arraydecl TK_LBRACK TK_INT TK_RBRACK
	   { $$ = yList((void *)$3, $1); }
	|  { $$ = NULL; }		/* empty */
	;

typeid:	  TK_ID { $$ = yTypeid($1);}
	;

type:	  TK_TYPE TK_ID TK_EQ typeid arraydecl
		{ $$ = yInsertType($2, $4, $5); }

forward:  TK_FORWARD TK_ID TK_LPAREN declist TK_RPAREN TK_COLON typeid
		{ $$ = yForward($2, $4, $7); }
	| TK_FORWARD TK_ID TK_LPAREN declist TK_RPAREN
		{ $$ = yForward($2, $4, NULL); }
	;

// proc decls
pdecl:	  pdecl var	{ if ($1) $$ = mkSeq($1, $2); else $$ = $2; }
	| pdecl type	{ if ($1) $$ = mkSeq($1, $2); else $$ = $2; }
	| /* empty */	{ $$ = NULL; }
	;

	  //proc1 proc2 { $1->n_r = $2; $$ = $1; }

proc:	  TK_PROC TK_ID TK_LPAREN declist TK_RPAREN TK_COLON typeid 
		{ $<node>$ = yProcPre($2, $4, $7); }
	  pdecl ostms TK_END		
		{ $<node>8->n_r = yProcPost($9, $10); $$ = $<node>8; }
	| TK_PROC TK_ID TK_LPAREN declist TK_RPAREN
		{ $<node>$ = yProcPre($2, $4, NULL); }
	  pdecl ostms TK_END		
		{ $<node>6->n_r = yProcPost($7, $8); $$ = $<node>6; }
	;

declist:  declst2	{ $$ = $1; }
	| /* empty */	{ $$ = NULL; }		
	;

declst2:  idlist TK_COLON typeid TK_COMMA declst2
		{ $$ = yDeclist($1, $3, $5); }
	| idlist TK_COLON typeid
		{ $$ = yDeclist($1, $3, NULL); }
	;

// optional stms
ostms:	  stms 		{ $$ = $1; }
	| /* empty */	{ $$ = NULL; }
	;

// one or more stms
stms:	  stm stms 	{ $$ = mkSeq($1, $2); }
	| stm		{ $$ = $1; }
	;

stm:      TK_IF exp TK_ARROW stms elses TK_FI   // if
		{ $$ = mkIf($2, $4, $5); }
        | TK_DO exp TK_ARROW stms TK_OD                 // do
		{ $$ = mkDo($2, $4); }
        | TK_FA TK_ID { yFa($2); } TK_ASSIGN exp TK_TO exp TK_ARROW stms TK_AF
		{ $$ = mkFa(mkId($2), $5, $7, $9); }
        | exp TK_SEMI
		{ $$ = mkExp($1); }
        | TK_BREAK TK_SEMI
		{ $$ = mkBreak(NULL); }
        | TK_EXIT TK_SEMI
		{ $$ = mkExit(NULL); }
        | TK_RETURN TK_SEMI
		{ $$ = mkReturn(NULL); }
        | TK_RETURN exp TK_SEMI
		{ $$ = mkReturn($2); }
	| TK_WRITE exp TK_SEMI
		{ $$ = mkWrite(1, $2); }
	| TK_WRITES exp TK_SEMI
		{ $$ = mkWrite(0, $2); }
        | lvalue TK_ASSIGN exp TK_SEMI
		{ $$ = mkAssign($1, $3); }
        | TK_SEMI               // the "empty" statement
		{ $$ = NULL; } // @@@ hmm...what should be returned?

elses:	  TK_BOX exp TK_ARROW stms elses
		{ $$ = mkIf($2, $4, $5); }
	| TK_BOX TK_ELSE TK_ARROW stms
		{ $$ = $4; }
	| 	{ $$ = NULL; }		/* empty */
	;

lvalue:	  lvalue TK_LBRACK exp TK_RBRACK
		{ $$ = mkArr($1, $3); }
	| TK_ID	
		{ $$ = yLval(mkId($1)); }
	;

exp:	  lvalue	{ $$ = $1; }
	| TK_INT	{ $$ = mkIlit($1); }
	| TK_TRUE	{ $$ = mkBlit(1); }
	| TK_FALSE	{ $$ = mkBlit(0); }
	| TK_SLIT	{ $$ = mkSlit($1); }
	| TK_READ	{ $$ = mkRead(); }
	| TK_MINUS exp %prec UMINUS
		{ $$ = mkBinop(B_SUB, mkIlit(0), $2); }
	| TK_QUEST exp
		{ $$ = $2; } // @@@ TBD
	| TK_ID TK_LPAREN TK_RPAREN
		{ $$ = yCall($1, NULL); }
	| TK_ID TK_LPAREN explist TK_RPAREN
		{ $$ = yCall($1, $3); }
	| exp TK_PLUS exp
		{ $$ = mkBinop(B_ADD, $1, $3); }
	| exp TK_MINUS exp
		{ $$ = mkBinop(B_SUB, $1, $3); }
	| exp TK_STAR exp
		{ $$ = mkBinop(B_MUL, $1, $3); }
	| exp TK_SLASH exp
		{ $$ = mkBinop(B_DIV, $1, $3); }
	| exp TK_EQ exp
		{ $$ = mkBinop(B_EQ, $1, $3); }
	| exp TK_NEQ exp
		{ $$ = mkBinop(B_NEQ, $1, $3); }
	| exp TK_GT exp
		{ $$ = mkBinop(B_GT, $1, $3); }
	| exp TK_LT exp
		{ $$ = mkBinop(B_LT, $1, $3); }
	| exp TK_GE exp
		{ $$ = mkBinop(B_GE, $1, $3); }
	| exp TK_LE exp
		{ $$ = mkBinop(B_LE, $1, $3); }
	| TK_LPAREN exp TK_RPAREN	{ $$ = $2; }

explist:  exp TK_COMMA explist		{ $$ = mkSeq($1, $3); }
	| exp				{ $$ = $1; }
	;

%%
static xnodePrint(void *a, void *b)
{
  nodePrint((int)b, (Node)a);
}

extern char *optarg;
extern int optind, opterr, optopt;
int getopt(int argc, char * const argv[], const char *optstring);

#define OPTS	"tgno:c"

static void usage(char *prog)
{
  fprintf(stderr, "%s [%s] <filename>\n", prog, OPTS);
  exit(-1);
}

main(int argc, char *argv[])
{
  char str[128];
  char *outfile = str;
  char *cout;
  int c;
  FILE *fd;
  int treePrint=0, genCode = 1, toC = 0;

  while ( (c = getopt(argc, argv, OPTS)) != EOF ) {
    switch ( c ) {
    case 't':	treePrint = 1;			break;
    case 'n':	genCode = 0;			break;
    case 'c':	toC = 1;			break;
    case 'g':	SigPrint = treePrint = 1;	break;
    case 'o':   outfile = optarg;		break;
    default:
      usage(argv[0]);
    }
  }
  if (toC && !genCode) {
    fprintf(stderr, "arg c not compatible with arg n\n");
    usage(argv[0]);
  }
  if (optind == argc) {
    FileName = "<stdin>";
  }
  else if (optind >= argc ) {
    usage(argv[0]);
  }
  else {
    FileName = argv[optind];
    fd = fopen(FileName, "r");
    if (fd == NULL) {
      perror("couldn't open source file");
      exit(1);
    }
    lexfrom(fd);
    //    yyin = fd;
  }

  Vars = tabMake();
  tabPush(Vars);

  Procs = tabMake();
  tabPush(Procs);

  Types = tabMake();
  Gint = sigMake(T_INT);
  symInsert(Types, symMake(symStr("int"), Gint));
  Gbool = sigMake(T_BOOL);
  symInsert(Types, symMake(symStr("bool"), Gbool));
  Gstr = sigMake(T_STR);
  symInsert(Types, symMake(symStr("string"), Gstr));
  Gnil = sigMake(T_NIL);
  Gerr = sigMake(T_ERR);
  tabPush(Procs);

  if (yyparse()) {
    exit(1);
  }

  if (treePrint) {
    nodePrint(0, Root);
  }
  if (genCode) {
    int len;

    // create output
    close(fd);
    if (toC) {
      if (outfile == str) {
	strcpy(str, FileName);
	len = strlen(str);
	if (str[len-2] == '.' && str[len-1] == '9') {
	  str[len-2] = '\0';
	}
      }
      cout = outfile;
      outfile = "_ice9.c";
    }
    else if (outfile == str) {
      strcpy(str, FileName);
      len = strlen(str);
      if (str[len-2] == '.' && str[len-1] == '9') {
	str[len-2] = '\0';
      }
      strcat(str, ".c");
    }
    fd = fopen(outfile, "w");
    if (fd == NULL) {
      char buf[128];
      sprintf(buf, "couldn't open output file %s", outfile);
      perror(buf);
      exit(-1);
    } 
    cgGen(fd, Root);
    fclose(fd);
    if (toC) {
      char buf[128];
      sprintf(buf, "gcc -g -o %s %s", cout, outfile);
      printf("%s\n", buf);
      system(buf);
      //unlink(outfile);
    }
  }
}
