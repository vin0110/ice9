%{
// $Log: ice9.l,v $
// Revision 1.3  2002/01/18 20:47:09  vin
// Three working phases: all the way to CG'g C code.
// Robustness not verified.
//
// Revision 1.2  2002/01/16 22:14:03  vin
// The basics of semantics are in and working.
//
// Revision 1.1.1.1  2002/01/09 18:02:35  vin
// new ice9
//
// Revision 1.1.1.1  2000/01/13 20:57:25  vin
// Initial import
//

// $Id: ice9.l,v 1.3 2002/01/18 20:47:09 vin Exp $

#include <stdio.h>
#include <ctype.h>
#include "ice9.h"
#include "yhelp.h"
#include "symtab.h"
#include "type.h"
#include "ast.h"
#include "ice9.tab.h"

int yynewlines=0;
// static int input();

static int doid();
static int strlit(char);
// static int chrlit();
//static int escape();
//static void comment();
static void toolong();
static void *ckalloc(unsigned);
%}
%option	yylineno

%%

if	return TK_IF;
fi	return TK_FI;
else	return TK_ELSE;
do	return TK_DO;
od	return TK_OD;
fa	return TK_FA;
af	return TK_AF;
to	return TK_TO;
proc	return TK_PROC;
end	return TK_END;
return	return TK_RETURN;
forward	return TK_FORWARD;
var	return TK_VAR;
type	return TK_TYPE;
"break"	return TK_BREAK;
"exit"	return TK_EXIT;
true	return TK_TRUE;
false	return TK_FALSE;
write	return TK_WRITE;
writes	return TK_WRITES;
read	return TK_READ;
"[]"	return TK_BOX;
"->"	return TK_ARROW;
"("	return TK_LPAREN;
")"	return TK_RPAREN;
"["	return TK_LBRACK;
]	return TK_RBRACK;
:	return TK_COLON;
;	return TK_SEMI;
:=	return TK_ASSIGN;
\?	return TK_QUEST;
,	return TK_COMMA;
"+"	return TK_PLUS;
-	return TK_MINUS;
"*"	return TK_STAR;
"/"	return TK_SLASH;
=	return TK_EQ;
!=	return TK_NEQ;
>	return TK_GT;
"<"	return TK_LT;
">="	return TK_GE;
"<="	return TK_LE;

[0-9]+	{ yylval.intt = atoi(yytext); return TK_INT; };

[A-Za-z][A-Za-z0-9_]*   return doid();		/* identifier */

"\""			return strlit('"');	/* string literal */
"\'"			return strlit('\'');	/* string literal */

#.*	;

[ \t\f\v]		  ;			/* whitespace */
\n			{ yynewlines++; }

.		{ Fatal(LINE, "Illegal charactor (%c)\n", *(char*)yytext); }
%%


static int doid ()
{
#ifdef YYDEBUG
  if (yydebug)
    fprintf (stderr, "`%s' ", yytext);
#endif /*YYDEBUG*/

  if (yyleng >= YYLMAX)
    toolong ();

  yylval.str = (char *) ckalloc(strlen(yytext)+1);
  strcpy(yylval.str, yytext);

  return TK_ID;
}



/*  strlit() -- process a string ("abc") literal  */

static int strlit(char term)
{
  char buf [YYLMAX + 1];
  char *p, *q;
  int c, len;

  p = buf;
  q = p + YYLMAX;
  len = 0;
  for (c = input(); c != term; c = input()) {
    if (c == '\n' || c == '\0') {
      Fatal(LINE, "unterminated string\n");
    }
    if (p < q)
      *p++ = c;
    len++;
  }
  *p++ = '\0';

  if (len > YYLMAX) {
    Fatal(LINE, "string too long\n");
  }

  yylval.str = (char *)ckalloc(len);
  strcpy(yylval.str, buf);

  return TK_SLIT;
}
/*  toolong () -- diagnose a token that is too long.
 *
 *  We don't try to recover; lex has already overwritten memory.
 *  If we're lucky we can issue a diagnostic on the way out.
 */
static void toolong()
{
    write (2, "token too long: ", 16);
    write (2, yytext, 30);
    write (2, "...\n", 4);
    _exit (1);
}

static void *ckalloc(unsigned int l)
{
  void *p = malloc(l);

  if ( p == NULL ) {
    Fatal(LINE, "ckalloc: malloc failed");
  }

  return p;
}

void lexfrom(FILE *fd)
{
  yy_switch_to_buffer (yy_create_buffer (fd, YY_BUF_SIZE));
}
