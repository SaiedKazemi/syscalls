CFLAGS=-Wall -Werror -O -static -D_GNU_SOURCE

all:	syscalls do_nothing get_killed LARGE_FILE

image: all
	docker build -t saied/syscalls:syscalls .

run:
	rm -rf DIR1
	time ./syscalls

compare:	run.sh
	./run.sh

LARGE_FILE:
	dd if=/dev/zero of=LARGE_FILE bs=4k count=1024

syscalls:	syscalls.c

do_nothing:	do_nothing.c

get_killed:	get_killed.c

clean:
	rm -f syscalls do_nothing get_killed DIR1 LARGE_FILE
	rm -f a.out r1 r2 r3 r4
