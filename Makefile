all:
	gcc main.c -fopenmp -o run
clean:
	rm run
