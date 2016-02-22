#Python script to plot trajectories of the chains, i.e. X_history.

import sys
import numpy
import matplotlib.pyplot as plt
import matplotlib.gridspec as gridspec
from matplotlib import lines



dir="./testout/";
nchains=5;
Xfilebase=dir+"Xhist_";
piXfilebase=dir+"piXhist_";
GRfilebase=dir+"GRhist";

plt.close('all');
fig = plt.figure();
for c in range(0, nchains):
    print( "Processing chain# " + str(c) );
    Xfile = Xfilebase+str(c);
    piXfile = piXfilebase+str(c);
    cX = numpy.recfromtxt( Xfile, delimiter='', names=True);
    cpiX = numpy.recfromtxt( piXfile, delimiter='', names=True);
    x = cX['VAR_0'];
    y = cX['VAR_1'];
    plt.plot( x, y, linestyle='-');
fig.savefig( dir+'Xtraj.pdf', format='pdf' );

plt.close('all');

fig = plt.figure();
for c in range(0, nchains):
    print( "Processing chain# " + str(c) );
    Xfile = Xfilebase+str(c);
    piXfile = piXfilebase+str(c);
    cX = numpy.recfromtxt( Xfile, delimiter='', names=True);
    cpiX = numpy.recfromtxt( piXfile, delimiter='', names=True);
    x = cX['VAR_0'];
    y = cX['VAR_1'];
    x = x[-len(x):];
    y = y[-len(y):];
    plt.plot( x, y, linestyle='', marker='+');

fig.savefig( dir+'Xdistr.pdf', format='pdf' );

plt.close('all');

fig = plt.figure();
for c in range(0, nchains):
    print( "Processing chain# " + str(c) );
    Xfile = Xfilebase+str(c);
    piXfile = piXfilebase+str(c);
    cX = numpy.recfromtxt( Xfile, delimiter='', names=True);
    cpiX = numpy.recfromtxt( piXfile, delimiter='', names=True);
    fit=cpiX['__NONAME'];
    print( "Fit size:", len(fit) );
    times=range(1, len(fit)*10, 10);
    print( "Times size: ", len(times) );
    plt.plot( times, fit, linestyle='-' );

fig.savefig( dir+'fits.pdf', format='pdf' );

plt.close('all');

fig = plt.figure();
GR = numpy.recfromtxt( GRfilebase, delimiter='', names=True);
times=range(1, len(GR['VAR_0'])*50, 50);
for d in GR.dtype.names:
    plt.plot( times, GR[d], linestyle='-' );

fig.savefig( dir+'GRs.pdf', format='pdf' );
plt.close('all');
