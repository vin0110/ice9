#!/usr/bin/python
"""
Driver program for ice9

VW Freeh copyright 2010

CSC 512
"""

#########
# Library imports
#########
import getopt, sys

#########
# Local imports
#########
import lex, tokens
from parser import parse

#########
# Globals
#########
currentToken=None

optstring="dvhO:"
longopts=['debug','verbose','help',
          'optimize=',
          'lexdebug', 'lexoptimize', 'lexonly',
          ]

def usage():
    print sys.argv[0], "-[", optstring, "]"
    print """
  -O, --optimize <n>:	set optimization level

  -d, --debug:		set debug level (more increases level)  
  -v, --verbose:	set verbose level (more increases level)  
  -h, --help:		this message
          """
def main():
    try:
        opts, args = getopt.getopt(sys.argv[1:], optstring, longopts)
    except getopt.GetoptError:
        # print help information and exit:
        usage()
        exit(2)

    # lex arguments
    lex_args = {'module' : tokens, }
    # parse arguments
    parse_args = { }

    opt_level = 0
    debug = 0
    verbose = 0

    for o, a in opts:
        if o in ("-o", "--optimize"):
            try:
                opt_level = int(a)
            except ValueError:
                print "invalid optimzation level", a
                exit(-1)
        elif o in ("-d", "--debug"):
            debug += 1
        elif o in ("-v", "--verbose"):
            verbose += 1
        elif o in ("-h", "--help"):
            usage()
            exit(1)
        elif o == 'lexdebug':
            lex_args['debug'] = 1
        elif o == 'lexoptimize':
            lex_args['optimize'] = 1
        elif o == '--lexonly':
            lexonly = True
        else:
            print "invalid option", o
            exit(1)

    if opt_level > 0:
        lex_args['optimize'] = 1

    lexer=lex.lex(**lex_args)

    try:
        lexonly
        lex.runmain(data=sys.stdin.read())
        exit(0)
    except NameError:
        pass

    try:
        lexer.input(source)
    except NameError:
        lexer.input(sys.stdin.read())

    parse_args['lexer'] = lexer
    parse_args['debug'] = debug
    parse_args['verbose'] = verbose

    parse(**parse_args)

if __name__ == "__main__":
    main()
