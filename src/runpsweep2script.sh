#!/bin/bash
#Examples for running psweep2

NCHAINS=100
MAXGEN=100000
STATEFILE=psweepdream.state
MINMAXFILE=testabc_minmax.bounds
OBSFILE=testabc_observ.data
EPSILFILE=testabc.epsilons
DIR=testabcdir
SEARCHTYPE=DREAM-ABCz
WORKSCRIPT=../configs/test_abc_twopeak.cfg
MACHINEFILE=./machinefile/coi0coi1.machines

stdo=stdout_psweep
stde=stderr_psweep

#nohup mpirun -n $NCHAINS ./test_psweep2_lib.exe -DIR $DIR -WORKSCRIPT $WORKSCRIPT -SEARCHTYPE $SEARCHTYPE -VARIABLES $MINMAXFILE -OBSERVATIONS $OBSFILE -EPSILONS $EPSILFILE -STATEFILE $STATEFILE -MAXGENS $MAXGEN -NCHAINS $NCHAINS 1>$stdo 2>$stde

#LD_LIBRARY_PATH=. nohup mpirun -n 80 --mca btl_base_verbose 20 --mca oob_base_verbose 10 --hostfile $MACHINEFILE ./test_psweep2_lib.exe -DIR $DIR -WORKSCRIPT $WORKSCRIPT -SEARCHTYPE $SEARCHTYPE -VARIABLES $MINMAXFILE -OBSERVATIONS $OBSFILE -EPSILONS $EPSILFILE -STATEFILE $STATEFILE -MAXGENS $MAXGEN -NCHAINS $NCHAINS -TAG="HUHWHAT" 1>$stdo 2>$stde &

#REV: use quotes round mca things
mpirun --mca btl_tcp_if_include eno2 --mca oob_tcp_if_include eno2 -x LD_LIBRARY_PATH=. -n 80 --hostfile $MACHINEFILE   test_psweep2_lib.exe -DIR $DIR -WORKSCRIPT $WORKSCRIPT -SEARCHTYPE $SEARCHTYPE -VARIABLES $MINMAXFILE -OBSERVATIONS $OBSFILE -EPSILONS $EPSILFILE -STATEFILE $STATEFILE -MAXGENS $MAXGEN -NCHAINS $NCHAINS -TAG="HUHWHAT" 
    
