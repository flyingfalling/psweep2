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


#hdf5_file_name = '__tecnet2_iBP_run11apr2017.state';
hdf5_file_name = '__tecnet2_iBP_run9jun2017_abc.state';
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

#REV: Y_param contains the "target" values.
#REV: I know "divergence" from them. Adding epsilon[blah] to my guy negative gives me the original value.

gen = paramsdict[ 't_gen' ];


#REV: print chain, gen, newval. Otherwise, they all stay at old value :0
#REV: do the same thing for X? Nah we already have Xval I guess...
#REV: make same plot for each separate variable, different colors for different chains.
#REV: I can then go and "access" the value at that point, fine.
chainaccs = [ [] for _ in range(0,nchains) ];

for g in range( gen-100000, gen ):
    start = (g) * nchains;
    end = (g+1) * nchains;
    #REV: note accepts first one is not accept, so need to do something else.
    acc = accepts[start-nchains:end-nchains];
    #x = Xvals[start:end];
    #divs = modelY[start:end];
    
    for chain, val in enumerate(acc):
        if( val ):
            chainaccs[ chain ].append( g );
            #it accepted this turn, use new values.
            #print( "Accepted chain ", chain );
            #REV: size is 4
            #size = len(epsilons);
            #tmpstart = size*chain;
            #tmpend = size*(chain+1);
            #tmpdiv = divs[tmpstart:tmpend];

            #REV: there is no epsilon etc. in this yet. tmpdiv is the raw result.
            #print( "New divs: ", tmpdiv );
            #print( "Errors: ", obs-tmpdiv );
            

#REV: now I can build the model performance matrices.
#REV: note, at some point, I need to actually save the model "run" values, to show what happened. Need a way to save it in the HDF5 file itself?
#REV: or I could re-run it? Use same random seeds? Impossible...



obsvals = [ [ [] for j in range(nchains)] for i in obs[0] ];

#REV: within each [], need Nchains, times array of Nparams, * N generations

for c in range(0, nchains):
    #print( "Chain ", c, " has ", len(chainaccs[c]) );
    for accgen in chainaccs[c]:
        gen = accgen;
        genstart = (accgen+0)*nchains;
        #genend = (accgen+1)*nchains;
        myresult = modelY[genstart + c]; #REV: this should work...
        #print(myresult);
        #REV: they better be in right order? :0
        for o, val in enumerate(myresult):
            if( val < 1e6 ):
            #print( "Appending ", (gen,val), " to ", o, c );
                obsvals[o][c].append( (gen, val) );
            #print( "Type: ", obsvals[o][c].__class__ );
            #print( "Size of obs o,c ", o, c, len(obsvals[o][c]) );


mypdf = PdfPages('obs_stats_iBP.pdf');

for o, obsval in enumerate(obsvals):
    plt.rc('text', usetex=False);
    fig = plt.figure();
    for c in range(nchains):
        gens = [];
        vals = [];
        for item in obsvals[o][c]:
            mygen = item[0];
            gens.append(mygen);
            myval = item[1];
            vals.append(myval);
        plt.plot( gens, vals, linestyle='-' );
    trueval = obs[0][o];
    plt.axhline( trueval, lw=3.0, linestyle='--' );
    plt.title( obsnames[o].decode('ascii') );
    #plt.savefig(filename+'.pdf', format='pdf');
    mypdf.savefig( fig );
    plt.close(fig);
    



mypdf.close();
