#ALl GPU must be in exclusive process ?

export CUDA_MPS_PIPE_DIRECTORY=/tmp/nvidia-mps
export CUDA_MPS_LOG_DIRECTORY=/tmp/nvidia-log
nvidia-cuda-mps-control -d
#REV: Apparently I'm supposed to also set visible CPUs etc.?
#sudo nvidia-cuda-mps-control -d
