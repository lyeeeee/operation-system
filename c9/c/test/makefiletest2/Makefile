test1.o : test1.c
	@gcc -c -o test1.o test1.c
test2.o : test2.c
	@gcc -c -o test2.o test2.c
test.bin: test1.o test2.o
	@gcc -o test.bin test1.o test2.o
all : test.bin
	echo "compile done!"
