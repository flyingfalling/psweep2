#!/bin/bash
#Examples for running psweep2

NRANKS=2

WPERRANK=3

NCHAINS=50
MAXGEN=100000
STATEFILE=psweepdreamABCz.state
MINMAXFILE=testabc_minmax.bounds
OBSFILE=testabc_observ.data
EPSILFILE=testabc.epsilons
DIR=testabcdir
SEARCHTYPE=DREAM-ABCz
WORKSCRIPT=../configs/test_abc_twopeak_gpu.cfg

HOSTFILE=/nfs-mirror/gpumachinefile

stdo=stdout
stde=stderr


#nohup mpirun -n $NRANKS --hostfile $HOSTFILE gputest.exe -DIR $DIR -WORKSCRIPT $WORKSCRIPT -SEARCHTYPE $SEARCHTYPE -VARIABLES $MINMAXFILE -OBSERVATIONS $OBSFILE -EPSILONS $EPSILFILE -STATEFILE $STATEFILE -MAXGENS $MAXGEN -NCHAINS $NCHAINS -WORKERSPERRANK=$WPERRANK  -TAG=ABCYOLO 1>$stdo 2>$stde &

nohup mpirun -n $NRANKS  gputest.exe -DIR $DIR -WORKSCRIPT $WORKSCRIPT -SEARCHTYPE $SEARCHTYPE -VARIABLES $MINMAXFILE -OBSERVATIONS $OBSFILE -EPSILONS $EPSILFILE -STATEFILE $STATEFILE -MAXGENS $MAXGEN -NCHAINS $NCHAINS -WORKERSPERRANK=$WPERRANK  -TAG=ABCYOLO 1>$stdo 2>$stde &

    
