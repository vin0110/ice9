* BEGIN preamble
   0:     LDA 7,1(7)	jump over local procs and builtins
   1:      LD 6,0(0)	load top of physical memory into fp
   2:     LDC 0,1(0)	load top of heap
   3:      ST 0,0(4)	store top of heap in memory
   4:     LDC 6,0(0)	increment FP to accommodate global AR
* END preamble
* O_READ
   5:      IN 0,0,0
   6:      ST 0,0(4)	move lhs to temp
   7:     LDC 0,0(0)	integer literal
   8:      LD 1,0(4)	move lhs from temp
   9:     SUB 0,0,1	sub
* then clause
  11:     LDC 0,1(0)	integer literal
  12:     OUT 0,0,0	write statment
  13:   OUTNL 0,0,0	newline for write statment
  10:     JEQ 0,4(7)	cond jump over then
* else clause
  15:     LDC 0,100(0)	integer literal
  16:     OUT 0,0,0	write statment
  17:   OUTNL 0,0,0	newline for write statment
  14:     LDA 7,4(7)	abs jump over else
  18:    HALT 0,0,0	end of file
