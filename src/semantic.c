/******************************************************************************
 *
 *  File Name........: semantic.c
 *
 *  Description......:
 *
 *  Created by vin on 12/29/09
 *
 *****************************************************************************/

#include "ice9.h"
#include "type.h"
#include "ast.h"
#inclued "symtab.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

void semProgram(Node n)
{
  assert(n);

  switch (n->oper) {
  case O_SEQ:
    /* n_l, n_r */
    semStatement(n->n_l);
    semStatement(n->n_r);
    break;
  case O_PROC:
    /* n_str, n_l, n_r */
    // @@ TBD
    // push symtable
    // walk code
    // pop symtable
    break;
  case O_FORWARD:
  case O_TYPE:
  case O_VAR:
  case O_ERR:
  default:
    CompilerError(n->n_loc, "semProgram: Invalid oper\n");
  }
}

void semStatement(Node n) {
  assert(n);

  switch (n->oper) {
  case O_SEQ:
    /* n_l, n_r */
    semStatement(n->n_l);
    semStatement(n->n_r);
    break;
  case O_EXP:
    /* n_r */
    semExp(n->n_r);
    break;
  case O_ID:
    /* n_str */
    break;
  case O_SYM:
    /* n_sym, n_r */
    break;
  case O_ARR:
    /* n_l, n_r */
    break;
  case O_ILIT:
    /* n_int */
    break;
  case O_SLIT:
    /* n_str */
    break;
  case O_BLIT:
    /* n_int */
    break;
  case O_PROC:
    /* n_str, n_l, n_r */
    break;
  case O_FORWARD:
    /* n_str, n_l */
    break;
  case O_TYPE:
    /* n_str */
    break;
  case O_VAR:
    /* n_r */
    break;
  case O_IF:
    /* n_l, n_r */
    break;
  case O_DO:
    /* n_l, n_r */
    break;
  case O_FA:
    /* n_l, n_r */
    break;
  case O_EXIT:
    /*  */
    break;
  case O_WRITE:
    /* n_int, n_r */
    break;
  case O_READ:
    /*  */
    break;
  case O_BREAK:
    /*  */
    break;
  case O_RETURN:
    /* n_r */
    break;
  case O_ASSIGN:
    /* n_l, n_r */
    break;
  case O_BINOP:
    /* n_binop, n_l, n_r */
    break;
  case O_CALL:
    /* n_l, n_r */
    break;
  case O_QUEST:
    /* n_r */
    break;
  case O_ERR:
  default:
    CompilerError(n->n_loc, "semProgram: Invalid oper\n");
  }
}

void semExp(Node n)
{
  assert(n);
  semStatement(n);
}
/*........................ end of semantic.c ................................*/
#if 0
  case O_EXP:
    /* n_r */
    break;
  case O_ID:
    /* n_str */
    break;
  case O_SYM:
    /* n_sym, n_r */
    break;
  case O_ARR:
    /* n_l, n_r */
    break;
  case O_ILIT:
    /* n_int */
    break;
  case O_SLIT:
    /* n_str */
    break;
  case O_BLIT:
    /* n_int */
    break;
  case O_PROC:
    /* n_str, n_l, n_r */
    break;
  case O_FORWARD:
    /* n_str, n_l */
    break;
  case O_TYPE:
    /* n_str */
    break;
  case O_VAR:
    /* n_r */
    break;
  case O_IF:
    /* n_l, n_r */
    break;
  case O_DO:
    /* n_l, n_r */
    break;
  case O_FA:
    /* n_l, n_r */
    break;
  case O_EXIT:
    /*  */
    break;
  case O_WRITE:
    /* n_int, n_r */
    break;
  case O_READ:
    /*  */
    break;
  case O_BREAK:
    /*  */
    break;
  case O_RETURN:
    /* n_r */
    break;
  case O_ASSIGN:
    /* n_l, n_r */
    break;
  case O_BINOP:
    /* n_binop, n_l, n_r */
    break;
  case O_CALL:
    /* n_l, n_r */
    break;
  case O_QUEST:
    /* n_r */
    break;
#endif
