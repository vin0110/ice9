#!/usr/bin/python
"""
codegen for ice9

VW Freeh copyright 2010

CSC 512
"""

#########
# Library imports
#########
import sys

#########
# Local imports
#########

#########
# Globals
#########
OutFile = sys.stdout
Comments = True
PC = 0
DataOffset = 1                  # leave room for code size

Instructions = {
    "HALT": "RR",
    "IN": "RR",
    "INB": "RR",
    "OUT": "RR",
    "OUTC": "RR",
    "OUTNL": "RR",
    "ADD": "RR",
    "SUB": "RR",
    "MUL": "RR",
    "DIV": "RR",
    "LD": "RM",
    "ST": "RM",
    "LDA": "RA",
    "LDC": "RA",
    "JLT": "RA",
    "JLE": "RA",
    "JGT": "RA",
    "JGE": "RA",
    "JEQ": "RA",
    "JNE": "RA"
    }

def emitFile(out, no_comments=False):
    global OutFile, Comments
    OutFile = out
    if no_comments:
        Comments = False

def emitSkip(cnt):
    global PC
    r = PC
    PC += cnt
    return r

def emitBackPatch(pc, inst, r, s, t, comment):
    global PC
    try:
        kind = Instructions[inst]
    except KeyError:
        raise CodeGenError(None, 'invalid instruction "%s"', inst)

    if pc < 0:
        pc = PC
        PC += 1
    OutFile.write("%4d:%7s " % (pc, inst))
    if kind == "RR":
        OutFile.write("%d,%d,%d" % (r,s,t))
    else:
        OutFile.write("%d,%d(%d)" % (r,s,t))
    if Comments and comment:
        OutFile.write("\t" + comment)
    OutFile.write("\n")
    return pc

def emit(inst, r, s, t, comment):
    return emitBackPatch(-1, inst, r, s, t, comment)

def emitComment(comment):
    if Comments and comment:
        OutFile.write("* %s\n" % (comment,))

def emitData(val, comment):
    global DataOffset
    OutFile.write(".DATA %4d" % (val,))
    if Comments and comment:
        OutFile.write("\t" + comment)
    OutFile.write("\n")
    r = DataOffset
    DataOffset += 1
    return r

def emitSData(val, comment):
    global DataOffset
    OutFile.write('.SDATA "%s"' % (val,))
    if Comments and comment:
        OutFile.write("\t" + comment)
    OutFile.write("\n")
    r = DataOffset
    DataOffset += len(val)
    return r

def emitDataOffset():
    return DataOffset


