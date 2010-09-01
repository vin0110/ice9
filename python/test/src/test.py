#!/usr/bin/python

import sys, os
from sys import exit
import getopt
from tempfile import mkstemp

optstring="c:rqdvh"
longopts=['config=', 'no-config',
    'quiet', 'debug', 'verbose', 'help']

Verbose, Debug, Quiet = 0, 0, False

def usage():
    print sys.argv[0], "-[", optstring, "]"
    print """
  -c, --config <f>:	use <f> for config file (default: ice9.cfg)
      --no-config:	do not load a config file
  -r, --recursive:	recurse through dir and all sub dirs
  -q, --quiet:		suppress basic output messages
  -d, --debug:		set debug level (more increases level)
  -v, --verbose:	set verbose level (more increases level)
  -h, --help:		this message
          """

BASE_PARTS=['/','home','vin','Classes','ice9','python']
BASE_DIR=os.path.join(*BASE_PARTS)
TEST_DIR=os.path.join(BASE_DIR,"test")
COMPILER="python " + os.path.join(BASE_DIR,"src","ice9.py")

GLOBAL_CFG='ice9.cfg'

class TestError(Exception):
    def __init__(self, value):
        self.value = value
    def __str__(self):
        return repr(self.value)

def readconfig(f):
    config = {}
    for line in f.readlines():
        line = line.strip()
        if len(line) == 0: continue
        if line.startswith('#'): continue
        a,b = line.split('=')
        a = a.strip(); b = b.strip()
        if config.has_key(a):
            raise TestError('multiple definitions of key "%s"', (a,))
        if b == '' or b.lower() == 't' or b.lower() == 'true':
            config[a] = True
        elif b.lower() == 'f' or b.lower() == 'false':
            config[a] = False
        else:
            try:
                b = int(b)
            except ValueError:
                pass
            config[a] = b
    return config
        
def runtest(test, inoptions={}):
    if Debug>2:
        print "runtest", test, inoptions
    if not test:
        raise TestError('no test specified')

    root, base = os.path.split(test)
    if base == GLOBAL_CFG: return

    directory = inoptions.get('dir', root)
    if directory:
        if Debug:
            print 'changing cwd to', directory
        try:
            os.chdir(directory)
        except OSError:
            raise TestError('chdir to %s failed' % (directory,))

    if base.split('.')[-1] in ('9','out','cfg'):
        base = base[:base.rfind('.')]

    src = base + '.9'
    cfg = base + '.cfg'

    try:
        globalcfg = open(GLOBAL_CFG, 'r')
        options = readconfig(globalcfg)
        for key in inoptions:
            options[key] = inoptions[key]
        if Debug:
            print 'read global config file'
    except IOError:
        options = inoptions
        if Debug:
            print 'no global config file'

    try:
        src_ = open(src,'r')
        src_.close()
    except IOError:
        raise TestError('Source file "%s" not found' % (src,))
    try:
        cfg_ = open(cfg,'r')
        config = readconfig(cfg_)
        cfg_.close()
    except IOError:
        config = {}

    if Debug > 1:
        print 'config', config
        print 'options', options

    for key in options:
        config[key] = options[key]

    out = config.get('out', True)
    if not out:
        out = False
    elif out == True:
        out_ = base + '.out'
    else:
        out_ = config['out']
        out = True

    try:
        if not out:
            raise IOError
        out = open(out_,'r')
        out.close()
        out = True
        output_tuple = mkstemp()
        os.close(output_tuple[0])
        output = output_tuple[1]
    except IOError:
        out = False
        output = "/dev/null"

    run = config.get('run', False)

    if Debug:
        print 'Debug: variables'
        print 'src', src
        print 'out', out
        print 'out_', out_
        print 'run', run

    status = []                 # set status to 'success'
    # first compile
    cmd = COMPILER +" "+ config.get('flags','') +' '+ src
    if run:
        cmd += ' > /dev/null'
    else:
        cmd += ' >' + output

    cmd += ' 2>&1'
    if Verbose > 1:
        print 'compiling as:', cmd
    rc = os.system(cmd)

    error = config.get('error',False)
    if (error and rc == 0) or (not error and rc != 0):
        status.append('compiler test failure: not equal to ' + str(error))
        if Verbose > 1:
            print status[-1]
        if Debug > 3 and not run:
            print "begin compiler output:"
            print open(output).read()
            print "end compiler output:"
    if Verbose > 2:
        print 'compiler test result rc =', rc

    if out and not run:
        # check compiler output
        if Debug:
            print 'diffing compiler output', out_, output
        if os.system('diff -wq %s %s > /dev/null' % (out_, output,)):
            status.append('compiler output different')
            if Verbose > 1 or Debug:
                print status[-1]
    
    # if generated an executable test that
    # @@@ executable is a .tm file !!
    if run:
        raise ValueError("not implemented")
    if out:
        os.unlink(output)

    return status

def main():
    global Verbose, Debug, Quiet
    try:
        opts, args = getopt.getopt(sys.argv[1:], optstring, longopts)
    except getopt.GetoptError:
        # print help information and exit:
        usage()
        exit(2)

    Quiet = False
    recurse = False
    config = GLOBAL_CFG
    for o, a in opts:
        if o in ("-c", "--config"):
            config = a
        elif o in ("--no-config"):
            config = {}
        elif o in ("-r", "--recursive"):
            recurse = True
        elif o in ("-q", "--quiet"):
            Quiet = True
        elif o in ('-d', '--debug'):
            Debug += 1
            pass
        elif o in ('-v', '--verbose'):
            Verbose += 1
            pass
        elif o in ("-h", "--help"):
            usage()
            exit(1)
        else:
            print "invalid option", o
            exit(1)

    if config:
        try:
            config = readconfig(open(config, 'r'))
            Verbose += config.get('verbose', 0)
            Debug += config.get('debug', 0)
            Quiet = config.get('quiet', 0)
        except IOError:
            if config != GLOBAL_CFG:
                print 'could not open config file:', config
            config = {}

    processArgs(args, config)

def processArgs(args, config):
    cwd = os.getcwd()
    for arg in args:
        if os.path.isdir(arg):
            if Debug:
                print 'processing directory', arg
            for f in os.listdir(arg):
                full = os.path.join(arg,f)
                if os.path.isdir(full):
                    processArgs([full], config)
                elif f.endswith('.9'):
                    dotest(full, config)
                    os.chdir(cwd)
                else:
                    if Debug:
                        print 'skipping file:', f
        else:
            dotest(arg, config)
            os.chdir(cwd)

def dotest(f, config):
    try:
        rc = runtest(f, config)
        if not Quiet:
            if rc:
                print 'FAILURE:', f
                if Verbose:
                    for s in rc:
                        print '\t' + s
            elif Verbose:
                print 'success:', f
                
    except TestError, e:
        print 'TESTING ERROR:', e
        exit(1)
            
if __name__ == "__main__":
    main()
