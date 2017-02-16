#!/bin/bash
#Examples for running psweep2

NCHAINS=50
MAXGEN=100000
STATEFILE=psweepdream.state
MINMAXFILE=testabc_minmax.bounds
OBSFILE=testabc_observ.data
EPSILFILE=testabc.epsilons
DIR=testabcdir
SEARCHTYPE=DREAM-ABCz
WORKSCRIPT=../configs/test_abc_twopeak.cfg

stdo=stdout_psweep
stde=stderr_psweep

#nohup mpirun -n $NCHAINS ./test_psweep2_lib.exe -DIR $DIR -WORKSCRIPT $WORKSCRIPT -SEARCHTYPE $SEARCHTYPE -VARIABLES $MINMAXFILE -OBSERVATIONS $OBSFILE -EPSILONS $EPSILFILE -STATEFILE $STATEFILE -MAXGENS $MAXGEN -NCHAINS $NCHAINS 1>$stdo 2>$stde

nohup mpirun -n 26 ./test_psweep2_lib.exe -DIR $DIR -WORKSCRIPT $WORKSCRIPT -SEARCHTYPE $SEARCHTYPE -VARIABLES $MINMAXFILE -OBSERVATIONS $OBSFILE -EPSILONS $EPSILFILE -STATEFILE $STATEFILE -MAXGENS $MAXGEN -NCHAINS $NCHAINS -TAG="HUHWHAT" 1>$stdo 2>$stde &

    
