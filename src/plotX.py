#Python script to plot trajectories of the chains, i.e. X_history.

import sys
import numpy
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from matplotlib import lines



dir="./testout/";
nchains=100;
Xfilebase=dir+"Xhist_";
piXfilebase=dir+"piXhist_";
GRfilebase=dir+"GRhist";

#fig = plt.figure(  );

for c in range(0, nchains):
    print( "Processing chain# " + str(c) );
    Xfile = Xfilebase+str(c);
    piXfile = piXfilebase+str(c);
    cX = numpy.recfromtxt( Xfile, delimiter='', names=True);
    cpiX = numpy.recfromtxt( piXfile, delimiter='', names=True);
    #print( "VAR_0 val for this chain: " );
    #print( cX['VAR_0'] );
    x = cX['VAR_0'];
    y = cX['VAR_1'];
    plt.plot( x, y, linestyle='-');

plt.show();
