#!/bin/bash
#Examples for running psweep2

NGPU=12
NPERGPU=1
NWORKERS=$(($NPERGPU * $NGPU))
#NRANKS=$((1+$NWORKERS))
NRANKS=$NWORKERS

echo "Starting with $NWORKERS workers and $NRANKS ranks"

NCHAINS=100
MAXGEN=100000
STATEFILE=psweepdreamABCz.state
MINMAXFILE=testabc_minmax.bounds
OBSFILE=testabc_observ.data
EPSILFILE=testabc.epsilons
DIR=testabcdir
SEARCHTYPE=DREAM-ABCz
WORKSCRIPT=../configs/test_abc_twopeak_gpu.cfg

HOSTFILE=./machinefile/gpu_coi0coi1.machines

stdo=stdout_psweepABCz
stde=stderr_psweepABCz



nohup mpirun --mca btl_tcp_if_include eno2 --mca oob_tcp_if_include eno2 -x LD_LIBRARY_PATH=. -n $NRANKS --hostfile $HOSTFILE gputest.exe -DIR $DIR -WORKSCRIPT $WORKSCRIPT -SEARCHTYPE $SEARCHTYPE -VARIABLES $MINMAXFILE -OBSERVATIONS $OBSFILE -EPSILONS $EPSILFILE -STATEFILE $STATEFILE -MAXGENS $MAXGEN -NCHAINS $NCHAINS -TAG=ABCYOLO 1>$stdo 2>$stde &
