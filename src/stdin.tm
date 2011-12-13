* BEGIN builtins
* END builtins
* BEGIN preamble
   0:     LDA 7,1(7)	jump over local procs and builtins
   1:      LD 6,0(0)	load top of physical memory into fp
   2:     LDC 0,1(0)	load top of heap
   3:      ST 0,0(4)	store top of heap in memory
   4:     LDC 6,0(0)	increment FP to accommodate global AR
* END preamble
* O_EXP
* O_BINOP
* O_I/BLIT
   5:     LDC 0,1(0)	integer literal
   6:      ST 0,0(5)	lhs of binop
   7:     LDA 5,1(5)	push
* O_I/BLIT
   8:     LDC 0,2(0)	integer literal
   9:     LDA 5,-1(5)	pop
  10:      LD 1,0(5)	lhs of binop
  11:     ADD 0,0,1	add
  12:    HALT 0,0,0	end of file
