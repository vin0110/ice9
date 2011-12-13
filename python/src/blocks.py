#!/usr/bin/python
"""
basic blocks for ice9

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
from emit import emitBackPatch

#########
# Class definitions
#########
class BlockError(Exception):
    pass

class Block(object):
    def __init__(self, start, end, pred=[], succ=[], num=None):
        assert end >= start
        if num:
            self.num = num
        else:
            self.num = len(Blocks)
        self.start = start
        self.end = end
        self.pred = pred
        self.succ = succ
        self.code = []

    def __str__(self):
        return "Block(%d)" % (self.num,)

    def show(self):
        print "Block(%d) Pred[%s], Succ[%s]" % (self.num,  \
                 ','.join([str(b.num) for b in self.pred]),\
                 ','.join([str(b.num) for b in self.succ]))

class Instruction(object):
    def __init__(self, loc, op, comment):
        self.loc = loc
        self.op = op
        self.comment = comment
        self.block = None

    def __str__(self):
        return "%3s 0,0,0" % (self.op,)

    def emit(self, pc):
        emitBackPatch(pc, self.op,0,0,0,self.comment)

    def kind(self):
        return "Inst"

class ROinst(Instruction):
    def __init__(self, loc, op, r, s, t,  comment):
        self.loc = loc
        self.op = op
        self.r = r
        self.s = s
        self.t = t
        self.comment = comment
        self.block = None

    def __str__(self):
        return "%3s %d,%d,%d" % (self.op,self.r,self.s,self.t)

    def emit(self, pc):
        emitBackPatch(pc, self.op,self.r,self.s,self.t,self.comment)

    def kind(self):
        return "ROinst"

class RMinst(Instruction):
    def __init__(self, loc, op, r, d, s,  comment):
        self.loc = loc
        self.op = op
        self.r = r
        self.d = d
        self.s = s
        self.comment = comment
        self.block = None

    def __str__(self):
        return "%3s %d,%d(%d)" % (self.op,self.r,self.d,self.s)

    def emit(self, pc):
        emitBackPatch(pc, self.op,self.r,self.d,self.s,self.comment)

    def kind(self):
        return "RMinst"

class RMinstBr(RMinst):
    def target(self):
        try:
            return self._target
        except AttributeError:
            if self.block:
                if self.op == "LD":
                    return self.r
                else:
                    # first succ is target of unconditional branch
                    self._target = self.block.succ[0].start
                    return self._target
            else:
                return self.r

    def __str__(self):
        print self.block
        if self.block:
            if self.op in "LDC":
                return "%3s %d,%d(%d)" % (self.op,self.r,self.target(),0)
            elif self.op == "LDA":
                self.op = "LDC"
                return "%3s %d,%d(%d)" % (self.op,self.r,self.target(),0)
            elif self.op == "LD":
                return "%3s %d,%d(%d)" % (self.op,self.r,self.d,self.s)
            else:
                raise Exception("invalid branch opcode")
        else:
            return "%3s %d,%d(%d)" % (self.op,self.r,self.d,self.s)

    def emit(self, pc):
        if self.block:
            emitBackPatch(pc, self.op,self.r,self.d,self.s,self.comment)
        else:
            emitBackPatch(pc, self.op,self.r,self.d,self.s,self.comment)

    def kind(self):
        return "RMinstBr"

class RMinstCondBr(RMinst):
    def target(self):
        try:
            return self._target
        except AttributeError:
            if self.block:
                # second succ is target of unconditional branch
                self._target = self.block.succ[1].start
                return self._target
            else:
                return self.r

    def __str__(self):
        if self.block:
            mypc = self.block.start
            for i in self.block.code:
                if i == self:
                    break
                mypc += 1
            mypc += 1           # add one for this instruction being executed
            return "%3s %d,%d(%d)" % (self.op,self.r,self.target()-mypc,self.s)
        else:
            return "%3s %d,%d(%d)" % (self.op,self.r,self.d,self.s)

    def emit(self, pc):
        if self.block:
            emitBackPatch(pc, self.op,self.r,self.d,self.s,self.comment)
        else:
            emitBackPatch(pc, self.op,self.r,self.d,self.s,self.comment)

    def kind(self):
        return "RMinstCondBr"

    

#########
# Globals
#########
HaltBlock = []
Blocks = []
ConditionalBranch = ["JLT","JLE","JEQ","JNE","JGE","JGT"]
Branch = ["LDC", "LDA", "LD"]
reg_sp = 4                      # stack pointer for temps
reg_gp = 5
reg_fp = 6                      # call frame pointer
reg_pc = 7
gp_regs = [0,1,2,3]

#########
# Local routines
#########
def findBlock(B, i):
    for b in B:
        if b.start <= i and b.end >= i:
            return b
    raise BlockError("block not found at inst " + str(i))

def splitBlock(b, i, conditional):
    '''
    inst at i is a jump; split block b AFTER instruction
    '''
    assert b.start <= i and b.end >= i
    if i == b.end:
        return b
    i += 1
    if b.start == i:
        if not conditional:
            b.succ = []
        return b
    new = Block(i, b.end, [], b.succ)
    if conditional:
        if b not in new.pred:
            new.pred.append(b)
        b.succ = [new]
    else:
        # test for proc call
        if Code[i-1].comment.startswith("jump to proc"):
            new.call = True
        b.succ = []
    b.end = i-1
    Blocks.append(new)
    return new
    
def splitBlockDest(b, i, src):
    """
    inst at i is destination for a jump from block src
    """
    assert b.start <= i and b.end >= i
    if b.start == i:
        if src not in b.pred:
            b.pred.append(src)
        return b
    new = Block(i, b.end, [b, src], b.succ)
    b.succ = [new]
    b.end = i-1
    Blocks.append(new)
    return new

def calcDest(inst):
    if inst.op == "LDC":
        return inst.d
    if inst.op in ConditionalBranch:
        if inst.s == reg_pc:
            return inst.d + inst.loc + 1
    if inst.op == "LDA":
        if inst.s == reg_pc and inst.r == reg_pc:
            return inst.d + inst.loc + 1
    if inst.op == "LD":
        pass
    raise BlockError("Destination unknown")

def walkInstructions(code):
    for pc in range(0,len(code)):
        op = code[pc].op
        if op in Branch:
            if code[pc].r == reg_pc:
                #print "%3i:" % (pc,), code[pc]
                b = findBlock(Blocks, pc)
                splitBlock(b, pc, False)
                try:
                    d_pc = calcDest(Code[pc])
                    dest = findBlock(Blocks, d_pc)
                    c = splitBlockDest(dest, d_pc, b)
                    if c not in b.succ:
                        b.succ.append(c)
                except BlockError:
                    pass
        elif op in ConditionalBranch:
            #print "%3i:" % (pc,), code[pc]
            b = findBlock(Blocks, pc)
            splitBlock(b, pc, True)
            try:
                d_pc = calcDest(Code[pc])
                dest = findBlock(Blocks, d_pc)
                c = splitBlockDest(dest, d_pc, b)
                if c not in b.succ:
                    b.succ.append(c)
            except BlockError:
                pass
        elif op == "HALT":
            # print "%3i:" % (pc,), code[pc]
            b = findBlock(Blocks, pc)
            splitBlock(b, pc, False)
            
Code=[]
Data=[]

def readInstructions(instructions):
    
    for ins in instructions:
        ins = ins.strip()
        if ins[0] == "*":
            continue
        if ins[0] == ".":
            Data.append(ins)
            continue
        flds = ins.split()
        pc = int(flds[0][:-1])
        while pc+1 > len(Code):
            Code.append(Instruction(len(Code),"HALT", "filler"))

        op = flds[1]
        if op in ["IN","OUT","INB", "OUTB", "OUTC", "ADD", "SUB", "DIV", "MUL"]:
            r,s,t = flds[2].split(',')
            instr = ROinst(pc, op, int(r), int(s), int(t),' '.join(flds[3:]))
        elif op in ["HALT", "OUTNL"]:
            instr = Instruction(pc, op, ' '.join(flds[3:])) 
        elif op in ["LD","ST"]:
            r, rest = flds[2].split(',')
            d, s = rest.split('(')
            s = s[:-1]
            instr = RMinst(pc, op, int(r), int(d), int(s),' '.join(flds[3:]))
        elif op in ["LDC","LDA"]:
            r, rest = flds[2].split(',')
            d, s = rest.split('(')
            s = s[:-1]
            if r == reg_pc:
                instr = RMinstBr(pc,op,int(r),int(d), int(s),' '.join(flds[3:]))
            else:
                instr = RMinst(pc,op,int(r),int(d), int(s),' '.join(flds[3:]))
        elif op in ["JLT","JLE","JEQ","JNE","JGE","JGT"]:
            r, rest = flds[2].split(',')
            d, s = rest.split('(')
            s = s[:-1]
            instr = RMinstCondBr(pc,op,int(r),int(d), int(s),' '.join(flds[3:]))
        else:
            raise BlockError("unknown op code: " + op)

        Code[pc] = instr

    Code.append(Instruction(len(Code),"HALT", "end of program"))

def main():
    global Blocks, HaltBlock

    if len(sys.argv) > 1:
        f = open(sys.argv[1])
    else:
        f = sys.stdin

    readInstructions(f.readlines())
    '''
    for d in Data:
        print d
    for i, c in enumerate(Code):
        print i, c
    '''
    
    root = Block(0,len(Code)-1,[],[])
    Blocks.append(root)
    walkInstructions(Code)

    # post processing of blocks
    # move insts from Code list to blocks (so that we can modify code
    for b in Blocks:
        for pc in range(b.start, b.end+1):
            inst = Code[pc]
            inst.block = b
            b.code.append(inst)
        b.start = b.end = -1

    
    optimizations = ["eliminateDeadCode",
                     "eliminateNoops"]
    #optimizations = []
    for opt in optimizations:
        O = eval(opt)
        O()

    localRegisterAllocationAll()
    printBlock()

def localRegisterAllocationAll:
    for b in Blocks:
        localRegisterAllocation(b)

def localRegisterAllocation(b):
    nregs = len(gp_regs)
    

def eliminateDeadCode():
    verb = True
    if verb:
        sys.stderr.write("Optimization: eliminateDeadCode\n")
    for i, b in enumerate(Blocks):
        if b.pred == []:
            if b.num != 0:
                try:
                    if b.call == True:
                        continue
                except AttributeError:
                    pass
                if verb:
                    sys.stderr.write("del'ing block %d at %d\n" % (i, b.num))
                del Blocks[i]
    
def eliminateNoops():
    verb = True
    for b in Blocks:
        for n, i in enumerate(b.code):
            if i.op == "LDA" and i.r == i.s and i.d == 0:
                if verb: sys.stderr.write("killing inst %s\n" % (i,))
                del b.code[n]

def printBlock():
    # determine pc
    pc = 0
    block = 0
    while block > 0:
        b = Blocks[block]
        b.start = pc
        pc += len(b.code)
        if b.succ:
            next = b.succ[0].num
            if Blocks[next].start < 0:
                block = next
            else:
                block = -1
        else:
            block = -1

    for b in Blocks:
        print "* ",
        b.show()
        if b.pred == []:
            if b.num == 0:
                print "*  --- START BLOCK ---"
            else:
                try: 
                    if b.call == True:
                        print "*  --- POST CALL ---"
                    else:
                        print "*  --- DEAD CODE ---"
                except AttributeError:
                    print "*  --- DEAD CODE ---"
        if b.start < 0:
            b.start = pc
            pc += len(b.code)
        s = b.start
        for inst in b.code:
            inst.emit(s)
            s += 1


if __name__ == "__main__":
    main()
