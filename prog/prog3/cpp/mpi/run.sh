#!/bin/sh

#CSSmpdboot -n 4 -v
mpirun -np 1 ./invertedindexes rfc TCP UDP LAN PPP HDLC
#mpdallexit