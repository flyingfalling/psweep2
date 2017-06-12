#REV: neuron uses NANOAMPERES.

#REV: Assuming that andreas used PICO, I need to multiply 20 by 1e-3. So, instead of 20, it should be 0.2? 20.0 -> 2.0 -> 0.2 -> 0.02


#REV: python which opens the HDF5 file, reads out "accuracy" for each diagnostic variable for each chain (not proposals, but values?).
#REV: e.g. for a neuron we are fitting Vrest, Rin, Vthresh. We want to see how those evolve.

#REV: Also, plot a distribution for each model over the last N generations, for each chain. Could run a PCA etc.
#REV: within chain? Or between chains?


import h5py
import numpy as np
import matplotlib as mpl
mpl.use('Agg')
import matplotlib.pyplot as plt

from matplotlib.backends.backend_pdf import PdfPages


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

obsnames = file['__Y_param'].value;
accepts = file['accept_hist'].value;
epsilons = file['epsilon_param'].value;
obs = file['Y_param'].value;
modelY = file['model_observ_diverg_hist'].value;

mins = file['dim_mins_param'].value[0];
maxes = file['dim_maxes_param'].value[0];

#REV: Y_param contains the "target" values.
#REV: I know "divergence" from them. Adding epsilon[blah] to my guy negative gives me the original value.

gen = paramsdict[ 't_gen' ];


#REV: print chain, gen, newval. Otherwise, they all stay at old value :0
#REV: do the same thing for X? Nah we already have Xval I guess...
#REV: make same plot for each separate variable, different colors for different chains.
#REV: I can then go and "access" the value at that point, fine.

#REV: now I can build the model performance matrices.
#REV: note, at some point, I need to actually save the model "run" values, to show what happened. Need a way to save it in the HDF5 file itself?
#REV: or I could re-run it? Use same random seeds? Impossible...



vals = [ [] for i in Xnames ];

startrow = (gen-50000) * nchains;
endrow = gen * nchains;

for g in range( startrow, endrow ):
    for paramidx, param in enumerate(Xnames):
        vals[ paramidx ].append( Xvals[ g ][ paramidx ] ); 

mypdf = PdfPages('histo_iBP.pdf');

for paramidx, param in enumerate(Xnames):
    plt.rc('text', usetex=False); #REV: what does this do?
    fig = plt.figure();
    pname = str( param.decode('ascii') );
    min = mins[paramidx];
    max = maxes[paramidx];
    print( "Param ", pname, " min ", min, " max ", max );
    val = vals[paramidx];
    nbins = 50;
    n, bins, patches = plt.hist(val, nbins, normed=0, facecolor='green', alpha=0.75);
    plt.xlim( [min, max] );
    plt.title( pname );
    #filename = 'histo_' + pname;
    #plt.savefig(filename + '.pdf', format='pdf');
    mypdf.savefig( fig );
    plt.close(fig);

mypdf.close();
