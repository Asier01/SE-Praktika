main: main.c ProcessQueue.c
	gcc main.c ProcessQueue.c -o main -I -lpthread.
