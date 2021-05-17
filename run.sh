#!/bin/bash

echo "running syscalls in unconfined seccomp mode"
docker run --security-opt seccomp=unconfined saied/syscalls:syscalls /syscalls/syscalls -m 1000 | \
	awk '{ print $1 }' > r1

echo "running syscalls in confined seccomp mode"
docker run saied/syscalls:syscalls /syscalls/syscalls -m 1000 | \
	awk '{ print $1 }' > r2

echo "comparing results (times are in milisecond)"
paste r1 r2 | \
	awk '{ printf("%8d %8d %6.2f%%\n", $1, $2, 100.0 * ($2 - $1) / $1) }' > r3
./syscalls -l > r4
echo "  unconf     conf"
paste r3 r4
