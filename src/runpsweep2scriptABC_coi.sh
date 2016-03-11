#!/bin/bash
#Examples for running psweep2

NRANKS=21

NCHAINS=40
MAXGEN=100000
STATEFILE=psweepdreamABC.state
MINMAXFILE=testabc_minmax.bounds
OBSFILE=testabc_observ.data
EPSILFILE=testabc.epsilons
DIR=testabcdir
SEARCHTYPE=DREAM-ABC
WORKSCRIPT=../configs/test_abc_twopeak.cfg

HOSTFILE=/nfs-mirror/cpumachinefile

stdo=stdout_psweepABC
stde=stderr_psweepABC

#nohup mpirun -n $NCHAINS ./test_psweep2_lib.exe -DIR $DIR -WORKSCRIPT $WORKSCRIPT -SEARCHTYPE $SEARCHTYPE -VARIABLES $MINMAXFILE -OBSERVATIONS $OBSFILE -EPSILONS $EPSILFILE -STATEFILE $STATEFILE -MAXGENS $MAXGEN -NCHAINS $NCHAINS 1>$stdo 2>$stde

#nohup mpirun -n 26 ./test_psweep2_lib.exe -DIR $DIR -WORKSCRIPT $WORKSCRIPT -SEARCHTYPE $SEARCHTYPE -VARIABLES $MINMAXFILE -OBSERVATIONS $OBSFILE -EPSILONS $EPSILFILE -STATEFILE $STATEFILE -MAXGENS $MAXGEN -NCHAINS $NCHAINS -TAG=ABCYOLO 1>$stdo 2>$stde &

nohup mpirun -n $NRANKS --hostfile $HOSTFILE ./test_psweep2_lib.exe -DIR $DIR -WORKSCRIPT $WORKSCRIPT -SEARCHTYPE $SEARCHTYPE -VARIABLES $MINMAXFILE -OBSERVATIONS $OBSFILE -EPSILONS $EPSILFILE -STATEFILE $STATEFILE -MAXGENS $MAXGEN -NCHAINS $NCHAINS -TAG=ABCYOLO 1>$stdo 2>$stde &

    
