#!/bin/bash

HOSTFILE="./machinefile/coi0coi1.machines"
OPTS="--mca btl_tcp_if_include eno2"
OPTS3="--mca btl_tcp_if_include 192.168.1.0/24"
OPTS2="--mca btl_base_verbose 20 --mca oob_base_verbose 10"
#mpirun -n 20 --hostfile $HOSTFILE $OPTS $OPTS2 hostname

mpirun -n 20 --hostfile $HOSTFILE --mca btl_tcp_if_include 192.168.1.0/24 --mca btl_base_verbose 20 --mca oob_base_verbose 10 hostname
