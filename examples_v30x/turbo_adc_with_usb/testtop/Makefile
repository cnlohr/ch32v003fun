all : testtop

# to make CI happy.
testtop.bulk.elf : testtop.c
	gcc -o $@ $^ -I../../../minichlink -ludev -lusb-1.0 -lX11 -lpthread

testtop : testtop.c
	gcc -o $@ $^ -I../../../minichlink -ludev -lusb-1.0 -lX11 -lpthread


clean :
	rm -rf *.o *~ testtop

