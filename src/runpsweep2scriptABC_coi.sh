#!/bin/bash
#Examples for running psweep2

NGPU=12
NPERGPU=1
NWORKERS=$(($NPERGPU * $NGPU))
NRANKS=$((1+$NWORKERS))

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

#HOSTFILE=/nfs-mirror/gpumachinefile
HOSTFILE=/home/riveale/git/code/psweep2/src/machinefile/coi0coi1.machines

stdo=stdout_psweepABCz
stde=stderr_psweepABCz

#nohup mpirun -n $NCHAINS ./test_psweep2_lib.exe -DIR $DIR -WORKSCRIPT $WORKSCRIPT -SEARCHTYPE $SEARCHTYPE -VARIABLES $MINMAXFILE -OBSERVATIONS $OBSFILE -EPSILONS $EPSILFILE -STATEFILE $STATEFILE -MAXGENS $MAXGEN -NCHAINS $NCHAINS 1>$stdo 2>$stde

#nohup mpirun -n 26 ./test_psweep2_lib.exe -DIR $DIR -WORKSCRIPT $WORKSCRIPT -SEARCHTYPE $SEARCHTYPE -VARIABLES $MINMAXFILE -OBSERVATIONS $OBSFILE -EPSILONS $EPSILFILE -STATEFILE $STATEFILE -MAXGENS $MAXGEN -NCHAINS $NCHAINS -TAG=ABCYOLO 1>$stdo 2>$stde &

nohup mpirun -n $NRANKS --hostfile $HOSTFILE gputest.exe -DIR $DIR -WORKSCRIPT $WORKSCRIPT -SEARCHTYPE $SEARCHTYPE -VARIABLES $MINMAXFILE -OBSERVATIONS $OBSFILE -EPSILONS $EPSILFILE -STATEFILE $STATEFILE -MAXGENS $MAXGEN -NCHAINS $NCHAINS -TAG=ABCYOLO 1>$stdo 2>$stde &
