* BEGIN builtins
* END builtins
* local procs
.DATA    0	t
* BEGIN preamble
   0:     LDA 7,1(7)	jump over local procs and builtins
   1:      LD 6,0(0)	load top of physical memory into fp
   2:     LDC 0,2(0)	load top of heap
   3:      ST 0,0(4)	store top of heap in memory
   4:     LDC 6,1(0)	increment FP to accommodate global AR
* END preamble
* O_FA
* lower bound
* O_I/BLIT
   5:     LDC 0,0(0)	integer literal
   6:      ST 0,0(6)	'i' <- lb
   7:     LDC 1,1(0)	get ub
   8:     SUB 0,0,1	'i' - ub
* fa loop body
* O_FA
* lower bound
* O_BINOP
* O_SYM
  10:      LD 0,-1(6)	i
  11:      ST 0,0(5)	lhs of binop
  12:     LDA 5,1(5)	push
* O_I/BLIT
  13:     LDC 0,1(0)	integer literal
  14:     LDA 5,-1(5)	pop
  15:      LD 1,0(5)	lhs of binop
  16:     ADD 0,0,1	add
  17:      ST 0,0(6)	'i' <- lb
  18:     LDC 1,166172232(0)	get ub
  19:     SUB 0,0,1	'i' - ub
* fa loop body
* O_IF
* O_BINOP
* O_SYM
  21:      LD 0,-1(6)	i
  22:      ST 0,0(5)	lhs of binop
  23:     LDA 5,1(5)	push
* O_SYM
  24:      LD 0,0(6)	j
  25:     LDA 5,-1(5)	pop
  26:      LD 1,0(5)	lhs of binop
  27:     SUB 0,1,0	int EQ int
  29:     LDC 0,0(0)	set to false
  30:     LDA 7,1(7)	jump over true
  28:     JGT 0,2(7)	go to true
  31:     LDC 0,1(0)	set to true
* then clause
* SEQ
* O_ASSIGN
  33:     LDA 0,1(4)	lvalue: address of t
  34:      ST 0,0(5)	lvalue
  35:     LDA 5,1(5)	push
* O_SYM
  36:      LD 0,-1(6)	i
  37:     LDA 5,-1(5)	pop
  38:      LD 1,0(5)	lvalue
  39:      ST 0,0(1)	assign
* SEQ
* O_ASSIGN
  40:     LDA 0,-1(6)	lvalue: address of i
  41:      ST 0,0(5)	lvalue
  42:     LDA 5,1(5)	push
* O_SYM
  43:      LD 0,0(6)	j
  44:     LDA 5,-1(5)	pop
  45:      LD 1,0(5)	lvalue
  46:      ST 0,0(1)	assign
* O_ASSIGN
  47:     LDA 0,0(6)	lvalue: address of j
  48:      ST 0,0(5)	lvalue
  49:     LDA 5,1(5)	push
* O_SYM
  50:      LD 0,1(4)	t
  51:     LDA 5,-1(5)	pop
  52:      LD 1,0(5)	lvalue
  53:      ST 0,0(1)	assign
  32:     JEQ 0,22(7)	cond jump over then
* fa loop tail
  54:      LD 0,0(6)	get 'i'
  55:     LDA 0,1(0)	'i'++
  56:     LDA 7,-40(7)	loop back to top
  20:     JGT 0,57(4)	jump out of fa loop
* fa loop tail
  57:      LD 0,0(6)	get 'i'
  58:     LDA 0,1(0)	'i'++
  59:     LDA 7,-54(7)	loop back to top
   9:     JGT 0,60(4)	jump out of fa loop
  60:    HALT 0,0,0	end of file
