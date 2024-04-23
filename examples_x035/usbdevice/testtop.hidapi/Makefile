all : testtop

testtop : testtop.c
	gcc -o $@ $^ -I../../../minichlink -ludev

clean :
	rm -rf *.o *~ testtop

