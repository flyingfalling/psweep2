#pragma once

//REV: todo: add support for gap junctions, non-spiking (gradient) neurons, etc. Add support for ACh, and other neurons that mediate conductance, stdp?
//REV: add support for synapto-synapse connections, i.e. modulate transmitter release without changing Vm of presynaptic neuron (?)

// REV: 2 March 2012 UPDATE: maximum neurons: 65500
// REV:                      maximum synapse: at least 4.2 million
// REV: for realtime, of course this is much smaller, more like 10k neurons, 100k synapses.

/* Richard Veale 15 Jan 2012
 * nsim3.h
 * header file for spiking neural network for robotic control
 *
 */

#define EXPON(argum) t2exp(argum)

/* REV: Implementation/Architecture (Features Overview)
 * 1) LIF, Izhikevich, and Poisson type neural dynamics
 *    LIF:     Update by dV = (-V + R(Ibg+Isyn))/tau, closed form solution from (????)
 *    Izhi:    Update by efficient implementation from Izhikevich 2010 (forward euler method with linear interpolation for calculating more accurate spike time)
 *    Poisson: Update by either internal probability function or by feeding spike-time vector (REV: "extended Izhi neuron", with k, Vt, Vr, Vpeak, etc.?) 2007, chpt 8.
 *       Neural dynamics implementation is at timestep dt, with dt/C if a switch is turned on to integrate at the smaller time period after an inward current
 * 2) Simple inward current type PSR, and also transmitter conductance type PSR (stored per-neuron, not per-synapse)
 *    Inward PSR:  Excit & Inhib type PSR current, decay exponentially. Stored per-neuron.
 *    Trans Cond PSR: AMPA, GABAa/b, NMDA voltage-gated conductances, decay exponential, stored per-neuron. Updated with "next step Vm" as per Izhi 2010
 * 3) Efficient spike buffers for spike delivery between neurons, 1d synapse array
 * 4) 3d placement of neurons in space with options for probabilistic connections, and even probabilistic placement of neurons, based on user-passed function
 *
 */


/*
 * Function for update of neural dynamics (determines whether to fire a spike or not) (dt is passed to make sure? when does accurate mode iterate many times?) 
 * -->Within min delay chunks.
 * Better to have one large simulator or many "parts" that are updated differently? I.e. "group" connections, because then we can have e.g. smaller spike buffers
 * And we can update large numbers at a time without worrying?
 * Add "group to group" specific stuff, so that we only have to write each function once? Pointer offsets are the most accurate way of doing things? Or pointer arithmatic?
 * REV: set all things to local variables, the compiler will optimize away, right? Also, do parallel update if necessary (though we want to save cores for other update fnct)
 */

//REV: might as well combine PRE and POST into a single variable (since we limit the total number of neurons/synapses we can have anyways via si_grp_ID etc...)

#pragma once

//#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <gsl/gsl_rng.h> //gsl random number generator
//#include <gsl_rng.h> //gsl random number generator
#include <gsl/gsl_randist.h> //for gamma random
//#include <gsl_randist.h> //for gamma random

#include <time.h>
#include <sys/time.h>


#include <iostream>

#include <string.h>

//for multithreading
//#ifndef __MACH__
#include <pthread.h>
//#endif

#include <vector>

#include "../util/t2exp/t2exp.h"


#include "idxsort.h"
#include "idxsortIMPL.h"


//opencv stuff
#ifdef OPENCV
#include "opencv/highgui.h"
#include "opencv/cxcore.h"
#include "opencv/cv.h"
#else
typedef struct _IplImage {
  int                  nSize;
  int                  ID;
  int                  nChannels;
  int                  alphaChannel;
  int                  depth;
  char                 colorModel[4];
  char                 channelSeq[4];
  int                  dataOrder;
  int                  origin;
  int                  align;
  int                  width;
  int                  height;
  struct _IplROI*      roi;
  struct _IplImage*    maskROI;
  void*                imageId;
  struct _IplTileInfo* tileInfo;
  int                  imageSize;
  char*                imageData;
  int                  widthStep;
  int                  BorderMode[4];
  int                  BorderConst[4];
  char*                imageDataOrigin;
} IplImage;
#endif

#ifdef __MACH__
#include <mach/mach_time.h> //for mach_absolute_time() on MACH OSX
#include <unistd.h> //for usleep() on MACH OSX
#else
#include <sys/timex.h>
#endif


#define NS_TO_US_CONVERSION (1e-3) //1000 nanosec in microsec

//REV; this is the "difference in microseconds" thing for linux. Wait, ntp val is changed? it's not real time?
/*static signed long long difference_micro(struct ntptimeval *bBefore, struct ntptimeval *aAfter)
{
	return (signed long long) aAfter->time.tv_sec * 1000000ll +
	       (signed long long) aAfter->time.tv_usec -
	       (signed long long) bBefore->time.tv_sec * 1000000ll -
	       (signed long long) bBefore->time.tv_usec;
	       }*/
  


//REV: 
/*
  uint64_t t1, t2;
  
  Init();

  t1 = mach_absolute_time();
  //code to time here.
  t2 = mach_absolute_time();

  double duration_ns = (double)(t2 - t1) * mach_timer_conversion_factor;  
*/


  //REV: AdEx neuron parameters (global, not set for each group, although it should be pain in the but to actually allocate it now, should write new version with better
  //scheduling etc., and encapsulation?). How much look-ahead do normal procs get?
  

/*   ///REV: might need to make these global */
/* extern float CM;// = 50.0; //pF //membrane capacitance. (specific capacitancE? ) */
/* extern float ONE_DIV_CM;// = (1.0/CM); //1/pF */
/* extern float gL;// = 5.7; //nS //leak conductance */
/* extern float E_L;// = -60.0; //mV //Reversal potential of leak conductance */
/* extern float Dt;// = 1.0; //sharpness of spikes. Dt*gL*exp(Vm-Vr / Dt) */
/* extern float Vt;// = -50.0; //mV  //threshold potential */
/* extern float Vr;// = E_L; //mV   //Reset potential */
  
/* extern float wtau;// = 40.0; //ms. Time constant of W constant (causes opposite current in V) */
/* extern float ONE_DIV_wtau;// = (1.0/wtau); */
/* extern float ADEX_a;// = 1.0; */
/* extern float ADEX_b;// = 40.0; //amount to add to W after a spike. */
/* extern float ADEX_VPEAK;// = 0; //peak of spike */


#ifdef PER_SYNAPSE_CONDUCTANCES
struct per_presyn_grp_conductance_state
{
  //REV: need to do it by either region or neuron. Easier by neuron (at least for now). We can examine current vs firing behavior as well
  //by looking at spike rate versus net current input...which should linearly sum ...right? No... there should be shunting...?
  //I.e. if total conductance is higher...it messes with the current non-linearly?
  //If we do by presyn neuron, each neuron state will just be a list of the presyn neuron, either separated by their blah or not. Just keep the 
  //total state too haha...easier? If so, I can check the "type", but too much work to look to the nsim3 sim. Ugh really need to rewrite that 
  //in some way to easily extract things...
  
  //So, make it an N (we know type blah), by 3 (each one is the list of presyn neurons of that type, might be zero haha), by X, where X is number
  //of presyn neurons of that type. But those are both the INDEX of that neuron (OK), and the conductance from that neuron (?). We need to look at all
  //the presyn conds of that neuron to correctly compute relative contribution of each type due to conductance/shunting etc.?
  std::vector<std::vector<float> > state;
  //vector (each postsyn neuron), of vectors (3 presyn types), of vector (each presyn neuron)
  //vector (each postsyn neuron), of vectors (3 presyn types), of vector (each presyn neuron)
  std::vector<std::vector<std::vector<float> > > per_neuron_state;
  std::vector<std::vector<std::vector<size_t> > > per_neuron_state_idx;
  bool init=false;
  std::vector<float> E;
  std::vector<float> TAU;
  
  std::vector<size_t> synidxs;
  
  
  //Neuron indices in bins.
  //Also, bin start/end locations.
  std::vector<std::vector<size_t>> binx;
  std::vector<std::vector<size_t>> binz;
  std::vector<size_t> neuron_xbins;
  std::vector<size_t> neuron_zbins;
  std::vector<float> binx_edges;
  std::vector<float> binz_edges;
  float binx_wid;
  float biny_wid;
  //Compute for each neuron which bin it falls in, and then just do the matrix computation? Or, do a 2d for each? Order them by xbin and ybin
  //and then I can easily binary search or index the start/ends of each... Just create grid guy for now haha.
  //Note, requires things like xmax/xmin etc...? Whatever.
  
  void create_xgrid(float min, float max, float binwid);
  
  void create_zgrid(float min, float max, float binwid);
  
  //really should do NFV or smthing..
  //lol 4d vector.
  std::vector< std::vector< std::vector< std::vector<float> > > > get_xzgrid_by_pregrp_by_postgrp(size_t pregrp, size_t postgrp);
  
  
  std::vector< std::vector<float> > get_xgrid_slice_by_pregrp_by_postgrp(size_t pregrp, size_t postgrp);
  std::vector< std::vector<float> > get_grid_full_firing(size_t grp);

  //REV: not used (just use z/2th row of above)
  std::vector< float > get_xgrid_slice_firing(size_t grp);
  
  //just sets them all to zero
  void reset_state();
  
  //constructor....expects global nsim guys to be set already...
  per_presyn_grp_conductance_state()
  {
    init = false;
    //state.resize(ni_grp_alloc, std::vector<float>( n_alloc, 0.0 ) );
  }
  
  void init_per_presyn_grp_conductance_state();
  
  //for each one, just add it, and then decay with corresponding? But need to know AMPA, etc...? For each type. Nasty.
  //I can do a check for pregrp (syngrp?) type. ah that was it the decay...
  
  void update();
  
  
  //Hm, just do this for all of them? Or do by name?
  float compute_current(size_t n, size_t g);
  
  //Let's just do the mean/etc. here and spam it out. Much easier...
  
  std::vector<float> get_current_by_grp_from_grp(size_t pregrp, size_t postgrp);
};
  
//a static instance...
extern per_presyn_grp_conductance_state ppgcs; //No...this won't work...it's global. It needs to be explicitly reset...outside

#endif


//REV: need to do this so that it correctly mangles (unmangles?) header file function defines so they can be found when linked...
#ifdef  __cplusplus
extern "C" {
#endif





#define NSIM_VERSION 3.0

//DEFINES 
//#define ACCURACY_MODE
#define EXTRATIME 0.1 //neurons integrate at dt/C times faster for this long after the most recent incoming spike before going back to dt.
//#define IZHI_PEAKMODE
//#define RECURSIVE_IZHI_MODE //REV: mode to accurately calculate izhi firing for VERY HIGH INPUT CURRENTS (would be crazy otherwise). Slower? Probably.
//Equivalent in all cases EXCEPT where multiple spikes would occur within one time zone (i.e. after back-calculating tpeak)
#define RECORD_FIRING

// TOGGLE MULTITHREADING OPTIONS
//#define MULTITHREAD_SYN_UPDATE
//#define MULTITHREAD_NEURAL_UPDATE

//REV: comment this out for "safer" per-neuron update. Uncomment it for "faster" per-grp update (only works in non-accuracy mode)
//using pointer offsets, etc. Also can multithread in grp update mode.


//Turn this on (preferably in the using program) to disable NMDA
//voltage gating. Defined in user program is best.
//#define NO_NMDA_VOLTAGE_GATE

//#define DEBUG_STDP


//REV: need to turn this OFF for accuracy mode!?!?! (?)
#define NEURAL_GRP_LEVEL_UPDATE

#define SANITY_ERROR_CHECKS


#define COND_W_MULT (1e-3) //REV: Not using this currently! Up to user to note different scales for COND versus BASE weights. (pC vs nC or smthing...)

#define RANDOM_START

#define NUM_IZHI_TYPES 7
#define IZHI_RS 0
#define IZHI_CH 1
#define IZHI_IB 2
#define IZHI_FS 3
#define IZHI_TC 4
#define IZHI_RZ 5
#define IZHI_LTS 6

#define NUM_DIMENSIONS 3
#define X_DIM 0
#define Y_DIM 1
#define Z_DIM 2

#define NUM_STP_TYPES 1
#define UDF_T 0

#define NUM_STDP_TYPES 1
#define SGFDI_T 0 //song (2000), guetig(2003), froemke&dan(2002), Izhi (2003). Algo is from song, nearest neighbor proof from izhi, 
//NLTAH from guetig, some params from froemke&dan?

#define NUM_NEURAL_TYPES 3
#define POISSON_T 0 //Poisson neuron (fires with prob)
#define LIF_T 1 //LIF neuron (current based)
#define IZHI_T 2 //Izhikevich neuron (basic type)

#define NUM_PSR_TYPES 2
#define BASE_T 0 //does just inhib and excit
#define COND_T 1 //does AMPA etc. conductances

#define NUM_PARAM_SET_TYPES 2
#define MANUAL_T 0
#define AUTOMATIC_T 1

#define NUM_PARAM_LEVEL_TYPES 2
#define GROUP_T 0
#define INDIV_T 1

#define NUM_PROB_DISTRIBUTIONS 3
#define CONSTANT_T 0 //i.e. all are equal to mean (in interval)
#define UNIFORM_T 1 //distribute uniformly (in interval?)
#define GAUSSIAN_T 2 //distribute gaussian (in interval?)
#define GAMMA_T 3 //distribute gamma (in interval?)

#define NULL_T -1

#define TRUE true
#define FALSE false
#define SMALL_NUM 1e-13 //1e-10 //1e-6

#define DESC_MAX_LEN 50 //REV: max length for char description of syn/neural impl grps.

#define SYN_GRP_BITS 10
#define SYN_OFFSET_BITS (sizeof(uint)*8-SYN_GRP_BITS) //REV: should be 22
#define SYN_GRP_MASK ((1 << SYN_OFFSET_BITS) - 1)
#define SYN_OFFSET_MASK ((1 << SYN_GRP_BITS)-1) //REV; minus the 1 we started with
#define GET_SYN_OFFSET(_sg) (((uint)_sg) & SYN_GRP_MASK)
#define GET_SYN_GRP(_sg) (((uint)_sg) >> SYN_OFFSET_BITS)
//want to leave the top 10 bits in place, and only add the bottom 22 bits... so we can just OR it right?
#define SET_SYN_GRP(_sg, _tv) ((uint)((uint)/*_sg*/GET_SYN_OFFSET(_sg) | (uint)_tv << SYN_OFFSET_BITS)) //set from current value _sg plus the offset of _tv
#define SET_SYN_OFFSET(_sg, _tv) ((uint)((uint)SET_SYN_GRP(0, (uint)GET_SYN_GRP(_sg)) | (uint)_tv)) //set from current value _sg plus the offset of _tv

#define NEUR_GRP_BITS 6
#define NEUR_SYN_START_BITS 26 //(sizeof(uint)*8-NEUR_GRP_BITS) //should be 22...
#define NEUR_GRP_MASK ((1 << NEUR_SYN_START_BITS) - 1)
#define NEUR_SYN_START_MASK ((1 << NEUR_GRP_BITS)-1) //REV; minus the 1 we started with
#define GET_NEUR_SYN_START(_ng) (((uint)_ng) & NEUR_GRP_MASK)
#define GET_NEUR_GRP(_ng) (((uint)_ng) >> NEUR_SYN_START_BITS)
#define SET_NEUR_GRP(_ng, _tv) ((uint)((uint)GET_NEUR_SYN_START(_ng) | (uint)_tv << NEUR_SYN_START_BITS)) //set from current value _sg plus the offset of _tv
#define SET_NEUR_SYN_START(_ng, _tv) (SET_NEUR_GRP(0, (uint)GET_NEUR_GRP(_ng)) | _tv) //set from current value _sg plus the offset of _tv


#define GET_X_LOC(_val, _width) (_val%_width)   //REV: get's x val taking a point in an array and a width of e.g. 2d img (assume integers)
#define GET_Y_LOC(_val, _width) (_val/_width)   //REV: returns y val taking point in array and a width of e.g. 2d img (assume integers, and will take floor)
#define VAL_FROM_XY(_x, _y, _width) (_width*_y + _x) //Returns offset in array from x and y locs in 2d array (and width)

//REV: Pre/Post stuff. Since we won't be able to handle that many neurons anyways, just store pre/post in same space
#define PRE_POST_BITS 16 //(sizeof(uint)*8)/2 //REV: should be 16 on 32 bit uints
#define PRE_MASK ((1 << PRE_POST_BITS) - 1)
#define POST_MASK ((1 << PRE_POST_BITS) - 1)
#define GET_PRE(_arg)  ((_arg) >> PRE_POST_BITS)
#define GET_POST(_arg)  ((_arg) & POST_MASK)
#define SET_PRE(_arg, _val) ((GET_POST(_arg)) | ((_val) << PRE_POST_BITS))
#define SET_POST(_arg, _val) (SET_PRE(0, GET_PRE(_arg)) | (_val)) //REV: erase the post part (by just saving the pre part), and overwrite it.




#define IZHI_VPEAK 30
#define IZHI_VMIN -90 //min rev potential, for gabab

#ifndef NSIM3_H
#define NSIM3_H

//TYPE DEFINITIONS

#include <vector>


//typedef unsigned long long ullong;
typedef int BOOL;
typedef unsigned int uint;
typedef unsigned char uchar;

typedef uint neural_ID_t;
typedef uint syn_ID_t;
typedef uint offset_ID_t;

typedef uint neural_impl_grp_ID_t;
typedef uint neural_named_grp_ID_t;

typedef uint syn_impl_grp_ID_t;
typedef uint syn_named_grp_ID_t;

typedef int psr_type_t;
typedef int neural_type_t;


typedef int prob_dist_t;
typedef int option_t;

  
  //typedef double simtime_t; //type of sim time //REV: this will lead to problem when I allocate float guys...shit
  typedef float simtime_t; //type of sim time //REV: this will lead to problem when I allocate float guys...shit
typedef simtime_t tls_t; //last spike time type
typedef simtime_t delay_t; //type of synaptic delay
typedef simtime_t refract_t; //PROBLEM IS THIS!
typedef simtime_t t_refract_t; //not the problem

typedef float tau_t; //for tau time constants of various things
typedef float Vm_t; //for lif/izhi
typedef float Wm_t; //for izhi
typedef float Iinj_t;
typedef float Ibg_t, Vreset_t, Vthresh_t, Vrest_t; //vm, ibg, reset, etc type for LIF neuron
typedef float izhi_param_t; //abcd vars of izhi type
typedef float euler_d_t; //decay constant for euler integration type
typedef float base_psr_t; //psr type
typedef float cond_psr_t; //conductances type

typedef float weight_t; //type of synaptic weights
typedef float Isyn_t;

typedef float ur_t;
typedef float UDF_t;
typedef float prob_t;

typedef int stp_t;
typedef int stdp_t;

typedef float distance_t; //distance, e.g. in 3d space euclidian

//REV: measure DELAY, SIMTIME, etc in INTEGERS for speed. Update neurons using MEMORY OFFSETS.

//REV: could typdef all my structs, but maybe its worthwhile to keep them explicitly structs so I remember :)


extern gsl_rng* nsim3_rand_source; //see init_LSM_sim for rest of init


#define SIXTY2 3600
#define ONEDIVSIXTY2 0.000277777778

//REV: for OSX
// FROM: http://stackoverflow.com/questions/5167269/clock-gettime-alternative-in-mac-os-x

  //extern uint64_t bob3333;

//REV: only idea is to have some sort of struct. The contents of the struct depend on the OS. I.e. its a union. For linux it has ntptimeval. For OSX it has uint64_t.
//REV: note for mac 2nd time BETTER be larger than first or we have big problems (subtracting uints)
/*
struct linux_start_end_times
{
  struct ntptimeval before;
  struct ntptimeval after;
};
  
struct mach_start_end_times
{
  uint64_t before;
  uint64_t after;
};
*/
#ifdef __MACH__
typedef uint64_t nsim_timer_type;
#else
typedef struct ntptimeval nsim_timer_type;
#endif

struct nsim_timer
{
  nsim_timer_type before;
  nsim_timer_type after;
};

#ifdef __MACH__
extern double mach_timer_conversion_factor;
#endif


  //REV: define pthread_barrier since it apparently doesnt exist in MACH -_-;
#ifdef __MACH__
// *sigh* OSX does not have pthread_barrier (you can ignore the pthread_barrier 
// code, the interesting stuff is lower)
typedef int pthread_barrierattr_t;
typedef struct
{
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int count;
    int tripCount;
} pthread_barrier_t;


static int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned int count)
{
    if(count == 0)
    {
        errno = EINVAL;
        return -1;
    }
    if(pthread_mutex_init(&barrier->mutex, 0) < 0)
    {
        return -1;
    }
    if(pthread_cond_init(&barrier->cond, 0) < 0)
    {
        pthread_mutex_destroy(&barrier->mutex);
        return -1;
    }
    barrier->tripCount = count;
    barrier->count = 0;

    return 0;
}

static int pthread_barrier_destroy(pthread_barrier_t *barrier)
{
    pthread_cond_destroy(&barrier->cond);
    pthread_mutex_destroy(&barrier->mutex);
    return 0;
}

static int pthread_barrier_wait(pthread_barrier_t *barrier)
{
    pthread_mutex_lock(&barrier->mutex);
    ++(barrier->count);
    if(barrier->count >= barrier->tripCount)
    {
        barrier->count = 0;
        pthread_cond_broadcast(&barrier->cond);
        pthread_mutex_unlock(&barrier->mutex);
        return 1;
    }
    else
    {
        pthread_cond_wait(&barrier->cond, &(barrier->mutex));
        pthread_mutex_unlock(&barrier->mutex);
        return 0;
    }
}
#endif //end if __MACH__

//to access, we do nsim_timer.
//e.g. start(nsim_timer). And it calls the appropriate thing to fill it.
//e.g. end(nsim_timer). calls the appropriate then to fill end.
//then, elapsed_milli_time(nsim_timer), calls the appropriate thing to get the difference in milliseconds and returns it.

static void nsim_timer_start(struct nsim_timer* timer)
{
#ifdef __MACH__
  timer->before = mach_absolute_time();
#else
  ntp_gettime(&(timer->before));
#endif
}

static void nsim_timer_end(struct nsim_timer* timer)
{
#ifdef __MACH__
  timer->after = mach_absolute_time();
#else
  ntp_gettime(&(timer->after));
#endif
}

static signed long long nsim_timer_udifference(struct nsim_timer* timer)
{
#ifdef __MACH__
  if(timer->before > timer->after)
    {
      fprintf(stderr, "REV: NSIM_TIMER MACH: T1 > T2, will have unsigned int overflow! (t1=%d t2=%d)\n", timer->before, timer->after);
      exit(1);
    }
  double diff = (double)(timer->after - timer->before) * mach_timer_conversion_factor * NS_TO_US_CONVERSION; 
  return (signed long long)diff;
#else
  return (signed long long) timer->after.time.tv_sec * 1000000ll +
    (signed long long) timer->after.time.tv_usec -
    (signed long long) timer->before.time.tv_sec * 1000000ll -
    (signed long long) timer->before.time.tv_usec;
#endif
}
  
  
static void init_nsim_timers() 
{
#ifdef __MACH__
  mach_timebase_info_data_t timebase;
  mach_timebase_info(&timebase);
  mach_timer_conversion_factor = (double)timebase.numer / (double)timebase.denom;
  fprintf(stdout, "MACH TIMING: conversion factor is %f\n", mach_timer_conversion_factor);
#else //LINUX
  /* REV: do nothing */
#endif
}


//3d position of neuron
struct pos_3d
{
  float x;
  float y;
  float z;

  pos_3d(float xi, float yi, float zi)
  :
  x(xi), y(yi), z(zi)
  {
    //REV: nothing to do
  }
}; //end pos_3d

struct izhi_grp_data
{
  //params
  offset_ID_t izhi_a_S, izhi_b_S, izhi_c_S, izhi_d_S; //REV: add params for Vpeak, Vreset, etc?
  offset_ID_t izhi_a_E, izhi_b_E, izhi_c_E, izhi_d_E;
  
  //state
  offset_ID_t Vm_S, Wm_S;
  offset_ID_t Vm_E, Wm_E;
}; //end izhi_grp_data

struct lif_grp_data
{
  //params
  offset_ID_t Ibg_S, Vreset_S, Vthresh_S, Vrest_S, t_refract_S, tauVm_S;
  offset_ID_t Ibg_E, Vreset_E, Vthresh_E, Vrest_E, t_refract_E, tauVm_E;
  
  //state
  offset_ID_t Vm_S, refract_S;
  offset_ID_t Vm_E, refract_E;
}; //end lif_grp_data

struct poisson_grp_data
{
  offset_ID_t poisson_prob_S, poisson_prob_E;
}; //end poisson_grp_data

//Anonymous union of neural type group data


struct base_psr_grp_data
{
  //params
  offset_ID_t tauI_S, tauI_E; //rev : these are also for dI, dE etc.
  offset_ID_t tauE_S, tauE_E;
  //states
  offset_ID_t psr_state_S, psr_state_E;
}; //end base_psr_grp_data

struct cond_psr_grp_data
{
  //params
  //offset_ID_t psr_param_S, psr_param_E;
  offset_ID_t tauAMPA_S, tauAMPA_E;
  offset_ID_t tauNMDA_S, tauNMDA_E;
  offset_ID_t tauGABAa_S, tauGABAa_E;
  offset_ID_t tauGABAb_S, tauGABAb_E;
  
  //state
  offset_ID_t psr_state_S, psr_state_E;
}; //end cond_psr_grp_data

typedef struct adexparams_t
{
  float CM = 50.0; //pF //membrane capacitance. (specific capacitancE? )
  float ONE_DIV_CM = (1.0/CM); //1/pF
  float gL = 5.7; //nS //leak conductance
  float E_L = -60.0; //mV //Reversal potential of leak conductance
  float Dt = 1.0; //sharpness of spikes. Dt*gL*exp(Vm-Vr / Dt)
  float Vt = -50.0; //mV  //threshold potential
  float Vr = E_L; //mV   //Reset potential
  
  float wtau = 40.0; //ms. Time constant of W constant (causes opposite current in V)
  float ONE_DIV_wtau = (1.0/wtau);
  float ADEX_a = 1.0;
  float ADEX_b = 40.0; //amount to add to W after a spike.
  float ADEX_VPEAK = 0; //peak of spike
} adexparams;
  void set_AdEx_params_of_named_neural_grp(const char* grpname, float cm, float g, float el, float d, float vt, float vr, float tau, float b, float a, float vpeak);
  adexparams* get_AdEx_params_of_named_neural_grp(const char* grpname);

struct neural_impl_grp
{
  //values common to all. Check neural type, and selectively access data. Have a function that does it (easier)
  uint size;
  neural_ID_t grp_S, grp_E;
  neural_type_t type;
  psr_type_t psr_type;
  char desc[DESC_MAX_LEN];
  
  //unions of special data for each, access with *_data.poisson.var  *_data.cond.var etc.
  //psr_grp_data psr_data;        //specific data for this psr type (e.g. start end of different parameters/variables)
  union
  {
    struct base_psr_grp_data base;
    struct cond_psr_grp_data cond;
  }; //end psr_grp_data
  
  //neural_grp_data neural_data;  //specific data for this neural type (e.g. start end of different parameters/variables)
  union
  {
    struct poisson_grp_data poisson;
    struct lif_grp_data lif;
    struct izhi_grp_data izhi;
  }; //end neural_grp_data
  
  adexparams adex_params;
  
}; //end neural_impl_grp


//function info struct
struct prob_dist_params
{
  double shape;
  double std;
  double mean;
  double min, max;
  //REV: what to do when below min/max? ALWAYS DO UNIFORM BETWEEN MIN/MAX?
};

struct cond_psr_options
{
  option_t tauAMPA_level, tauNMDA_level, tauGABAa_level, tauGABAb_level;
  option_t tauAMPA_dist, tauNMDA_dist, tauGABAa_dist, tauGABAb_dist;
  struct prob_dist_params tauAMPA_params, tauNMDA_params, tauGABAa_params, tauGABAb_params;
};

struct base_psr_options
{
  option_t tauI_psr_level, tauE_psr_level;
  option_t tauI_psr_dist, tauE_psr_dist;
  struct prob_dist_params tauI_psr_params, tauE_psr_params;
};

struct lif_options
{
  option_t Vthresh_set, Vthresh_level;
  option_t Vreset_set, Vreset_level;
  option_t Vrest_set, Vrest_level;
  option_t tauVm_set, tauVm_level; // also for dVm, since they must be same.
  option_t t_refract_set, t_refract_level;
  option_t Ibg_set, Ibg_level;
  
  option_t Vthresh_dist, Vreset_dist, Vrest_dist, tauVm_dist, t_refract_dist, Ibg_dist;
  struct prob_dist_params Vthresh_params, Vreset_params, Vrest_params, tauVm_params, t_refract_params, Ibg_params;
};

struct izhi_options
{
  //option_t izhi_abcd_set, izhi_abcd_level;
  option_t izhi_a_set, izhi_a_level;
  option_t izhi_b_set, izhi_b_level;
  option_t izhi_c_set, izhi_c_level;
  option_t izhi_d_set, izhi_d_level;
  
  option_t izhi_a_dist, izhi_b_dist, izhi_c_dist, izhi_d_dist;
  //option_t izhi_abcd_dist;
  struct prob_dist_params izhi_a_params, izhi_b_params, izhi_c_params, izhi_d_params;
  //prob_dist_params izhi_abcd_params; //lol just do them all together, same as the other guys?
};

struct poisson_options
{
  //REV: add option for whether it bothers to check the function? It needs to have probability in canonical time, e.g. per second, and then correct for each dt step.
  //REV: even better, needs to produce spikes for next N time from a distribution and add them to that spike thing... (this is way to go!). Can't change partway...?
  option_t poisson_prob_set, poisson_prob_level;
  option_t poisson_prob_dist;
  struct prob_dist_params poisson_prob_params;
};

//REV: make functions to create these of the correct type and fill them with defaults etc.
//for initialization
struct neural_grp_params
{
  neural_type_t type;
  psr_type_t psr_type;
  //option_t psr_set, psr_level; //moved to per-type
  
  char desc[DESC_MAX_LEN];
  
  uint size;
  struct pos_3d* pos; //they place the positions... would rather have a way to put neurons in relation to other neurons (i.e. probabilistically)
  
  union
  {
    struct lif_options lif;
    struct izhi_options izhi;
    struct poisson_options poisson;
  };
  
  union
  {
    struct cond_psr_options cond;
    struct base_psr_options base;
  };

};


struct UDF_grp_data
{
  //offset_ID_t prespk_S, prespk_E; //now need to deal with this separately...? (bc of stdp)
  offset_ID_t stp_state_S, stp_state_E; //for state fields u and r
  offset_ID_t stp_U_S, stp_U_E, stp_D_S, stp_D_E, stp_F_S, stp_F_E; //for param fields U, D, F
};

struct SGFDI_grp_data
{
  //offset_ID_t prespk_S, prespk_E; //wait, this might point to a different spot for pre firing time and post firing time! need separate for each.
  //offset_ID_t postspk_S, postspk_E; //wait, this might point to a different spot for pre firing time and post firing time! need separate for each.
  
  offset_ID_t stdp_state_S, stdp_state_E;
  offset_ID_t stdp_a_S, stdp_a_E;
  offset_ID_t stdp_u_S, stdp_u_E;
  offset_ID_t stdp_tau_S, stdp_tau_E;
  offset_ID_t stdp_wmax_S, stdp_wmax_E;
  offset_ID_t stdp_lambda_S, stdp_lambda_E;
};

struct syn_impl_grp
{
  stp_t stp_type;
  stdp_t stdp_type;
  uint size;
  char desc[DESC_MAX_LEN];
  
  syn_ID_t grp_S, grp_E; //REV: these are pretty deprecated since we sort them...but they still index correct order of grp? we don't want to index synapse...
  offset_ID_t weight_S, weight_E;
  offset_ID_t delay_S, delay_E;
  
  //REV: super ghetto, just store the info here. 1 feb 2014
  float NMDA_GABAb_proportion = 0;
  
  //need here, because shared by both STDP and STP implementations.
  offset_ID_t prespk_S, prespk_E;
  
  //STP union (only one kind of stp now...)
  union
  {
    struct UDF_grp_data UDF;
  };
  
  //STDP union (only one kind now...)
  union
  {
    struct SGFDI_grp_data SGFDI;
  };
}; // end syn_impl_grp 



//REV: params for setting up a synaptic group, including which neural groups will be connected (as lists), 
//what distribution to use over different parameters, what type of synapses these are, etc. Should a syn_impl_group always
//have the same parameters? YES! In which case, we don't need to worry about keeping track of specifics, just have syn NAMED groups just like
//syn impl groups? Syn named groups are for convenience, but a single syn impl group could e.g. connect neural named group to neural named group.
//but it would always be with same params... NO! we would want to e.g. do input to liquid from input neurons, we want different weights depending 
//on the type of liquid neuron being injected. This will lead to a lot of complexity since we need N*M parameterizations, for N pre and M post groups.


struct UDF_options
{
  //init u/r? Oh shit, we need a way to set initialization for all state variables in all neurons/synapses!!
  
  option_t stp_U_level, stp_D_level, stp_F_level;
  option_t stp_U_dist, stp_D_dist, stp_F_dist;
  struct prob_dist_params stp_U_params, stp_D_params, stp_F_params;
};

//froemke dan options
struct SGFDI_options
{
  option_t stdp_u_level, stdp_a_level, stdp_tau_level, stdp_lambda_level, stdp_wmax_level;
  option_t stdp_u_dist, stdp_a_dist, stdp_tau_dist, stdp_lambda_dist, stdp_wmax_dist;
  struct prob_dist_params stdp_u_params, stdp_a_params, stdp_tau_params, stdp_lambda_params, stdp_wmax_params;
};

//just do them one at a time...?
//REV: ah, add the SYNAPTIC GROUP, and THEN add synapses to it. I.e. set weight_S, delay_S etc. at time of group creation, and then add the other fields
//as necessary, adding one synapse at a time? But for neurons, we know how many we're adding at time the time of creation (_size). Have user generate
//fake version, and then copy it into our data structures after the fact? Difficult...
struct syn_grp_params
{
  float NMDA_GABAb_proportion;
  
  neural_impl_grp_ID_t* pregrp;
  uint num_pregrp;
  neural_impl_grp_ID_t* postgrp;
  uint num_postgrp;
  
  stp_t stp_type;
  stdp_t stdp_type;
  
  char desc[DESC_MAX_LEN];
  
  option_t weight_level, delay_level;
  option_t weight_dist, delay_dist; //allow below 0 in some distributions? for some cases we might, for others, not.
  struct prob_dist_params weight_params, delay_params;
  
  //params for STP type distribution etc.
  union
  {
    struct UDF_options UDF;
  };
  
  //REV: NOte: might want to keep params etc. around (in their own array) for later reference...?
  //REV: what's the point of having "synaptic groups"? It's for ease of checking weights etc., right? we want as few groups as possible. No use at runtime...
  
  //params for STDP type distribution etc.
  union
  {
    struct SGFDI_options SGFDI;
  };
  
  //only question now is how to generate the actual connection matrix and parameters based on properties of connected neurons? Do we need to have user fill this out?
  //If so, we should do something similar for neuron placement too, i.e. allow probabilistically placed or existent neurons! Function pointer in here?
  //REV: set level, and then let user try to set it. E.g. if we set group-level weights, we set it (err, user sets it?) and then he passes us this thing, 
  //and he tells us the groups he wants to connect. And then we go through each neuron (pair) in those groups, and call his function on it. His function takes
  //specifics of the neurons and returns a synapse and its parameterizations etc (for all that are set to INDIV LEVEL). Let's just let it take multiple groups if 
  //he wants to, and his function could then check which group its in and branch based on that? No, but his helper function won't have access to his local names, 
  //so that won't work. All it knows is neurons, and their parameters. It could (reverse-)infer group type from the parameters, but that's expensive and roundabout.

  //set num connections manually?
};


struct UDF_data
{
  UDF_t stp_U, stp_D, stp_F;
};

struct SGFDI_data
{
  float stdp_u, stdp_a, stdp_tau, stdp_lambda, stdp_wmax;
};

struct syn_params
{
  weight_t weight;
  delay_t delay;
  stp_t stp_type;
  stdp_t stdp_type;
  
  union
  {
    struct UDF_data UDF;
  };
  
  union
  {
    struct SGFDI_data SGFDI;
  };
  
};


//REV: number of threads for multithreaded neural updating


//muxes and conditional variables for multithreading neural update
//extern pthread_mutex_t neural_cond_mux;
//extern pthread_cond_t neural_cond_var;
//extern pthread_mutex_t neural_update_mux;

#ifdef MULTITHREAD_NEURAL_UPDATE
 extern uint num_update_threads;
 extern pthread_barrier_t neural_barrier;
 extern pthread_barrier_t neural_broadcast_barrier;
 extern pthread_t* neural_update_threads;
 extern void* neural_update_function(void* args);
 extern struct neural_thread_grps* neural_thread_grp_resps;

struct neural_thread_grps
{
  uint* grps; //array of uints specifying the grp nums
  uint num_grps; //how many grps are in grps
  
  //BOOL signaller;
#ifdef MULTITHREAD_SYN_UPDATE
  syn_ID_t* synID_ptr;
  simtime_t* stp_ptr;
  uint grpsize;
#endif //end MULTITHREAD_SYN
};
#endif //end MULTITHREAD_NEURAL

#ifdef MULTITHREAD_SYN_UPDATE
 extern pthread_mutex_t* neural_var_mux;
#endif //end MULTITHREAD_SYN

//extern pthread_barrier_t syn_barrier;
//extern pthread_barrier_t syn_broadcast_barrier;

//extern pthread_mutex_t neural_release_cond_mux;
//extern pthread_cond_t neural_release_cond_var;


//extern pthread_mutex_t neural_continuation_mux;

//REV: also here give some data structure for specifying/remembering what neural groups each is responsible for.

//function that will do the neural updating (based on the groups assigned to this thread)






extern simtime_t dt;
extern simtime_t dt_SMALL;
extern simtime_t simtime;
extern uint i_simtime;
extern int small_slices_per_dt;



//REV: declare variables "extern" so it doesn't get double-included in multiple implementation files (rev: compiling as library would avoid that problem?)
//====================== NEURONS ======================//

//neural implementation groups (also keeps track of conductance types)
extern struct neural_impl_grp* ni_grps ;
extern uint ni_grp_alloc ;

//---------- --ALL NEURONS ----------------//
//(for bookkeeping or for synapse purposes)
//3d position
extern struct pos_3d* pos ; //3d pos of neuron
extern neural_impl_grp_ID_t* ni_grp_ID ; //all neurons keep track of which impl grp they're in for faster update (REV: actually, want to keep track of PSR group?)
extern uint n_alloc ; //num of neurons allocated

//time last spike of neuron

//extern uint tls_alloc ;



//---------- IZHI ---------//
//Vm of neuron
extern Vm_t* Vm ;
extern uint Vm_alloc ; //should be able to determine from number of Izhi + num LIF neurons

//Wm of izhi
extern Wm_t* Wm ;
extern uint Wm_alloc ; //should be able to determine from number of Izhi neurons...

//arrays for SOME IZHI neurons
extern izhi_param_t* izhi_a ;
extern izhi_param_t* izhi_b ;
extern izhi_param_t* izhi_c ;
extern izhi_param_t* izhi_d ;
extern uint izhi_a_alloc , izhi_b_alloc, izhi_c_alloc, izhi_d_alloc;
extern uint izhi_param_alloc;

  extern float* Iinj;

//--------- LIF ---------//
//arrays for ALL LIF neurons
extern refract_t* refract ;
extern uint refract_alloc ;

//arrays for SOME LIF neurons
extern Ibg_t* Ibg ; //background current (lif n)
extern uint Ibg_alloc;

extern euler_d_t* dVm; //decay constant for euler integration of membrane potential
extern euler_d_t* dVm_SMALL;
extern uint Vm_param_alloc ;

extern Vreset_t* Vreset ; //reset potential
extern uint Vreset_alloc ;

extern Vthresh_t* Vthresh ; //threshold potential
extern uint Vthresh_alloc ;

extern Vrest_t* Vrest ;
extern uint Vrest_alloc ;

extern t_refract_t* t_refract ; //refract time 
extern uint t_refract_alloc ;

//can deallocate after
extern tau_t* tauVm ; //time constant (tau) of membrane potential
extern uint tauVm_alloc ;


//-------- POISSON ----------//
extern prob_t* poisson_prob ; //or have a function pointer? That's probably better...and then just call the function every time step :) But it needs some permanent data, so class...?
extern uint poisson_prob_alloc ;



extern float* m;
extern float* V_ext_eff;


//======================= PSRs ========================//

//keep track of postsyn!
/*extern float* per_syn_I_psr;
extern float* per_syn_E_psr;
extern float* per_syn_gAMPA;
extern float* per_syn_gNMDA;
extern float* per_syn_gGABAa;
extern float* per_syn_gGABAb;*/

extern float* per_syn_hitweight;
extern float* per_syn_hitdelta;

//(REV: note these use s_alloc for size..a waste but whatever? Globally index synapse number... could just do single hitweight, and offset, and then it could compute)
//when it adds... FUCK IT we really should do _S and _E for these, etc.!



//---------- CONDUCTANCE ---------//
//conductance psr params (per neuron)
extern euler_d_t* dAMPA ;
extern euler_d_t* dNMDA  ;
extern euler_d_t* dGABAa ;
extern euler_d_t* dGABAb ;
extern euler_d_t* dAMPA_SMALL ;
extern euler_d_t* dNMDA_SMALL  ;
extern euler_d_t* dGABAa_SMALL ;
extern euler_d_t* dGABAb_SMALL ;
//uint cond_psr_param_alloc;

//conductance psr state (per neuron)
extern cond_psr_t* gAMPA ;
extern cond_psr_t* gNMDA ;
extern cond_psr_t* gGABAa ;
extern cond_psr_t* gGABAb ;
extern uint cond_psr_state_alloc;


//can be dealloc after init
extern tau_t* tauAMPA ;
extern tau_t* tauNMDA ;
extern tau_t* tauGABAa ;
extern tau_t* tauGABAb ;
extern uint tauAMPA_alloc, tauNMDA_alloc, tauGABAa_alloc, tauGABAb_alloc;


//------------ BASE --------------//
//base (IE) psr param (per neuron)
extern euler_d_t* dI_psr ;
extern euler_d_t* dE_psr ;

extern euler_d_t* dI_psr_SMALL ;
extern euler_d_t* dE_psr_SMALL ;
//can be deallocated after initialization (unless we change dt)
extern tau_t* tauI_psr ;
extern tau_t* tauE_psr ;
extern uint tauI_psr_alloc, tauE_psr_alloc;

//base (IE) psr state (per neuron)
extern base_psr_t* I_psr ;
extern base_psr_t* E_psr ;
extern uint base_psr_state_alloc;



// ********** FUNCTIONS TO MODIFY NEURONS (and psrs) ***********//

  struct neural_impl_grp* neurgrp(int grpnum);

void init_sim_multi();

  void get_first_last_in_neuron_presynapses(uint n, uint& s1, uint& s2);

  void update_hit_synapses_range(syn_ID_t* hitsyns, simtime_t* hitdeltas, uint startidx, uint endidx);

void free_net();
void enumerate_all_neural_synaptic_state(FILE* f);
  
int get_grpnum_of_named_neural_grp(const char* desc);
int get_grpnum_of_named_syn_grp(const char* desc);
  char* get_neur_grpname(int grpnum);

  inline int get_synid_from_prepost(int preneuron, int postneuron)
  {
    //Synapses are sorted, by pre...?
    //Need to binary search for pre, and then within that search for post? Fuck this for now..just take #0.. lol
  }
  

//add a group (neuron) based on a function passed (?), i.e. possibly probabilistic, and in relation to another neural group or reference point in 3d space
neural_impl_grp_ID_t  add_neural_impl_grp( struct neural_grp_params* _ngp );

//allocate base types for all neurons, and call to resize specifics
neural_impl_grp_ID_t  allocate_neural_impl_grp( neural_type_t _type, uint _size );

//resize various data structures to make room for the new (generic) neural impl grp
void resize_state_fields_for_impl_grp(neural_type_t _type, uint _size, struct neural_impl_grp* _grp );

//specific reallocation of fields for particular neural type (lif, izhi, poisson)
void resize_state_fields_for_lif_grp(uint _size, struct neural_impl_grp* _grp);
void resize_state_fields_for_izhi_grp(uint _size, struct neural_impl_grp* _grp);
void resize_state_fields_for_poisson_grp(uint _size, struct neural_impl_grp* _grp);

//resizes the appropriate fields for a float-type field, including S and E, currsize, etc. (based on passed args)
void resize_float_field(float* *_ptr, uint* _currsize, offset_ID_t* S, offset_ID_t* E, uint _size);
void resize_float_fields(float** *_ptr, uint* _currsize, offset_ID_t* S, offset_ID_t* E, uint _size, uint _numfields); //pass a pointer to an (already allocated) 2d array
void resize_double_field(double* *_ptr, uint* _currsize, offset_ID_t* S, offset_ID_t* E, uint _size);
void resize_double_fields(double** *_ptr, uint* _currsize, offset_ID_t* S, offset_ID_t* E, uint _size, uint _numfields); //pass a pointer to an (already allocated) 2d array

//fills a parameter array passed _targ up to size in 1d array after it with distribution _dist and params of that distribution _params (struct)
void fill_distribution_param(float* _targ, option_t _dist, struct prob_dist_params _params, offset_ID_t _size);
void fill_distribution_param_double(double* _targ, option_t _dist, struct prob_dist_params _params, offset_ID_t _size);

//resize state fields for different PSR types (conductance and base I/E type)
void resize_base_psr_state_fields_with_impl_group(uint _size, struct neural_impl_grp* _grp);
void resize_cond_state_fields_with_impl_group(uint _size, struct neural_impl_grp* _grp);


void set_pre(uint n, uint val);
uint pre(uint n);

void set_post(uint n, uint val);
uint post(uint n);

//=================== SYNAPSES ====================//

extern struct syn_impl_grp* si_grps;
extern uint si_grp_alloc;

// ---------- ALL SYNAPSES ------------//
//uint num_syn_alloc;
/*extern neural_ID_t* pre; //all synapses have pre and post neural ids // REV: can groups have all the same targets? If large number of neurons connect to a single postsyn, or visa v.
  extern neural_ID_t* post;*/

extern neural_ID_t* prepost;
extern syn_impl_grp_ID_t* si_grp_ID; //holds ID's of each grp so we don't have to search through it each time (tabulating this is faster than calc each time, for large grpnums right?)

//extern neural_ID_t* postpre;
  extern neural_ID_t* postsorted_pre;
  extern neural_ID_t* postsorted_post;
  extern syn_impl_grp_ID_t* postsorted_si_grp_ID; //copy this over from original before first sort!
  extern uint* synapse_locations;

  extern uint* postsorted_startsyns;
  extern uint* postsorted_endsyns;




//REV: also have one to hold start of that neuron's section in the syn array? Much faster than binary searching each time we want to process a presyn spike! (?)
//REV: first 10 bits hold grp#, second 22 bits hold synapses offset in grp (easier to hold *initial* offset, don't bother sorting those parts too :D)

extern uint s_alloc; //num syns alloc (size of pre, post, weight, delay, and syn_grp arrays as well, since all synapses have those fields at minimum)

extern weight_t* weight; //FALSE, some synapse groups may have all the same weights and delays! But, then we run into difficulty updating...? But we can just do "register"
extern uint weight_alloc;

//REV; note, to update it we'll have to check group number of this synapse, check what type it is, and then do all that stuff anyways, so we might as well just do this too
//to save space if we have large number of delays or weights of same without distribution over them.

extern delay_t* delay;
extern uint delay_alloc;
//uint delay_alloc; //uint weight_alloc; //uint weight_delay_alloc; // == s_alloc, since all have it

// ------------ BOTH STP and STDP SYNAPSES (ACUTUALLY NEURAL STATE VARIABLES NOW) -------------- //

extern tls_t* t_l_spike;
//extern uint t_l_spike_alloc;
//just allocate for every neuron.


extern tls_t* t_l_prespk;
extern tls_t* t_l_postspk;
extern uint t_l_prespk_alloc;

//extern tls_t* t_l_pre_spike; //TL PRE spike (actually)
//extern tls_t* t_l_post_spike;
//extern uint t_l_pre_spike_alloc, t_l_post_spike_alloc;

//only allocate pre/post spike time if it is pre to STP, or pre or post to STDP synapse. Then, synapses access by looking to pre/post and then cueing 
//that neuron's offset + start in pre/post list?

//REV: if I'm going to refactor the code to be synchronous, I may as well do it now...? Just store TLspike at the beginning of every turn, and then
//update, and then if it fired, set TLspike to now. In that case, we need only one "previous spike time"? And it should be time of spike HIT, not
//time of presyn spike (i.e. should be spiketime+synapticdelay). Otherwise, for every postsyn spike, we need to go through and figure out all afferents,
//which would be O(N)

// ----------- ALL STP SYNAPSES --------//
extern ur_t* stp_u;
extern ur_t* stp_r;

extern uint stp_state_alloc;

extern UDF_t* stp_U;
extern UDF_t* stp_D;
extern UDF_t* stp_F;
extern uint stp_U_alloc, stp_D_alloc, stp_F_alloc; //let people do distro on group or indiv level.



// ------------ ALL STDP SYNAPSES --------//
//extern tls_t* t_l_post_spike; //time of last post spike //REV: todo: we can somehow make it so there is only one per neuron, and tabulate whether
//they should spike and then update at end of each turn. But that's too difficult for now...

extern float* stdp_u; //mu of stdp
extern float* stdp_wmax; //set to 1, and then normalize...? otherwise there will be a lot of computation...? I.e. have 'weight' and then the one that we 
//actually modify, which is between 0 and 1. And that*maxweight is what we multiply u*r*w by. Hm, should weight modification be based on STP? it would
//be kind of like F&D, but there would be a time of facilitation too...?

//extern float wmin; //assume 0?
extern tau_t* stdp_tau; //time constant for STDP
extern float* stdp_a; //alpha, the proportion of LTD to LTP synapses. If >1, LTD is greater strength per instance, all else equal. If <1 (>0), 
//LTP is stronger.

extern float* stdp_lambda; //learn rate of weights.

extern uint stdp_a_alloc, stdp_u_alloc, stdp_tau_alloc, stdp_wmax_alloc, stdp_lambda_alloc;//, stdp_state_alloc;


extern BOOL* si_grp_stdp;
extern BOOL* si_grp_stp;
// ------------ SYNAPSE UPDATE FUNCTIONS ----------//


  inline neural_impl_grp* get_impl_grp_from_name( const std::string& name)
  {
    return &ni_grps[ get_grpnum_of_named_neural_grp( name.c_str() ) ];
  }
  
  void set_t_l_postspk(syn_ID_t _s, tls_t _i);
  void set_t_l_postspk2(syn_ID_t _s, tls_t _i, struct syn_impl_grp* grp);
  tls_t get_t_l_postspk(syn_ID_t _s);
  tls_t get_t_l_postspk2(syn_ID_t _s, struct syn_impl_grp* grp);
  void update_stdp_post(syn_ID_t _s, simtime_t _delta);
  void update_stdp_pre(syn_ID_t _s, simtime_t _delta);
  
  
void turn_off_stdp_for_whole_net();
void turn_off_stdp_for_target_syn_grp(syn_impl_grp_ID_t gid);
void turn_on_stdp_for_whole_net();
void turn_on_stdp_for_target_syn_grp(syn_impl_grp_ID_t gid);


void ensure_sanity_of_synapse_locations();
void sort_synapses();
void sort_synapses_post();
void swap_syn_idx_with_next(syn_ID_t _idx);

//initialize state variables of synapses
void reset_synapse_state(); //REV: should group have init conditions for state variables u, r, etc.? Problem is, then we might also need DISTRIBUTION etc. for init


neural_impl_grp_ID_t  get_neur_grp(neural_ID_t _n);
offset_ID_t get_neur_grp_offset(neural_ID_t _n);
syn_ID_t get_syn_start(neural_ID_t _n);
syn_impl_grp_ID_t get_syn_grp(syn_ID_t _s);
offset_ID_t get_syn_grp_offset(syn_ID_t _s);

// ------------------------- GET ------------------------------//
//ALL SYNAPSES must have! :)
tls_t get_t_l_spike(neural_ID_t _n);

tls_t get_t_l_prespk(syn_ID_t _s);


Vm_t get_Vm(neural_ID_t _n);
Wm_t get_Wm(neural_ID_t _n);
refract_t get_refract(neural_ID_t _n);
izhi_param_t get_izhi_a(neural_ID_t _n);
izhi_param_t get_izhi_b(neural_ID_t _n);
izhi_param_t get_izhi_c(neural_ID_t _n);
izhi_param_t get_izhi_d(neural_ID_t _n);
Ibg_t get_Ibg(neural_ID_t _n);
euler_d_t get_dVm(neural_ID_t _n);
Vreset_t get_Vreset(neural_ID_t _n);
Vthresh_t get_Vthresh(neural_ID_t _n);
Vrest_t get_Vrest(neural_ID_t _n);
t_refract_t get_t_refract(neural_ID_t _n);
tau_t get_tauVm(neural_ID_t _n);


prob_t get_poisson_prob(neural_ID_t _n);


//          CONDUCTANCES            //
cond_psr_t get_gAMPA(neural_ID_t _n);
cond_psr_t get_gNMDA(neural_ID_t _n);
cond_psr_t get_gGABAa(neural_ID_t _n);
cond_psr_t get_gGABAb(neural_ID_t _n);
euler_d_t get_dAMPA(neural_ID_t _n);
euler_d_t get_dNMDA(neural_ID_t _n);
euler_d_t get_dGABAa(neural_ID_t _n);
euler_d_t get_dGABAb(neural_ID_t _n);
tau_t get_tauAMPA(neural_ID_t _n);
tau_t get_tauNMDA(neural_ID_t _n);
tau_t get_tauGABAa(neural_ID_t _n);
tau_t get_tauGABAb(neural_ID_t _n);


//            BASE PSR            //

base_psr_t get_I_psr(neural_ID_t _n);
base_psr_t get_E_psr(neural_ID_t _n);
tau_t get_tauI_psr(neural_ID_t _n);
tau_t get_tauE_psr(neural_ID_t _n);
euler_d_t get_dE_psr(neural_ID_t _n);
euler_d_t get_dI_psr(neural_ID_t _n);


//           SYNAPSES            //
weight_t get_weight(syn_ID_t _s);
delay_t get_delay(syn_ID_t _s);

//          UDF STP            //
ur_t get_stp_u(syn_ID_t _s);
ur_t get_stp_r(syn_ID_t _s);
UDF_t get_stp_U(syn_ID_t _s);
UDF_t get_stp_D(syn_ID_t _s);
UDF_t get_stp_F(syn_ID_t _s);

//           SGFDI STDP               //
float get_stdp_state(syn_ID_t _s);
void set_stdp_state(syn_ID_t _s, float _i);
float get_stdp_state2(syn_ID_t _s, struct syn_impl_grp* grp);
void set_stdp_state2(syn_ID_t _s, float _i, struct syn_impl_grp* grp);


//stdp params.
float get_stdp_tau(syn_ID_t _s);
float get_stdp_lambda(syn_ID_t _s);
float get_stdp_a(syn_ID_t _s);
float get_stdp_u(syn_ID_t _s);
float get_stdp_wmax(syn_ID_t _s);

float get_stdp_tau2(syn_ID_t _s, struct syn_impl_grp* grp);
float get_stdp_lambda2(syn_ID_t _s, struct syn_impl_grp* grp);
float get_stdp_a2(syn_ID_t _s, struct syn_impl_grp* grp);
float get_stdp_u2(syn_ID_t _s, struct syn_impl_grp* grp);
float get_stdp_wmax2(syn_ID_t _s, struct syn_impl_grp* grp);

float set_stdp_tau2(syn_ID_t _s, float _i, struct syn_impl_grp* grp);
float set_stdp_lambda2(syn_ID_t _s, float _i, struct syn_impl_grp* grp);
float set_stdp_a2(syn_ID_t _s, float _i, struct syn_impl_grp* grp);
float set_stdp_u2(syn_ID_t _s, float _i, struct syn_impl_grp* grp);
float set_stdp_wmax2(syn_ID_t _s, float _i, struct syn_impl_grp* grp);


// ---------------- SET -------------------//
//ALL neurons must have! :)
//void set_t_l_spike(syn_ID_t _n, tls_t _t);
void set_t_l_spike(neural_ID_t _n, tls_t _t);
void set_t_l_prespk(syn_ID_t _s, tls_t _t);
void set_t_l_prespk2(syn_ID_t _s, tls_t _i, struct syn_impl_grp* grp);
tls_t get_t_l_prespk2(syn_ID_t _s, struct syn_impl_grp* grp);

void set_Vm(neural_ID_t _n, Vm_t _v);
void set_Wm(neural_ID_t _n, Wm_t _w);
void set_refract(neural_ID_t _n, refract_t _r);
void set_izhi_a(neural_ID_t _n, izhi_param_t _i);
void set_izhi_b(neural_ID_t _n, izhi_param_t _i);
void set_izhi_c(neural_ID_t _n, izhi_param_t _i);
void set_izhi_d(neural_ID_t _n, izhi_param_t _i);
void set_Ibg(neural_ID_t _n, Ibg_t _i);
void set_dVm(neural_ID_t _n, euler_d_t _i);
void set_Vreset(neural_ID_t _n, Vreset_t _i);
void set_Vthresh(neural_ID_t _n, Vthresh_t _i);
void set_Vrest(neural_ID_t _n, Vrest_t _i);
void set_t_refract(neural_ID_t _n, t_refract_t _i);
void set_tauVm(neural_ID_t _n, tau_t _i);

//poison
void set_poisson_prob(neural_ID_t _n, prob_t _i);

//   PSRS  //
//cond psr
void set_gAMPA(neural_ID_t _n, cond_psr_t _i);
void set_gNMDA(neural_ID_t _n, cond_psr_t _i);
void set_gGABAa(neural_ID_t _n, cond_psr_t _i);
void set_gGABAb(neural_ID_t _n, cond_psr_t _i);
void set_dAMPA(neural_ID_t _n, euler_d_t _i);
void set_dNMDA(neural_ID_t _n, euler_d_t _i);
void set_dGABAa(neural_ID_t _n, euler_d_t _i);
void set_dGABAb(neural_ID_t _n, euler_d_t _i);

void set_tauAMPA(neural_ID_t _n, tau_t _i);
void set_tauNMDA(neural_ID_t _n, tau_t _i);
void set_tauGABAa(neural_ID_t _n, tau_t _i);
void set_tauGABAb(neural_ID_t _n, tau_t _i);

//base psr
void set_I_psr(neural_ID_t _n, base_psr_t _i);
void set_E_psr(neural_ID_t _n, base_psr_t _i);
void set_tauI_psr(neural_ID_t _n, tau_t _i);
void set_tauE_psr(neural_ID_t _n, tau_t _i);
void set_dE_psr(neural_ID_t _n, euler_d_t _i);
void set_dI_psr(neural_ID_t _n, euler_d_t _i);

//synapses
void set_weight(syn_ID_t _s, weight_t _i);
void set_delay(syn_ID_t _s, delay_t _i);
void set_stp_u(syn_ID_t _s, ur_t _i);
void set_stp_r(syn_ID_t _s, ur_t _i);
void set_stp_U(syn_ID_t _s, UDF_t _i);
void set_stp_D(syn_ID_t _s, UDF_t _i);
void set_stp_F(syn_ID_t _s, UDF_t _i);


// FAST IMPL

//tls_t get_t_l_spike2(syn_ID_t _n, struct syn_impl_grp* grp);
tls_t get_t_l_spike2(neural_ID_t _n);


Vm_t get_Vm2(neural_ID_t _n, struct neural_impl_grp* grp);
Wm_t get_Wm2(neural_ID_t _n, struct neural_impl_grp* grp);
refract_t get_refract2(neural_ID_t _n, struct neural_impl_grp* grp);
izhi_param_t get_izhi_a2(neural_ID_t _n, struct neural_impl_grp* grp);
izhi_param_t get_izhi_b2(neural_ID_t _n, struct neural_impl_grp* grp);
izhi_param_t get_izhi_c2(neural_ID_t _n, struct neural_impl_grp* grp);
izhi_param_t get_izhi_d2(neural_ID_t _n, struct neural_impl_grp* grp);
Ibg_t get_Ibg2(neural_ID_t _n, struct neural_impl_grp* grp);
euler_d_t get_dVm2(neural_ID_t _n, struct neural_impl_grp* grp);
Vreset_t get_Vreset2(neural_ID_t _n, struct neural_impl_grp* grp);
Vthresh_t get_Vthresh2(neural_ID_t _n, struct neural_impl_grp* grp);
Vrest_t get_Vrest2(neural_ID_t _n, struct neural_impl_grp* grp);
t_refract_t get_t_refract2(neural_ID_t _n, struct neural_impl_grp* grp);
tau_t get_tauVm2(neural_ID_t _n, struct neural_impl_grp* grp);


prob_t get_poisson_prob2(neural_ID_t _n, struct neural_impl_grp* grp);


//          CONDUCTANCES            //
cond_psr_t get_gAMPA2(neural_ID_t _n, struct neural_impl_grp* grp);
cond_psr_t get_gNMDA2(neural_ID_t _n, struct neural_impl_grp* grp);
cond_psr_t get_gGABAa2(neural_ID_t _n, struct neural_impl_grp* grp);
cond_psr_t get_gGABAb2(neural_ID_t _n, struct neural_impl_grp* grp);
euler_d_t get_dAMPA2(neural_ID_t _n, struct neural_impl_grp* grp);
euler_d_t get_dNMDA2(neural_ID_t _n, struct neural_impl_grp* grp);
euler_d_t get_dGABAa2(neural_ID_t _n, struct neural_impl_grp* grp);
euler_d_t get_dGABAb2(neural_ID_t _n, struct neural_impl_grp* grp);
tau_t get_tauAMPA2(neural_ID_t _n, struct neural_impl_grp* grp);
tau_t get_tauNMDA2(neural_ID_t _n, struct neural_impl_grp* grp);
tau_t get_tauGABAa2(neural_ID_t _n, struct neural_impl_grp* grp);
tau_t get_tauGABAb2(neural_ID_t _n, struct neural_impl_grp* grp);


//            BASE PSR            //

base_psr_t get_I_psr2(neural_ID_t _n, struct neural_impl_grp* grp);
base_psr_t get_E_psr2(neural_ID_t _n, struct neural_impl_grp* grp);
tau_t get_tauI_psr2(neural_ID_t _n, struct neural_impl_grp* grp);
tau_t get_tauE_psr2(neural_ID_t _n, struct neural_impl_grp* grp);
euler_d_t get_dE_psr2(neural_ID_t _n, struct neural_impl_grp* grp);
euler_d_t get_dI_psr2(neural_ID_t _n, struct neural_impl_grp* grp);


//           SYNAPSES            //
weight_t get_weight2(syn_ID_t _s, struct syn_impl_grp* grp);
delay_t get_delay2(syn_ID_t _s, struct syn_impl_grp* grp);

//          UDF STP            //
ur_t get_stp_u2(syn_ID_t _s, struct syn_impl_grp* grp);
ur_t get_stp_r2(syn_ID_t _s, struct syn_impl_grp* grp);
UDF_t get_stp_U2(syn_ID_t _s, struct syn_impl_grp* grp);
UDF_t get_stp_D2(syn_ID_t _s, struct syn_impl_grp* grp);
UDF_t get_stp_F2(syn_ID_t _s, struct syn_impl_grp* grp);

//           S, GF&D, I STDP               //

extern float* stdp_state; //REV: this is the built up values of change from multiple presyn spikes
extern uint stdp_state_alloc;



// ---------------- SET -------------------//
//ALL neurons must have! :)



void set_Vm2(neural_ID_t _n, Vm_t _v, struct neural_impl_grp* grp);
void set_Wm2(neural_ID_t _n, Wm_t _w, struct neural_impl_grp* grp);
void set_refract2(neural_ID_t _n, refract_t _r, struct neural_impl_grp* grp);
void set_izhi_a2(neural_ID_t _n, izhi_param_t _i, struct neural_impl_grp* grp);
void set_izhi_b2(neural_ID_t _n, izhi_param_t _i, struct neural_impl_grp* grp);
void set_izhi_c2(neural_ID_t _n, izhi_param_t _i, struct neural_impl_grp* grp);
void set_izhi_d2(neural_ID_t _n, izhi_param_t _i, struct neural_impl_grp* grp);
void set_Ibg2(neural_ID_t _n, Ibg_t _i, struct neural_impl_grp* grp);
void set_dVm2(neural_ID_t _n, euler_d_t _i, struct neural_impl_grp* grp);
void set_Vreset2(neural_ID_t _n, Vreset_t _i, struct neural_impl_grp* grp);
void set_Vthresh2(neural_ID_t _n, Vthresh_t _i, struct neural_impl_grp* grp);
void set_Vrest2(neural_ID_t _n, Vrest_t _i, struct neural_impl_grp* grp);
void set_t_refract2(neural_ID_t _n, t_refract_t _i, struct neural_impl_grp* grp);
void set_tauVm2(neural_ID_t _n, tau_t _i, struct neural_impl_grp* grp);

//poison
void set_poisson_prob2(neural_ID_t _n, prob_t _i, struct neural_impl_grp* grp);

//   PSRS  //
//cond psr
void set_gAMPA2(neural_ID_t _n, cond_psr_t _i, struct neural_impl_grp* grp);
void set_gNMDA2(neural_ID_t _n, cond_psr_t _i, struct neural_impl_grp* grp);
void set_gGABAa2(neural_ID_t _n, cond_psr_t _i, struct neural_impl_grp* grp);
void set_gGABAb2(neural_ID_t _n, cond_psr_t _i, struct neural_impl_grp* grp);
void set_dAMPA2(neural_ID_t _n, euler_d_t _i, struct neural_impl_grp* grp);
void set_dNMDA2(neural_ID_t _n, euler_d_t _i, struct neural_impl_grp* grp);
void set_dGABAa2(neural_ID_t _n, euler_d_t _i, struct neural_impl_grp* grp);
void set_dGABAb2(neural_ID_t _n, euler_d_t _i, struct neural_impl_grp* grp);

void set_tauAMPA2(neural_ID_t _n, tau_t _i, struct neural_impl_grp* grp);
void set_tauNMDA2(neural_ID_t _n, tau_t _i, struct neural_impl_grp* grp);
void set_tauGABAa2(neural_ID_t _n, tau_t _i, struct neural_impl_grp* grp);
void set_tauGABAb2(neural_ID_t _n, tau_t _i, struct neural_impl_grp* grp);

//base psr
void set_I_psr2(neural_ID_t _n, base_psr_t _i, struct neural_impl_grp* grp);
void set_E_psr2(neural_ID_t _n, base_psr_t _i, struct neural_impl_grp* grp);
void set_tauI_psr2(neural_ID_t _n, tau_t _i, struct neural_impl_grp* grp);
void set_tauE_psr2(neural_ID_t _n, tau_t _i, struct neural_impl_grp* grp);
void set_dE_psr2(neural_ID_t _n, euler_d_t _i, struct neural_impl_grp* grp);
void set_dI_psr2(neural_ID_t _n, euler_d_t _i, struct neural_impl_grp* grp);

//synapses
void set_weight2(syn_ID_t _s, weight_t _i, struct syn_impl_grp* grp);
void set_delay2(syn_ID_t _s, delay_t _i, struct syn_impl_grp* grp);
void set_stp_u2(syn_ID_t _s, ur_t _i, struct syn_impl_grp* grp);
void set_stp_r2(syn_ID_t _s, ur_t _i, struct syn_impl_grp* grp);
void set_stp_U2(syn_ID_t _s, UDF_t _i, struct syn_impl_grp* grp);
void set_stp_D2(syn_ID_t _s, UDF_t _i, struct syn_impl_grp* grp);
void set_stp_F2(syn_ID_t _s, UDF_t _i, struct syn_impl_grp* grp);
//void set_t_l_spike2(syn_ID_t _n, tls_t _t, struct syn_impl_grp* grp);



void add_synapse(neural_ID_t _pre, neural_ID_t _post, syn_impl_grp_ID_t _grpnum);
void resize_stp_state_fields(struct syn_impl_grp* _grp, uint _add);
void resize_stdp_state_fields(struct syn_impl_grp* _grp, uint _add);


// ------------ SPIKE BUFFER -------------//
extern simtime_t MAX_DELAY, MIN_DELAY;
extern uint nsb_grps, sb_head, sb_grp_alloc;

extern simtime_t* grp_indices;

extern syn_ID_t** spk_syn_idx;
extern simtime_t** spike_t; //global timer?

extern uint* sb_grp_filled;

extern simtime_t sb_C; //constant by which dt is divided, i.e. dt/sb_C is smallDT

//REV: HERE implement TODO
void make_spike_buffer(float _syn_spike_factor, simtime_t _C);
void add_spike(syn_ID_t _sid, simtime_t _hittime);
uint advance_spikes(syn_ID_t* *_syn_idx_ptr, simtime_t* *_spike_t_ptr);

void advanceMT();


// ---------- ACTIVE NEURONS ARRAY --------// (neurons that got (positive! shit!) inward spike recently)
struct ll_node
{
  //char active;
  //simtime_t stoptime; //this is placeholder for active. As long as stoptime > simtime, keep integrating it at smaller time step (and keep it in list).
  /*long*/simtime_t stoptime; //this is placeholder for active. As long as stoptime > simtime, keep integrating it at smaller time step (and keep it in list).
  uint next;
  uint prev;
};
extern uint head;
extern uint n_active;
extern struct ll_node* active_list;

//functions to make the list
uint list_head();
void make_active_list();
void cleanup_node(uint _n);
void add_to_active_list(uint _n, /*long*/simtime_t _extratime);
BOOL list_empty();
uint next(uint _n);
BOOL end_of_list(uint _n);
uint list_tail(); //returns prev(head)
void remove_from_active_list(uint _n);




float dist_to_neuron(uint n, float srcx, float srcy, float srcz);
void set_Iinj(uint n, float targ);
void set_V_ext_eff(uint n, float I_el);
float compute_m_0(float Vm, float Vextra, float& tau);



void print_group_spiking(neural_impl_grp_ID_t grpnum);

syn_impl_grp_ID_t   connect_neural_groups(struct syn_grp_params* _sgp, int (*_connect_rule)(neural_ID_t _pre, neural_ID_t _post, struct syn_grp_params* _sgp2, struct syn_params* _sp/*callback*/));

  syn_impl_grp_ID_t   connect_neural_groups2(struct syn_grp_params* _sgp, std::vector< std::vector< size_t > >& conns );

struct pos_3d* get_pos_3d(neural_ID_t _nid);

distance_t euclid_distance(struct pos_3d* a, struct pos_3d* b);
  distance_t euclid_distance_xz(struct pos_3d* a, struct pos_3d* b);
  distance_t euclid_distance_y(struct pos_3d* a, struct pos_3d* b);
distance_t euclid_dist(float* a, float* b, uint dims);

void print_network_to_file(const char* _fn, float subsampling_prob);
void print_neuron_afferents_to_file(const char* _fn, uint n);

void assign_grp_colors_and_print(const char* _fn, char* *_color,  char* *_efferentcolor);

void assign_syn_colors_and_print(const char* _fn, char* *_syncolor);

void naive_sim_reset(int randomise);

void quicksort(uint l, uint r);
uint qs_partition(uint l, uint r, uint p);
void sort_synapses2();

void enumerate_neural_state();
void enumerate_neural_state2();
void enumerate_synaptic_state();
void enumerate_neural_vars(neural_ID_t n);
void enumerate_synaptic_grp_state(syn_impl_grp_ID_t grpnum, FILE* f);
void enumerate_neural_grp_state(neural_impl_grp_ID_t grpnum, FILE* f);



void reset_ngp(struct neural_grp_params* _ngp, neural_type_t _nt, psr_type_t psrt);
void reset_sgp(struct syn_grp_params* _sgp, stp_t _stp, stdp_t _stdp);

extern uint fb_grps, fb_head_grp, fb_grp_alloc;
extern simtime_t fb_max_time;
extern simtime_t* fb_grp_idx;
extern uint* fb_grp_filled;

extern neural_ID_t** fb_neur_idx; //holds neural number
extern simtime_t** fb_spike_t;



struct fb_node //firebuffer node
{
  uint next;
  uint prev;
  simtime_t spk_t;  
  neural_ID_t nid;
};

extern uint fb_head;
extern uint fb_filled;
extern uint fb_alloc;
extern uint fb_tail;
extern struct fb_node* fb_extra;

void make_fb_extra(uint _size);
BOOL add_fb_node(neural_ID_t _nid, simtime_t _t);
BOOL rm_fb_node(uint _n); //rm nth fb node
BOOL is_fb_end(uint _n);
uint next_open(uint _n);


void make_fire_buffer(float _factor, uint _dtslices, uint _extra_size);
void add_fb_spike(neural_ID_t _n, simtime_t _t); //REV: this takes GLOBAL SIMTIME OF SPIKE (not time to spike)
uint advance_fb_spikes(); //either return an array of those that will fire, or just call the thing here.
void process_fb_extra_into_fb();


void process_presyn_spike(neural_ID_t _n, simtime_t _timeoffset);


void set_izhi_type_params(struct neural_grp_params* _ngp, option_t _izhi_type);

simtime_t get_dt_in_seconds();



#ifdef RECORD_FIRING
extern BOOL* firing_array;
extern simtime_t* firing_time_offsets;
#endif




//Implementation of poisson neurons is the primary problem. Just feed it spike times..

//REV: remember, implement GLOBAL TIMER, for simplicity of keeping track of when we want to update (LIF) neurons who are not getting superthreshold input...

//REV: basically, reuse old code, with added updates for cleanliness. How to deal with Poisson neurons? Have "type"? No, just have ALL neurons have the ability
//to "buffer" (force) artificially generate spikes at specified times. To do this, add a GLOBAL SPIKE BUFFER TIMER that simply specifies neuron ID, and time.
//REV: have it be exactly like the spike buffer (it is a spike buffer), except it gets the target neuron and generates all efferent spikes (adds them to 
//approriate spike buffers). Is it generated on the fly, based on estimated number of artificial spikes we will schedule? Just base it on number of poisson neurons!
//Then, for each time step, look at "this" time step's array and generate postsyn spikes for those neurons. Reset all other dynamics too? Nah, too complex. We'll
//just assume they are pretty much always poisson neurons... Assume that the user will keep track of which neurons are which? Also give him ability to always generate
//spikes at a given probability (i.e. check and add to those neurons at the time when we go through all spikes to add manually, generate them there with probability)
//store data for probability within each poisson neuron (?) -- SO we still have to iterate through all poisson neurons...orz. Whatever, that is their "update" function!
//It's empty if their function is NULL, else it uses some data members/structs to generate with a given probability distribution over time.


//REV: need to know when last time a neuron etc. was updated so I know what decay values to use! Can't use partial decay values if it exits active-list mid dt?
//but let's just make it so that they only exit on whole dt steps! Makes things all sorts of easier :0 Err, but then I have to rewrite cleanup code.

//update a neural impl grp (neural dynamics) (will only ever do this at dt time...?)
void                  update_neural_impl_grp( neural_impl_grp_ID_t _nig, uint s, uint e ); //at a dt?
  void rewire_connections(int syngrp, float proportion_to_swap);
//advance whole network simulation time forward in time
void                  advance2();
void                  advance_small();
void                  advance_big();

//update a single neuron -- will only ever do this at dt_SMALL time?)
void                  update_neuron( neural_ID_t _nid, simtime_t _delta);
void                  update_synapse(syn_ID_t _id, simtime_t _delta);

//REV: for multi
//void                  update_synapse2(syn_ID_t _id, simtime_t _delta);


void update_neuron2(neural_ID_t _n, simtime_t _delta);

Isyn_t update_base_psr(neural_ID_t _n, struct neural_impl_grp* _grp, simtime_t _delta);

Isyn_t update_cond_psr(neural_ID_t _n, struct neural_impl_grp* _grp, simtime_t _delta);
void update_cond_psr2(neural_ID_t _n, struct neural_impl_grp* _grp, simtime_t _delta);

void update_poisson(neural_ID_t _n, struct neural_impl_grp* _grp, simtime_t _delta, Isyn_t Isyn);

void update_lif(neural_ID_t _n, struct neural_impl_grp* _grp, simtime_t _delta, Isyn_t Isyn);

void update_izhi(neural_ID_t _n, struct neural_impl_grp* _grp, simtime_t _delta, Isyn_t Isyn);
void update_izhi2(neural_ID_t _n, struct neural_impl_grp* _grp, simtime_t _delta);

  void add_random_gaussian_current_to_grp(struct neural_impl_grp* g, float mean, float std);
  void set_random_gaussian_current_to_grp(struct neural_impl_grp* g, float mean, float std);

  void set_random_uniform_current_to_grp(struct neural_impl_grp* g, float min, float max);

void spike_poisson(neural_ID_t _n, simtime_t _st);
void spike_lif(neural_ID_t _n, simtime_t _st);

void spike_izhi(neural_ID_t _n, simtime_t _st);

void add_cond_psr(neural_ID_t _n, Isyn_t _weight, simtime_t _delta);
//void add_cond_psr2(neural_ID_t _n, Isyn_t _weight, simtime_t _delta);

void add_base_psr(neural_ID_t _n, Isyn_t _weight, simtime_t _delta);
//void add_base_psr2(neural_ID_t _n, Isyn_t _weight, simtime_t _delta);

Isyn_t update_stp(syn_ID_t _s, simtime_t _delta);
void update_stdp(syn_ID_t _s, simtime_t _delta);

  void init_sim(float, float);
//void init_sim_multi(uint numthreads);
void init_multithreading(uint numthreads);

void init_random();
void init_random_w_seed(long rand_seed);

struct mask_2d
{
  uint x;
  uint y;
  Iinj_t val;
};

struct mask_1d
{
  uint y;
  Iinj_t val;
};

struct img
{
  uint width;
  uint height;
};


struct photoreceptor_mask_2d
{
  uint x;
  uint y;
  float val;
  uint c_radius;
  uint s_radius;
  
  uint c_num_pixels;
  uint s_num_pixels;
  uint* c_pixels;
  uint* s_pixels;
};


extern uint vision_mask_size;
extern struct mask_2d* vision_mask;
extern neural_impl_grp_ID_t vision_grp;

extern uint ear_mask_size;
extern struct mask_1d* ear_mask;
extern neural_impl_grp_ID_t ear_grp;

void set_1d_input_slice(uint _height, neural_impl_grp_ID_t _grpnum, option_t _width_dim, float _n_minwidth, float _n_maxwidth);

void set_2d_input_slice(uint _width, uint _height, neural_impl_grp_ID_t _grpnum, 
			option_t _n_width_dim, float _n_minwidth, float _n_maxwidth, 
			option_t _n_height_dim, float _n_minheight, float _n_maxheight);

//void write_ear_data_to_auditory_nerve(struct neural_impl_grp* ongrp, struct neural_impl_grp* offgrp);
void write_ear_data_to_auditory_nerve(struct neural_impl_grp* ongrp, float scale);
void ear_to_ear_mask(float* _data);

//REV: Idea, do X-LPF in the first transfer, and then Y-LPF in the second one (for all we need to know min/max x and y though).
//void write_frame_data_to_net(struct mask_2d* _mask, char* _framedata, struct neural_impl_grp* _grp, float (*_transfer_func)(char _pixel));
void write_frame_data_to_net(struct mask_2d* _mask, const uchar* _framedata, struct img imgparams, struct neural_impl_grp* _grp, Iinj_t (*_transfer_func)(uint x, uint y, struct img imgparams, const uchar* _framedata));
void inject_current_to_neuron(neural_ID_t _n, Iinj_t _Iinj, struct neural_impl_grp* _grp);

void write_frame_data_to_bipolars(struct neural_impl_grp* ongrp, struct neural_impl_grp* offgrp);

Iinj_t directinjbase(uint x, uint y, struct img imgparams, const uchar* _data);

Iinj_t directinjbaseYLPF9(uint _x, uint _y, struct img imgparams, const uchar* _data);
void frame_to_vision_mask_YLPF9(struct img imgparams, const uchar* _data);
void frame_to_vision_mask(struct img imgparams, const uchar* _data);



void reset_neural_grp_random(neural_impl_grp_ID_t gid);
void reset_neural_grp(neural_impl_grp_ID_t gid);
void reset_spike_buffer();

//http://www.aldebaran-robotics.com/documentation/naoqi/vision/alvideodevice-api.html#ALVideoDeviceProxy::getGVMColorSpace__ssCR
//functions to get angle from image position
//function to get frame rate of camera
//function to get image size and format.
//Use setColorSpace(id, kBGRColorSpace = 13) (for use with opencv.. :D). REV: crap, we need to do mask LPF beforehand, or else we can't do both X and Y linear kernels
//from just x, y positions without wasting a lot of computation. Do it during copy :D
//Y returns only 8 bits!
//Set params to disable white balance etc. via setParams

//note we can also use it to get img size (w*h), and use it to find angles (getAnglesfromPos()) etc.? Does it take into account camera lens eccentricity?

//use kYuvColorSpace, at qqVga?). Subscribe. kQQVGA=0. kYuv=0, fps=30

//Note, we expect that in these modes, we should get back a pointer to a place with 8bits*160*120 = 19200. If we subsample e.g. 1/5 it will be 32*24 = 768. And LPF.
//For each of 760, we will have 2 bipolar cells (i.e. 1500 neurons). And for each bipolar cells we will have 1 centre-surround (only one level?), i.e. 3000 neurons.
//for each centre-surround, we'll have e.g. 4 angle detectors, so 1500*4 = 6000. Total 9000 so far (shit). Then there will be interneurons...
//So, let's do once every e.g. 10, so it will be 16x12 = 192. Times 2 for PBC, i.e. 400 (total 600n, 600syn)). For each bipolar, one C-S, i.e. 400 (1000n, 600+(numeachcs=9)*400 ~= 4000 = 4600syn). And 4 orientation detectors for each of those (4e.g.  syn per orientation detector?, 1600*4 = 6000, total 10000syn). Plus inhibitory, about 8 per
//inhib(?) makes 4000, total 15000syn. One to each SC, i.e. another 200 + internal SC synapses (maybe 1000? = 16000syn). And then whatever else we model.
//400, so 1600. Total 2600 neurons. Interneurons (another 500?), 3000 neurons. Then we have SC which is about 200 neurons (one layer) to map locations, 3200 neurons.
//So, I'll estimate about 4000 neurons and 20000 synapses for the whole oculo-motor system. But we're only going to take a circle-ish shape, so a few less bipolars.

void update_base_izhi_grp(struct neural_impl_grp* g);
void update_cond_izhi_grp(struct neural_impl_grp* g, uint s, uint e);
void update_base_lif_grp(struct neural_impl_grp* g, uint s, uint e);
void update_poisson_grp(struct neural_impl_grp* g);

void randomly_shuffle_3d_pos_between_grps(uint grp1, uint grp2, float shuffles_per_neuron);
void print_impl_grp_spiking(uint grpnum, FILE* f);







//REV: one of these will be raw, and I just won't write to it ;) NO, I'll just leave a space for it...
struct display_visualization
{
  BOOL is_rawimage; //FALSE if it's neurons, TRUE if it's e.g. some sort of raw image..
  char desc[DESC_MAX_LEN];
  
  IplImage* img; //which contains width, height, etc. info
  struct mask_2d* mask;
  struct mask_2d* mask2;
  struct mask_2d* mask3;
  int neuron_radius;
  struct neural_impl_grp* grp;
  
#ifdef OPENCV
  struct CvRect loc_in_overall;
  struct CvRect text_loc;
#endif

  uint layer_num; //layer, from left to right
  uint order_num; //order within layer...?
  
  float update_frequency_ms; //e.g. 33ms
  //float* temporal_avg_activity; //REV: using mask val for same thing
  float min_activity, max_activity;
};



struct neural_grp_visualization_boundary
{
  float ngrpmin_w; 
  float ngrpmax_w;
  float ngrpmin_h;
  float ngrpmax_h;
};

void update_display_vis_based_on_neural_Vm(struct display_visualization* dv);
void update_display_vis_based_on_neural_firing(struct display_visualization* dv);

void set_display_visualization_update_frequency_and_value_bounds(struct display_visualization* dv, float update_freq, float min_val, float max_val);

//Allow them to set Iinj type (shit?) to any value, i.e. it could be GABA, etc...? But we choose to do it as avg spiking activity.


extern struct display_visualization* group_display_visualizations; //this will have all the stuff needed...
extern uint num_group_display_visualizations;

struct display_visualization* add_display_visualization(char* newdesc, int newneuron_radius, struct neural_impl_grp* newgrp, struct neural_grp_visualization_boundary bounds, 
							BOOL set_is_rawimage, float pixels_per_unit_space, struct img imgdims, uint new_layernum, uint new_ordernum);


IplImage* create_overall_visualization();
extern IplImage* overall_display_visualization; //displayimg always contains raw?


void copy_visualization_to_overall(struct display_visualization* dv);
void draw_visualization_from_vals(struct display_visualization* dv);
void copy_mask(struct display_visualization* dv);
void copy_mask2(struct display_visualization* dv);

void make_photoreceptor_struct(struct photoreceptor_mask_2d* mask, uint C_xpos, uint C_ypos, float init_val, uint C_RADIUS, uint S_RADIUS, struct img imgdims);
void inject_mask_current(struct photoreceptor_mask_2d* masks, struct neural_impl_grp* grp);
void neural_input_from_monochrome_image_with_mask_2d(struct photoreceptor_mask_2d* masks, struct img imgparams, struct neural_impl_grp* grp, uchar* image, 
						     void (*_pixels_to_inj_current_funct)(struct photoreceptor_mask_2d* cb_mask, uchar* cb_image) );

void set_visualization_vals_from_neural_param(struct display_visualization* dv, float (*set_per_neuron_funct)(uint nindex));

void bipolar_OFFc_transform(struct photoreceptor_mask_2d* cb_mask, uchar* cb_image );
void bipolar_ONc_transform(struct photoreceptor_mask_2d* cb_mask, uchar* cb_image );

void makemask_2d2(struct mask_2d* *mask, struct img imgparams, struct neural_impl_grp* grp, struct neural_grp_visualization_boundary bounds);


void makemask_2d(struct mask_2d* *mask, struct img imgparams, struct neural_impl_grp* grp, float ngrpmin_w, float ngrpmax_w, float ngrpmin_h, float ngrpmax_h);

//call this from robot copier thing.
void lowpass_9_x_optim(const uchar* src,
		       uint w,
		       uint h,
		       uchar* dst);

void lowpass_9_y_optim(const int* src,
		       uint w,
		       uint h,
		       int* dst);

void save_net(const char* filename);
void load_net(const char* filename);


  //GPU STUFF //
void init_GPU();
void reset_GPU();
void advanceGPU(int eargrpnum, float earscaleb);







#ifdef  __cplusplus
}
#endif









int findrandidx(float randnum, float* probs, int max);
float syn_branching_factor(float pre_n, float density_um3, float volume_um3);








#endif //end header guard
