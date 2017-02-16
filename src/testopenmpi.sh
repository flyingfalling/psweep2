mpirun --mca btl_base_verbose 30 --mca oob_base_verbose 30 --mca btl_tcp_if_include eno2 --mca oob_tcp_if_include eno2 -np 2 -host coiworkstation0,coiworkstation1 hostname
