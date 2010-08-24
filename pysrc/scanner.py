"""
Scanner for ice9 for csc512

VW Freeh copyright 2010
"""

tokens=(
    'IF',
    'FI',
    'ELSE',
    'DO',
    'OD',
    'FA',
    'AF',
    'TO',
    'PROC',
    'END',
    'RETURN',
    'FORWARD',
    'VAR',
    'TYPE',
    'BREAK',
    'EXIT',
    'TRUE',
    'FALSE',
    'WRITE',
    'WRITES',
    'READ',
    'BOX',
    'ARROW',
    'LPAREN',
    'RPAREN',
    'LBRACK',
    'RBRACK',
    'COLON',
    'SEMI',
    'ASSIGN',
    'QUEST',
    'COMMA',
    'PLUS',
    'MINUS',
    'STAR',
    'SLASH',
    'MOD',
    'EQ',
    'NEQ',
    'GT',
    'LT',
    'GE',
    'LE',
    'INT',
    'ID',
    'SLIT',
    'COMMENT',
    'newline',
    )

t_IF   		= r'if'
t_FI   		= r'fi'
t_ELSE 		= r'else'
t_DO   		= r'do'
t_OD   		= r'od'
t_FA   		= r'fa'
t_AF   		= r'af'
t_TO   		= r'to'
t_PROC 		= r'proc'
t_END  		= r'end'
t_RETURN     	= r'return'
t_FORWARD      	= r'forward'
t_VAR  		= r'var'
t_TYPE 		= r'type'
t_BREAK		= r'break'
t_EXIT 		= r'exit'
t_TRUE 		= r'true'
t_FALSE		= r'false'
t_WRITE		= r'write'
t_WRITES       	= r'writes'
t_READ 		= r'read'
t_BOX  		= r'\[]'
t_ARROW		= r'->'
t_LPAREN       	= r'\('
t_RPAREN       	= r'\)'
t_LBRACK       	= r'\['
t_RBRACK       	= r']'
t_COLON		= r':'
t_SEMI 		= r';'
t_ASSIGN       	= r':='
t_QUEST		= r'\?'
t_COMMA		= r','
t_PLUS 		= r'\+'
t_MINUS		= r'-'
t_STAR 		= r'\*'
t_SLASH		= r'/'
t_MOD  		= r'%'
t_EQ   		= r'='
t_NEQ  		= r'!='
t_GT   		= r'>'
t_LT   		= r'<'
t_GE   		= r'>='
t_LE   		= r'<='
t_ID   		= r'[A-Za-z_][\w_]*'
t_SLIT		= r'(\"([^\\\n]|(\\.))*?\")|(\'([^\\\n]|(\\.))*?\')'

def t_INT(t):
    r'[0-9]+'
    t.value 	= int(t.value)
    return t

def t_COMMEMT(t):
    r'\#.*'
    pass
    # No return value. Token discarded
   
# Define a rule so we can track line numbers
def t_newline(t):
    r'\n+'
    t.lexer.lineno += len(t.value)

# skip white space
t_ignore           = ' \t\x0c'


# Error handling rule
def t_error(t):
    print "Illegal character '%s'" % t.value[0]
    t.lexer.skip(1)

import lex
lexer = lex.lex()

def main():
    lex.runmain(lexer)
                
if __name__ == "__main__":
    main()
