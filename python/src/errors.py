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
    pass

class ParseError(CompilerError):
    def __init__(self, la, fmt, *args):
        Exception.__init__(self, *args)
        self.la = la
        self.fmt = fmt
        self.args = args

    def __str__(self):
        return 'ParseError: line %d: ' % (self.la.lineno) \
            + self.fmt % tuple(self.args)
    pass


class SemanticError(CompilerError):
    pass

