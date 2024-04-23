all : testtop

# to make CI happy.
testtop.hidapi.elf : 
	gcc -o $@ $^ -I../../../minichlink -ludev

testtop : testtop.c
	gcc -o $@ $^ -I../../../minichlink -ludev

clean :
	rm -rf *.o *~ testtop

