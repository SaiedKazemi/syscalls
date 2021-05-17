#!/bin/bash

set -eu

: ${TMP_DIR=/tmp}

R_UNCONF="$TMP_DIR/r_unconf"
R_CONF="$TMP_DIR/r_conf"
R_COMP="$TMP_DIR/r_comp"
R_SCALLS="$TMP_DIR/r_scalls"

IMAGE="saied/syscalls:syscalls"
UNCONFINED="--security-opt seccomp=unconfined"
CONFINED=""
OPTS="--rm"

echo "running syscalls in unconfined seccomp mode"
docker run $OPTS $UNCONFINED $IMAGE | awk '{ print $1 }' > $R_UNCONF

echo "running syscalls in confined seccomp mode"
docker run $OPTS $CONFINED $IMAGE | awk '{ print $1 }' > $R_CONF

echo "comparing results (times are in milliseconds)"
paste $R_UNCONF $R_CONF | \
	awk '{ printf("%8d %8d %6.2f%%\n", $1, $2, 100.0 * ($2 - $1) / $1) }' > $R_COMP

docker run $OPTS $CONFINED $IMAGE /syscalls/syscalls -l > $R_SCALLS
echo "  unconf     conf   diff"
paste $R_COMP $R_SCALLS

rm -f $R_UNCONF $R_CONF $R_COMP $R_SCALLS
