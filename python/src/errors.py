#!/usr/bin/python
"""
exceptions for ice9

VW Freeh copyright 2010

CSC 512
"""

#########
# Exceptions
#########
class CompilerError(Exception):
    def __init__(self, tok, fmt, *args):
        super(CompilerError,self).__init__()
        self.tok = tok
        self.fmt = fmt
        self.args = args
        self.name = "CompilerError"

    def __str__(self):
        try:
            self.tok.lineno
            return self.name + ': line %d: ' % (self.tok.lineno) \
                + self.fmt % tuple(self.args)
        except AttributeError:
            return self.name + ': ' + self.fmt % tuple(self.args)

class SigError(CompilerError):
    def __init__(self, tok, fmt, *args):
        super(SigError,self).__init__(tok, fmt, *args)
        self.name = "TypeError"

class ParseError(CompilerError):
    def __init__(self, tok, fmt, *args):
        super(ParseError,self).__init__(tok, fmt, *args)
        self.name = "ParseError"
   
class SemanticError(CompilerError):
    def __init__(self, tok, fmt, *args):
        super(SemanticError,self).__init__(tok, fmt, *args)
        self.name = "SemanticError"
            
class CodeGenError(CompilerError):
    def __init__(self, tok, fmt, *args):
        super(CodeGenError,self).__init__(tok, fmt, *args)
        self.name = "CodeGenError"
