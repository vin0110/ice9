#
# Makefile for ice9
CC	= gcc

# for c code
#CG=cg
#CGDEF=
# for tm code
CG=cg_tm
CGDEF=-DTM

CFLAGS	= -g -Wall -pedantic -std=c99  $(CGDEF)

LIB	= -lfl
OBJ	= ice9.yy.o ice9.tab.o util.o list.o symtab.o type.o ast.o yhelp.o $(CG).o

ice9:	$(OBJ)
	$(CC) -o $@ $^ $(LIB)

$(OBJ):	ice9.h

ice9.tab.c: ice9.y
	bison -d -t -v -o $@ $<

ice9.tab.h: ice9.y
	bison -d -o ice9.tab.c $<
	rm -f ice9.tab.c

ice9.tab.o: ice9.tab.c
	$(CC) $(CFLAGS) -c $<

ice9.yy.c: ice9.l ice9.tab.h
	flex -o$@ $<

ice9.yy.o: ice9.yy.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) ice9.output

cleanest:
	make clean
	rm -f ice9.tab.c ice9.yy.c ice9.tab.h ice9

