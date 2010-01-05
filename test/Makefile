#
TEST=src/test.py
DIR=p1 p2 p3 p4

all:	$(DIR)

p1:
	python $(TEST) -r lex

p2:	
	python $(TEST) -r parse

p3:	
	python $(TEST) -r semantic

p4:	
	python $(TEST) -r cg


clean:
	rm -f */*.swp */*~ */_ice9.c lex/*.c parse/*.c semantic/*.c cg/*.c