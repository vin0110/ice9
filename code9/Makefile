EXE=bsort fact fib ifact sticks sieve

all:	$(EXE)

clean:
	rm -f $(EXE) _ice9.c

cleanest:
	make clean; rm -f *.c

bsort:	bsort.9
	ice9 -n $<

fib:	fib.9
	ice9 -n $<

fact:	fact.9
	ice9 -n $<

ifact:	ifact.9
	ice9 -n $<

sticks:	sticks.9
	ice9 -n $<

sieve:	sieve.9
	ice9 -n $<
