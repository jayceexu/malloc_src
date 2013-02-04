all: test_main

test_main: main.o memlib.o malloc.o
	gcc -g main.o memlib.o malloc.o -o test_main

main.o: main.c 
	gcc -c main.c -I ./include/

malloc.o: src/malloc.c 
	gcc -c src/malloc.c -I ./include

memlib.o: src/memlib.c 
	gcc -c src/memlib.c -I ./include

clean:
	rm -rf *.o test_main
