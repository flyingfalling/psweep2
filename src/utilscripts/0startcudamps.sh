

#!/bin/bash
#ALl GPU must be in exclusive process ?

#REV: need to set max number of file descriptors higher because nvidia thing uses shared memory/file pipe ethings
ulimit -Sn 20000
#sudo nvidia-smi -i 2,3,4 -c EXCLUSIVE_PROCESS

#export CUDA_VISIBLE_DEVICES="0,1" #,2,3,4,5,6,7"
#GPU 1: Tesla K80 (UUID: GPU-dce812e3-d9fa-34c1-d3e9-44478ff8f5e7)
#GPU 2: Tesla K80 (UUID: GPU-763f0b04-2db4-6f01-2a2f-596c8358fbc7)
#GPU 3: Tesla K80 (UUID: GPU-609117fb-0efa-90ad-6f1e-e24fd39615a4)
#GPU 4: Tesla K80 (UUID: GPU-0cb0b90c-b0cc-d0d0-db88-ef042f331f47)
#GPU 5: Tesla K80 (UUID: GPU-b51dbb01-4b95-9738-ecf4-a3ce613ebfea)
#GPU 6: Tesla K80 (UUID: GPU-9a512923-6437-4633-bc52-804998099ba6)
#GPU 7: Tesla K80 (UUID: GPU-51c7fb16-17cc-e762-7d77-03c3e9da1e72)
#GPU 8: Tesla K80 (UUID: GPU-71baec9d-c2c0-355c-7eb2-7c095914c78a)

#GPU 0: Quadro K620 (UUID: GPU-1685f2e0-0f3a-fef1-c94c-00bf21afeb24)
#GPU 1: Tesla K80 (UUID: GPU-9e8b10fb-8005-24c7-b7aa-5795c39b4c15)
#GPU 2: Tesla K80 (UUID: GPU-3d917409-02ae-079b-3941-bacd9570b8c6)
#GPU 3: Tesla K80 (UUID: GPU-8faf997f-67a1-b729-6205-1da501a39470)
#GPU 4: Tesla K80 (UUID: GPU-99da7098-9e60-d67a-c5c8-de52e4b30c30)


export CUDA_VISIBLE_DEVICES="GPU-9e8b10fb-8005-24c7-b7aa-5795c39b4c15,GPU-3d917409-02ae-079b-3941-bacd9570b8c6,GPU-8faf997f-67a1-b729-6205-1da501a39470,GPU-99da7098-9e60-d67a-c5c8-de52e4b30c30"
export CUDA_MPS_PIPE_DIRECTORY=/tmp/nvidia-mps
export CUDA_MPS_LOG_DIRECTORY=/tmp/nvidia-log
nvidia-cuda-mps-control -d

#!/bin/bash
#NGPUS=$1 # Number of gpus with compute_capability 3.5 per server
# Start the MPS server for each GPU
#for (( i=0; i<$NGPUS; i++ ))
#for i in $(seq 0 $(($NGPUS-1)))
#do
#    mkdir /tmp/mps_$i
#    mkdir /tmp/mps_log_$i
#    export CUDA_VISIBLE_DEVICES=$i
#    export CUDA_MPS_PIPE_DIRECTORY=/tmp/mps_$i
#    export CUDA_MPS_LOG_DIRECTORY=/tmp/mps_log_$i
#    nvidia-cuda-mps-control -d
#done


#REV: Apparently I'm supposed to also set visible CPUs etc.?
#sudo nvidia-cuda-mps-control -d
