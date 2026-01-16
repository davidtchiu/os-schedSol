all: sched_test.c cpu_hog.c interactive.c
	gcc -O0 -Wall -g cpu_hog.c -o cpu_hog
	gcc -O0 -Wall -g interactive.c -o interactive
	gcc -O0 -Wall -g sched_test.c -o sched_test

clean:
	rm -f *.o cpu_hog interactive sched_test

