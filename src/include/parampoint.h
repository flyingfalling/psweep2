
#pragma once

#include <functrep.h>
#include <utility_functs.h>

#include <boost/mpi.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <hierarchical_varlist.h>

#include <string_manip.h>

#include <fake_system.h>


struct seedgen
{
  std::seed_seq seq;
  std::default_random_engine reng; //keep my own in here to generate values to generate seeds...
  
  
  inline void seed( const long& seedval );
  

  //Hopefully it keeps state...?
  //REV: Generate random int in here from our chain and use it as seed...not very beautiful, but same as doing seeds(1)
  inline std::uint32_t nextseed();

};

struct parampoint_coord
{
  //bool working;
  size_t parampointn;
  size_t psetn;
  size_t pitemn;


  inline parampoint_coord();
  
  inline parampoint_coord( const size_t& pp, const size_t& ps, const size_t& pi );
};
  

//REV: OK so this is it. This is what we build. Note this is a SINGLE representation of a PSET (not a parampoint)
struct pset_functional_representation
{
  functlist myfl; //default constructed? Single static? No state.
  std::vector< functrep > frlist;

  size_t pset_width; //width i.e. num threads or whatever of this pthread.
  std::string myname;
  
  //Takes some registered functions struct (list?), and the (unrolled) stmnts thing.
  //    std::vector< client::STMNT >& stmnts ) //so, we take the STMNT list of this pset, and make what we need.
  inline pset_functional_representation( client::PSET& p );
  


  
};



//REV: this is a worker item. I.e. a single parameterized worker thing.
struct pitem
{
  
 
  //std::string mycmd; //the main thing, this is the cmd I am running via system()?
  std::vector< std::string > mycmd; //Needs to be catted.
  
  std::vector< std::string > required_files;
  std::vector< std::string > success_files;
  std::vector< std::string > output_files;
  std::string input_file;
  
  std::string mydir;

  long myrandseed;
  
  size_t my_hierarchical_idx; //index in my parampoint hierarchical varlist array of my leaf node.

  std::vector<size_t> setlocalidx;
  const std::string __WORKER_IDX_FLAG="__LOCALIDX";
  
  //REV: REQUIRED for boost serialization (to send over MPI)
  friend class boost::serialization::access;
  template<class Archive>
  inline void serialize(Archive & ar, const unsigned int version);
  
  
  //In new version, this may call from memory to speed things up.
  inline bool execute_cmd( fake_system& fakesys, memfsys& myfsys );
  
  

  //Basically, OLD MYDIR is X, NEW MYDIR is Y
  //**** I will now rewrite all of:
  //1) REQUIRED file list (these will all be copied from source location to MYDIR/required_files and renamed as well). We keep that "renamer" around.
  //2) SUCCESS file list (these will all be transferred to MYCMD, no renaming as some might be hardcoded in user program? In which case if DIR doesn't exist
  // and we can't specify it, it will error out -- so we will check to make sure it is forced to be in user dir). We copy these back.
  //3) OUTPUT file list (these must be inside MYDIR or else error, so I will rename them). We copy these back (and read out?)
  //4) INPUT file (name) just a single one (rename this and specify, inside MYDIR).
  //5) MYDIR string (just a single one) -- will modify to user.
  //6) MYCMD, a vector of strings, will be concat with some "SEP" at the end. Find all of the above changed guys, stringmatch, and change to new updated
  //      guys. Note, change to some canonical form first, to handle stuff like ../blah versus /local/blah, etc. I.e. use PWD, etc. Do that later.

  //####  OK DO THIS STUFF ;)


  //SLAVE waits for MESG. Until it has WORK.
  
  //First thing is from MASTER to SLAVE, I call a function that sends PITEM.
  //SLAVE receives PITEM. Makes dir locally (same dir?)
  //MASTER sends REQ files (#FILES, followed by FILENAME, FILELENGTH (BYTES), FILE CONTENTS).
  //SLAVE receives REQ files one at a time, and writes to arbitrary filename (MASTER side does the PRE/POST conversion? No, do it on SLAVE side and store
  //  remember the PRE/POST names)
  //NOW MASTER is done (just waits for finished)
  //Now on SLAVE side:
  //check/rewrite SUCCESS files from SOURCE mydir to NEW mydir
  //check/rewrite OUTPUT file list from SOURCE mydir to NEW mydir
  //check/rewrite INPUT file (name) from SOURCE mydir to NEW mydir
  //check/rewrite MYDIR from SOURCE to NEW
  //for all MYCMD, match SOURCE of any of the above, and switch to NEW.

  //Execute the CMD. REDO, until it is done. If error, return some MESG that indicates what happend (i.e. failed to run, didn't have success files,
  //etc. These return to MASTER and cause error at MASTER

  //Check SUCCESS files existence.
  //Read out OUTPUT files (don't bother copying back?)
  //I could copy back SUCCESS files if I want to (do it later?)
  //SLAVE says "we're done!", MASTER receives.
  //SLAVE sends OUTPUT. Master recieves OUTPUT varlist.
  //SLAVE sends # FILES (and correspondences?), then sends one file at a time.
  //SLAVE is DONE.

  inline void rename_to_targ( std::vector<std::string>& targvect, std::vector<bool>& marked, const std::string& olddir, const std::string& newdir,
		       const std::vector<std::string>& oldfnames, const std::vector<std::string>& newfnames );
 

  inline void rename_str_to_targ( std::string& targstr, std::vector<bool>& marked, const std::string& olddir, const std::string& newdir,
			   const std::vector<std::string>& oldfnames, const std::vector<std::string>& newfnames );
  

  inline void rename_str_to_targ_dir( std::string& targ, std::vector<bool>& marked, const std::string& olddir, const std::string& newdir );
  
  
  inline void rename_to_targ_dir( std::vector<std::string>& targvect, std::vector<bool>& marked, const std::string& olddir, const std::string& newdir );
 
  
  //REV: For success etc. check existence AND THAT IT IS A FILE!!!! I.e. we don't do DIRS (for now)
  inline void re_base_directory( const std::string& olddir, const std::string& newdir, const std::vector<std::string>& oldfnames, const std::vector<std::string>& newfnames );
 
  inline pitem( );

  inline void set_local_worker_idx_flag();
  
  inline pitem( pset_functional_representation& pfr, const size_t idx,  hierarchical_varlist<std::string>& hv, memfsys& myfsys, const std::uint32_t& myseed, const bool& usedisk=false );
 
  inline std::vector<std::string> check_ready( memfsys& myfsys );
  
  inline std::vector<std::string> checkdone( memfsys& myfsys );
  
  
  inline varlist<std::string> get_output( memfsys& myfsys, const bool& usedisk=false );
  

  inline std::vector<std::string> get_cmd();
 
  
  
};


struct functional_representation
{
  std::vector< pset_functional_representation > pset_list;
  
  //Constructs the list of each parampoint. Great.
  inline functional_representation( client::PARAMPOINT& pp );
  

  inline functional_representation();
  
  
};



//REV:
struct pset
{
  
  
  std::string mydir;
  std::vector< pitem > pitems;
  size_t my_hierarchical_idx; //index in my parampoint hierarchical varlist array of my leaf (or middle) node.

  /*bool done;
  
  
  bool checkdone()
  {
    if(done == true)
      {
	return done;
      }
    else
      {
	done = true;
	//checks if this pset is done. Specifically, by checking if all (remaining) pitems are done! If not, wait until they return.
	for(size_t pi=0; pi<pitems.size(); ++pi)
	  {
	    if( pitems[pi].checkdone() == false )
	      {
		done=false;
		return done;
	      }
	  }
      }
    fprintf(stderr, "ERROR in checkdone in pset, why am I reaching end?\n"); exit(1);
  }
  */

 inline  pset(hierarchical_varlist<std::string>& hv);
  
  
  
  inline void add_pitem( const pitem& p );
 
  
  
};

//REV: need a way to set that it is "done" too, i.e. go through and re-write back the files (?) to master, and check they're all there, and BAM.
//Furthermore, need to only give next pitem (next pset) when that is done.
//Anyway, need to rewrite this, or make a 3-rd party struct that keeps track of "done"-ness. Problem is if we check done-neess of previous but unneeded
//guys at local worker location, will fail because we just didnt copy files locally to TMP so be careful. Mm.
//Waste of time to look through ALL parampoints after gen 300k or something, so best to have a "start" point or something? That we periodically check.
//That makes the  most sense, not elegant though.


struct parampoint
{
  std::string mydir;

  std::vector< pset > psets;

  

  static const size_t my_hierarchical_idx = 0;


  inline void add_pset( const pset& myps);
  
  //bool done=false;

  /*
  pitem get_next_pitem()
  {
    for(size_t p=0; p<psets.size(); ++p)
      {
	if( psets[p].checkfarmed() == false )
	  {
	    return psets[p].get_next_pitem();
	  }
      }
  }
  */
  

  

  
  //Can do "check done" but better to just "get next pitem"
  /*
  bool checkfarmed()
  {
    for(size_t p=0; p<psets.size(); ++p)
      {
	if( psets[p].checkfarmed() == false )
	  {
	    return false;
	  }
      }
    return true;
  }
  
  
  bool checkdone()
  {
    if(done)
      {
	return done;
      }
    else
      {
	done = true;
	for(size_t p=0; p<psets.size(); ++p)
	  {
	    if( psets[p].checkdone() == false )
	      {
		done = false;
		return done;
	      }
	  }
      }
  }
  
  pitem get_next_pitem()
  {
    for(size_t p=0; p<psets.size(); ++p)
      {
	if( psets[p].checkdone() == false )
	  {
	    return psets[p].get_next_pitem();
	  }
      }

    fprintf(stderr, "REV: get_next_pitem: ERROR, we are trying to get next pitem despite PSET being ostensibly finished! Error!\n");
    exit(1);
  }
  */


  //Makes the dir at the location "inside", but success and required files are still treated as if they are purely global names.
  inline parampoint( hierarchical_varlist<std::string>& hv, std::string dirname );
  


  
  //REV: this will always be the root node in the parampoint hierarchical varlist array...
  
};



//This should work. I need to repeat this, and push back the new guy in place of the old one. Repeatedly until all check_atomics are == 1
//not threadsafe heh.
//Wait, this will lead to a problem, in that we will only call this if it is not a SET_VAR.
//Wait, we also want to call this on SET??!?!!



//REV: this is absolutely nasty. I will return two things: one is the new (checked) SET_VAR with some guys reset. Other is list of new STMNTS


//Make a thing that takes the client::STMNT or LEAF_STMNT, and produces
//an executable thing (based on variables). Note, FUNCTNAMES cannot be
//variable I assume...? I could do that later.

//So, takes a STMNT, poops out a thing that actually searches for the function, binds the executable functional, and has args, and allows it to be "called", along with variable references etc. to extract it? In the end, functions all just take string lists. They're guaranteed to be first-level.
//Functs always return a string, and take a string vector argument?
//nargs is the

//REV: basically takes a "stmnt" passed by user and applies this to it.

//REV: Functions must have access to appropriate variable lists to do things.




struct executable_representation
{
  client::PARAMPOINT ppscript; //This is the list of PSET things, which are lists of STMNTs
  //REV: current problem is that we can't have statements outside of psets? I.e. we want global PARAMPOINT things to have an effect.
  //Let it be for now heh. THIS IS JUST THE SCRIPT SIDE. Need to go build the function side too.
  //To do that, the ppscript needs to have access to the VARIABLES themselves? No, not yet. I can build it without that.
  //But, I need to know what the functions do? I.e. I need POINTERS to the actual functions. Right. So I build that. And for stuff like
  //for loops etc., if they are nested in the script...no they won't exist. I.e. there's no online parsing of loop variables etc.

  //This is the functional representation of the script, specifically a list of the things to do.
  functional_representation fscript;
  
  inline executable_representation();
  
  
  //build the exec rep using config file(s) passed by user.
  //For now, force it to be a single file...
  inline executable_representation( const std::string& script_filename );
 
  //FSCRIPT now contains the list (vector!) of pset functional representations, at construction.
  //Constructing a parampoint involves actually executing and generating the workers based on the passed param vals passed.
  //I.e. for each
  //  pset p in fscript.pset_list[p], do
  //   for each stmnt s in fscript.pset_list[p].frlist[s], do
  //    fscript.pset_list[p].frlist[s].execute(hiervarlist_vect, mypitemidx_corresponding_HVL)

  //Make certain set functs like getworkingdir(), etc.

  // To make this easier, I can literally generate the workers. Which have effects, such as filling REQUIRED, and SUCCESS file variables
  // (to actually check), as well as CMD variable. And other variables, and other side effects such as directories to generate, move commands to execute,
  //etc.

  //Builds and returns a parampoint. Which can then be executed in its own way.
  inline parampoint build_parampoint( hierarchical_varlist<std::string>& hv, const std::string& dir, memfsys& myfsys, seedgen& sg, const bool& usedisk=false );
  
};




//REV: 1 Jan 2016. I'm going through this linearly.
//1) We will have a farm funct that calls a parampoint generator to compute the results for a given parameter point. Passed as a single varlist ?
//2) This requires having the (specific) parampoint (variables) set properly. It also requires the SCRIPT for how to generate each
//   param point.
//3) This is decomposed into the individual PSETS () and PITEMS (). It checks required files etc. and resolves variables at that point.
//   We only execute the PITEMS at execution time
//4) We finally generate the CMD string (variable), which is executed at the end of the script (?)

//We construct a parampoint generator which has the set of named vars (the global ones), as well as the (list of) param point vars for each param point.
//Those are (constructed by the) made by the script. OK, start out with root which has the specific ones passed to me...


//Something will call generate in this. Specifically, the farm_funct. There is one of these made for the whole sweep full of the named variables etc.
//I need to specify the exec_rep (which is the list of executable statements for each PSET), which is generated from the script in turn.


//Two methods, one to get next pitem (and parampoint corresp parampoint num? Bc I need to copy it duh -_-)
//One to check if it's done. I.e. I need to know whether it is finished/successful.
//Another performs the actual farming out and sets to "being processed". Furthermore we mark which worker it is that is doing the processing?
//Have a list of currently processing. PARAMPOINT/PSET/PITEM. Linked list or something... deque.




struct pset_result
{
  std::vector< varlist<std::string> > pitem_results;
  inline pset_result( const size_t& nitems );
 
};

struct parampoint_result
{
  std::vector< pset_result > pset_results;

  inline parampoint_result( const parampoint& myp  );
  
};


struct parampoint_generator
{
  //This needs to have the ability to take current list and generate new ones?
  
  std::vector< hierarchical_varlist<std::string> > named_vars;
  std::string basedir="./";


  //One made for each parampoint we make.
  std::vector< hierarchical_varlist<std::string> > parampoint_vars;
  std::vector< parampoint > parampoints;
  std::vector< parampoint_result > parampoint_results;
  std::vector< memfsys > parampoint_memfsystems;
  
  

  
  //It's a single executable representation...
  executable_representation exec_rep; //representation of the "script" to run to generate psets (param point) etc.

    
  inline void set_result( const parampoint_coord& pc, const varlist<std::string>& result );
 

  inline varlist<std::string> get_result( const parampoint_coord& pc );
 

  inline std::vector<varlist<std::string>> get_last_N_results( const size_t& N );
 

  inline varlist<std::string> get_result( const size_t& ppnum, const size_t& psetnum, const size_t& pitemn );
  


  //REV: How to do this? User should specify how often to cleanup...
  //I'll leave it up to user to know when to clean it up.
  //Best to simply "delete" them, i.e. remove from beginning of vector..
  //Meh it's a vector but whatever.
  
  //REV: Might want to also clean up (write to HDF5 log) the stuff
  //from file system if it is saved there? Otherwise it will grow.
  //In other words, memfsystem? I.e. iteratively do parampoint.
  inline void cleanup_parampoints_upto( const size_t& ppidx );
  
  
  inline void log_parampoints_upto( const size_t& ppidx);
 
  
  
  
  //Construct it by specifying global vars or something?

 inline  parampoint_generator();
 
  inline parampoint_generator( const std::string& scriptfname, const std::string& bdir );
  
  inline size_t generate( const varlist<std::string>& vl, seedgen& sg, const bool& usedisk=false );
 
  
}; //end struct parampoint_gen


#include <parampoint.cpp>
