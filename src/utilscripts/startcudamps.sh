
1;2802;0c#!/bin/bash
#ALl GPU must be in exclusive process ?

#REV: need to set max number of file descriptors higher because nvidia thing uses shared memory/file pipe ethings
ulimit -Sn 20000
#sudo nvidia-smi -i 2,3,4 -c EXCLUSIVE_PROCESS

#!/bin/bash
NGPUS=$1 # Number of gpus with compute_capability 3.5 per server
# Start the MPS server for each GPU
#for (( i=0; i<$NGPUS; i++ ))
for i in $(seq 0 $(($NGPUS-1)))
do
    mkdir /tmp/mps_$i
    mkdir /tmp/mps_log_$i
    export CUDA_VISIBLE_DEVICES=$i
    export CUDA_MPS_PIPE_DIRECTORY=/tmp/mps_$i
    export CUDA_MPS_LOG_DIRECTORY=/tmp/mps_log_$i
    nvidia-cuda-mps-control -d
done

#export CUDA_MPS_PIPE_DIRECTORY=/tmp/nvidia-mps
#export CUDA_MPS_LOG_DIRECTORY=/tmp/nvidia-log
#nvidia-cuda-mps-control -d
#REV: Apparently I'm supposed to also set visible CPUs etc.?
#sudo nvidia-cuda-mps-control -d
