"""
Tokens for ice9

VW Freeh copyright 2010

CSC 512
"""

reserved = {
    'if'		: 'IF',
    'fi'		: 'FI',
    'else'		: 'ELSE',
    'do'		: 'DO',
    'od'		: 'OD',
    'fa'		: 'FA',
    'af'		: 'AF',
    'to'		: 'TO',
    'proc'		: 'PROC',
    'end'		: 'END',
    'return'		: 'RETURN',
    'forward'		: 'FORWARD',
    'var'		: 'VAR',
    'type'		: 'TYPE',
    'break'		: 'BREAK',
    'exit'		: 'EXIT',
    'true'		: 'TRUE',
    'false'		: 'FALSE',
    'write'		: 'WRITE',
    'writes'		: 'WRITES',
    'read'		: 'READ',
}

tokens=[
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
    ] + list(reserved.values())

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
t_SLIT		= r'(\"([^\\\n]|(\\.))*?\")|(\'([^\\\n]|(\\.))*?\')'

def t_INT(t):
    r'[0-9]+'
    t.value 	= int(t.value)
    return t

def t_COMMEMT(t):
    r'\#.*'
    pass
    # No return value. Token discarded

def t_ID(t):
    r'[a-zA-Z_][a-zA-Z_0-9]*'
    t.type = reserved.get(t.value,'ID')    # Check for reserved words
    return t
   
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

if __name__ == "__main__":
    import lex
    lexer = lex.lex()
    lex.runmain(lexer)
