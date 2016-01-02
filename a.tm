0: LD  0, 1(6)
1: LDC 1, 3(0)
2: SUB 2, 0, 1
3: JGE 2, 5(7)
4: LD  3, 1(6)
5: LDC 4, 1(0)
6: ADD 5, 3, 4
7: ST  5, 1(6)
8: LDA 7, -9(7)   * branch back to 0:
9: OUT 0,0,0
10: OUTNL 0,0,0
