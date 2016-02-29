#!/bin/bash
#Examples for running psweep2

NCHAINS=10
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

mpirun -n 3 ./test_psweep2_lib.exe -DIR $DIR -WORKSCRIPT $WORKSCRIPT -SEARCHTYPE $SEARCHTYPE -VARIABLES $MINMAXFILE -OBSERVATIONS $OBSFILE -EPSILONS $EPSILFILE -STATEFILE $STATEFILE -MAXGENS $MAXGEN -NCHAINS $NCHAINS 1>$stdo 2>$stde

    
