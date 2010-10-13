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
# Global error class
#########
from errors import CompilerError, SemanticError

#########
# Local imports
#########
import lex, tokens
from parser import parse
from semantic import doSemantics
from codegen import codegen

#########
# Globals
#########
currentToken=None

optstring="dvhO:LPTGSo:"
longopts=['debug','verbose','help',
          'optimize=',
          'lexdebug', 'lexoptimize', 'lexonly',
          'parseonly',
          'showast',
          'showsym',
          'nocg',
          'nosemantics',
          'output='
          ]

def usage():
    print sys.argv[0], "-[", optstring, "] [<file>]"
    print """
Read ice9 source from <file> (if present) or standard input.

  -L, --lexonly:	stop after scanning (outputs tokens)
      --lexdebug:	turn on debugging info in lexing (PLY option)
      --lexoptimize:	generate table for lexer (PLY option)

  -P, --parseonly:	stop after parsing (no output on success)
  -T:			stop after parsing and show AST 
			(same as --parseonly --showast)
      --showast:	show AST after parsing phase (does not stop)

  -S, --nocg:		stop after semantic checks (no output on success)
      --showsym:	show symbol table at end of each context (pop)

  -O, --optimize <n>:	set optimization level (only for CG)
  -o, --output <f>:	send output to file; use - for stdout
			default is a.out (only for CG) 

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

    # set default configuration options
    opt_level = debug = verbose = 0
    lexonly=parseonly=showast=showsym=nocg=False
    outfile = 'a.out'

    for o, a in opts:
        if o in ("-o", "--optimize"):
            try:
                opt_level = int(a)
                if 0 < opt_level < 4:
                    print 'optimization: TBD' # @@@
                else:
                    raise ValueError
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
        elif o == '--parseonly':
            parseonly = True
        elif o == '--showast':
            showast = True
        elif o == '--showsym':
            showsym = True
        elif o == '--nocg':
            nocg = True
        elif o == '-L':
            lexonly = True
        elif o == '-P':
            parseonly = True
        elif o == '-T':
            parseonly = showast = True
        elif o == '-S':
            nocg = True
        elif o in ('-o', '--output'):
            outfile = a
        else:
            print "invalid option", o
            exit(1)

    if len(args) == 1:
        #print args[0]
        source = open(args[0],'r')
    elif len(args) > 1:
        print 'too many arguments'
        usage()
        raise CompilerError
    else:
        source = sys.stdin

    if lexonly:
        if parseonly:
            raise CompilerError("incompatible arguments: lexonly and parseonly")
        elif showast:
            raise CompilerError("incompatible arguments: lexonly and showast")
        elif showsym:
            raise CompilerError("incompatible arguments: lexonly and showsym")
        elif nocg:
            raise CompilerError("incompatible arguments: lexonly and nocg")
    elif parseonly:
        if showsym:
            raise CompilerError("incompatible arguments: parseonly and showsym")
        elif nocg:
            raise CompilerError("incompatible arguments: parseonly and nocg")

    if opt_level > 0:
        lex_args['optimize'] = 1

    params = dict(
        source=source,
        opt_level=opt_level,
        debug=debug,
        verbose=verbose,
        lexonly=lexonly,
        parseonly=parseonly,
        showast=showast,
        showsym=showsym,
        nocg=nocg,
        outfile=outfile)
    ice9(**params)

def ice9(source=None,opt_level=0,debug=0,verbose=0,
         lexonly=False,parseonly=False,showast=False,showsym=False,nocg=False,
         outfile='a.out'):
    # set default configuration options
    lex_args = {'module' : tokens, }
    parse_args = { }

    lexer=lex.lex(**lex_args)

    if lexonly:
        lex.runmain(data=sys.stdin.read())
        exit(0)

    lexer.input(source.read())

    parse_args['lexer'] = lexer
    parse_args['debug'] = debug
    parse_args['verbose'] = verbose
    parse_args['nosemantics'] = parseonly
    parse_args['showsym'] = showsym

    if parseonly:
        import nosymbol as symbol
    else:
        import symbol

    Symbols = symbol.Symbols()
    parse_args['symbols'] = Symbols

    if parseonly:
        ast = parse(**parse_args)
        if showast and ast:
            ast.show(0)
        exit(0)

    ast = parse(**parse_args)
    doSemantics(ast, debug, verbose)
    # check if all forward were finished
    for n, sym in Symbols.procs.items():
        if sym.__dict__.has_key('forward') and sym.forward:
            raise SemanticError(None, 'forwarded proc "%s" not defined',n)

    if showast and ast:
        ast.show(0)

    if outfile == '-':
        outstream = sys.stdout
    else:
        outstream = open(outfile, 'w')

    if not nocg:
        codegen(ast, outstream)
    exit(0)

if __name__ == "__main__":
    try:
        main()
        exit(0)
    except (CompilerError, IOError), e:
        print e
        exit(-1)


