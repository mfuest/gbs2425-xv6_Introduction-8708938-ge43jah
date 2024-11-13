#!/bin/sh

# Generate usys.S, the stubs for syscalls.

set -e

echo "# generated by usys.sh - do not edit"

echo "#include \"kernel/syscalls.h\""

entry () {
  local name=$1
  echo ".global $name"
  echo "$name:"
  echo " li a7, SYS_$name"
  echo " ecall"
  echo " ret"
}

SYSCALLS=$(cat <<EOS
    fork
    exit
    wait
    pipe
    read
    write
    close
    kill
    exec
    open
    mknod
    unlink
    fstat
    link
    mkdir
    chdir
    dup
    getpid
    sbrk
    sleep
    uptime
EOS
)

for name in $SYSCALLS
do
  entry $name
done
