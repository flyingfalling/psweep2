#REV: python which opens the HDF5 file, reads out "accuracy" for each diagnostic variable for each chain (not proposals, but values?).
#REV: e.g. for a neuron we are fitting Vrest, Rin, Vthresh. We want to see how those evolve.

#REV: Also, plot a distribution for each model over the last N generations, for each chain. Could run a PCA etc.
#REV: within chain? Or between chains?


import h5py
import numpy as np
import matplotlib.pyplot as plt


hdf5_file_name = '__tecnet2_iBP_run11apr2017.state';
file = h5py.File(hdf5_file_name, 'r')

for item in file:
    print( item );

#REV: __blah are the varnames of each matrix.

#REV: find only those in which there is an accept, and only keep those values...ugh.
#REV: in other words "compress" it, so that it is start and end generations for each set of values.

paramsdict = dict([(i, k[0]) for i,k in file['__PARAMETERS'].items()]);
print( paramsdict );

nchains = paramsdict[ 'N_chains_param' ];
Xnames = file['__X_hist'].value;
Xvals = file['X_hist'].value; #REV: chains are flat. I.e. need to div by 

accepts = file['accept_hist'].value;
epsilons = file['epsilon_param'].value;

errors = file['model_observ_diverg_hist'].value;

#REV: Y_param contains the "target" values.
#REV: I know "divergence" from them. Adding epsilon[blah] to my guy negative gives me the original value.

gen = paramsdict[ 't_gen' ];

for g in range( gen-1000, gen ):
    start = g * 80;
    end = (g+1) * 80;
    acc = accepts[start:end];
    x = Xvals[start:end];
    divs = errors[start:end];
    
    for chain, val in enumerate(acc):
        if( val ):
            #it accepted this turn, use new values.
            print( "Accepted chain ", chain );
            print( "New divs: ", divs+epsilons );
            
