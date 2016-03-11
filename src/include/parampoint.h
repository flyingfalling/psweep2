
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
  
  
  void seed( const long& seedval );
  

  //Hopefully it keeps state...?
  //REV: Generate random int in here from our chain and use it as seed...not very beautiful, but same as doing seeds(1)
  std::uint32_t nextseed();

};

struct parampoint_coord
{
  //bool working;
  size_t parampointn;
  size_t psetn;
  size_t pitemn;


  parampoint_coord();
  
  parampoint_coord( const size_t& pp, const size_t& ps, const size_t& pi );
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
  pset_functional_representation( client::PSET& p );
  


  
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

  //REV: REQUIRED for boost serialization (to send over MPI)
  friend class boost::serialization::access;
  template<class Archive>
  void serialize(Archive & ar, const unsigned int version);
  
  
  //In new version, this may call from memory to speed things up.
  bool execute_cmd( fake_system& fakesys, memfsys& myfsys );
  
  

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

  void rename_to_targ( std::vector<std::string>& targvect, std::vector<bool>& marked, const std::string& olddir, const std::string& newdir,
		       const std::vector<std::string>& oldfnames, const std::vector<std::string>& newfnames );
 

  void rename_str_to_targ( std::string& targstr, std::vector<bool>& marked, const std::string& olddir, const std::string& newdir,
			   const std::vector<std::string>& oldfnames, const std::vector<std::string>& newfnames );
  

  void rename_str_to_targ_dir( std::string& targ, std::vector<bool>& marked, const std::string& olddir, const std::string& newdir );
  
  
  void rename_to_targ_dir( std::vector<std::string>& targvect, std::vector<bool>& marked, const std::string& olddir, const std::string& newdir );
 
  
  //REV: For success etc. check existence AND THAT IT IS A FILE!!!! I.e. we don't do DIRS (for now)
  void re_base_directory( const std::string& olddir, const std::string& newdir, const std::vector<std::string>& oldfnames, const std::vector<std::string>& newfnames );
 
    pitem( );
  
  pitem( pset_functional_representation& pfr, const size_t idx,  hierarchical_varlist<std::string>& hv, memfsys& myfsys, const std::uint32_t& myseed, const bool& usedisk=false );
 
  
  std::vector<std::string> checkdone( memfsys& myfsys );
  
  
  varlist<std::string> get_output( memfsys& myfsys, const bool& usedisk=false );
  

  std::vector<std::string> get_cmd();
 
  
  
};


struct functional_representation
{
  std::vector< pset_functional_representation > pset_list;
  
  //Constructs the list of each parampoint. Great.
  functional_representation( client::PARAMPOINT& pp );
  

  functional_representation();
  
  
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

  pset(hierarchical_varlist<std::string>& hv);
  
  
  
  void add_pitem( const pitem& p );
 
  
  
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


  void add_pset( const pset& myps);
  
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
  parampoint( hierarchical_varlist<std::string>& hv, std::string dirname );
  


  
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
  
  executable_representation();
  
  
  //build the exec rep using config file(s) passed by user.
  //For now, force it to be a single file...
  executable_representation( const std::string& script_filename );
 
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
  parampoint build_parampoint( hierarchical_varlist<std::string>& hv, const std::string& dir, memfsys& myfsys, seedgen& sg, const bool& usedisk=false );
  
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
  pset_result( const size_t& nitems );
 
};

struct parampoint_result
{
  std::vector< pset_result > pset_results;

  parampoint_result( const parampoint& myp  );
  
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

    
  void set_result( const parampoint_coord& pc, const varlist<std::string>& result );
 

  varlist<std::string> get_result( const parampoint_coord& pc );
 

  std::vector<varlist<std::string>> get_last_N_results( const size_t& N );
 

  varlist<std::string> get_result( const size_t& ppnum, const size_t& psetnum, const size_t& pitemn );
  


  //REV: How to do this? User should specify how often to cleanup...
  //I'll leave it up to user to know when to clean it up.
  //Best to simply "delete" them, i.e. remove from beginning of vector..
  //Meh it's a vector but whatever.
  
  //REV: Might want to also clean up (write to HDF5 log) the stuff
  //from file system if it is saved there? Otherwise it will grow.
  //In other words, memfsystem? I.e. iteratively do parampoint.
  void cleanup_parampoints_upto( const size_t& ppidx );
  
  
  void log_parampoints_upto( const size_t& ppidx);
 
  
  
  
  //Construct it by specifying global vars or something?

  parampoint_generator();
 
  parampoint_generator( const std::string& scriptfname, const std::string& bdir );
  
  
  //takes an argument which is just a (new?) variable list of the parameters to test, and does stuff. Note some (other fixed parameters) might be global.
  //This ONLY generates, it is totally independent to sending stuff to be executed.

  //I would like to totally dissociate the farming out (of work) from the search function.
  //To make new param points, we do that by calling for new points. No, something is on top of that I guess. I.e. something calls the VL thing.
  //And then farms out the work (inside this structure?) until it is done
  //I.e. control passes to that to farm out that "set" of paramthings at a time? May be a generation, may not be?
  //We pass a set of varlists to execute, and it does everything for me.
  
  size_t generate( const varlist<std::string>& vl, seedgen& sg, const bool& usedisk=false );
 
  
}; //end struct parampoint_gen


//If I set up a "farmer" device, it will contain a parampoint_gen,
//but I'll need internal access to which ones are active etc.
//And a way to signal when a specific worker is finished.
//Need to know parampoint # and worker number in it. It will "wait" to
//check it, write back the files (if neccessary), and mark it done (check
//files locally too). Make MD5 or something?

//I can spin through all of them, checking. Or just wait to receive
//ANY message, I think that is possible.


//What is executed in main thread? There is some main search algo, that we call, i.e. we construct a search funct. And we simply iterate
//it until it is "done" But the search function also constructs (by itself) a parampoint_generator for handling vls at a time. The user
//passes that separately. I.e. he makes a parampoint_generator by himself




//Parampoint generator -- generates an actual parampoint (psets etc.) based on originally passed SCRIPT, and also varlist containing PARAMPOINT
//to evaluate.
//Only generates when called. Keeps track of parampoints as well, i.e. history. Keeps track of which have been "farmed", which are "ready", and
//which are "done". Note each parampoint struct keeps track of this in addition to other information. That is bad, should separate that out? Meh.

//Furthermore, there is a search function that gets outputs from finished parampoints, generates new ones, etc.. I.e. that is the controlling runner.
//It gets a pointer to parampoint_gen and passes guys to it to get out things to run. Finally, when it is finished, it somehow gets out the
//results and does something with them.

//Is the search function at all related to how I actually farm out the guys? No, has to do with workers. Set a "max farmed at a time" type variable,
//so it writes that many parampoints to file as it goes. Much easier that way no obvious "generations" or something.

//Every generation could be a file (?). Easier to have one large file... in which case we need to handle errorful writing? We can correct it...erase
//most recent generation? Or do checking of success.

//What does the SEARCH function do? It has state I assume, may require responses before continuing, or not?
//It generates a bunch of guys at the same time, computes them, gets results, writes to disk, etc., until we decide it's done.

//OK, this is "at a time" value that was selected. However there may be "stops" e.g. if not enough to get a generation. So we don't know what
//it uses to generate the next set of guys. But we know it must be passed a VL as a result. It has its own unknown method for generating.
//Base class, and make it a higher level class. I really don't want to use a pointer or some shit like that?

//Takes generic hierarch varlist that handles things?





//REV: HOW TO DO THIS.


//User builds the search function (specifies params, which function to use etc. In some main proram).
//User also specifies the method for building the param points by giving the script. Specifies #workers etc.
//The search function generates varlists for parampoints, and gets results back. That is all it cares about.
//It knows nothing else.

//Separate than that, we have something that executes parameter points. It goes through a list of parameter points to execute, and does them.
//The list is a LOCAL list (not the full list) of only "this turns" param points.
//So, we build a mini param point generator, and pass that as a chunk, get the results all back.
//Inside this mini list, we will be distributing to workers.

//I guess this is the point at which we write out to HDF5 or at least write out regularly, because e.g. if it is a SWEEP then there will be no
//"generations", and thus no stopping points (because it might be inefficient).

//This works best. Then, we might have a global list of all the param points after.












    //REV: we now have the actual parampoint, we need to/want to execute it now.
    //Also, parampoint_vars has the hierarchical varlist pushed back.
    //We need to actually EXECUTE it in order, doing the CHECKS and running CMD with SYSTEM, getting RESULTS, etc.
    //Note, as of now this just generates parampoints, but it also makes the required dirs etc. We need to literally
    //have the "signal" to each to execute the CMD. But, we need to "get" the cmd. Note, will each worker independently handle itself? Yea,
    //it can all do it itself. We can pass the pitem to the worker, and it will handle everything ;) Otherwise, local /tmp won't work or something.
    //So, serialize it etc. So, the local worker needs to do execution when passed the CMD (in fact the thing itself. FUCK PARAMPOINT CREATION MUST
    //BE DONE BY THE WORKER LOCALLY, or file stuff might be messed up. I.e. construction must be done there. That is not good. I.e. the actuall).

    //REV: why do this here? If we will write it out of course. Problem is of course what? Ah, PSET must be made locally. But we can make/do everything
    //for workers, then copy it to the correct location. That is how it SHOULD work. We ASSUME there is a network drive (lustre etc.) of some kind.
    //Why am I using this and not HADOOP like thing? At any rate, workers give the thing the targets to write to locally (i.e. place to execute from?)
    //I assume we move everything to the RAMDISK. And then copy it only after it is done. I guess that works. Same worker is not guaranteed to do
    //every pset of the guy we need, so in between we need to copy back to the shared file system (and/or have some way to communicate to targets the
    //stuff they need). If we communicate via file system it works with LUSTRE, but it will not work if the file-systems are totally separate.
    //If the file-systems are separate, we have a problem. So it's best to make a shared file-system. But, we could do it over networking too. In other
    //words, move everything via network from all the workers to the "master's" system after it is finished. Question is whether this is worth it or
    //should we give "afinity" to a single box for all pitems? Of course, but meh. How long will copying of results take? Could take a while. But, it
    //is on DISK, so we need to READ FROM DISK, and SEND TO MASTER. Figure this out later...for now we are using shared file system as a hack.
    //To send it back over MPI, it is a bit of a pain in the ass. Because the "master" needs to receive it. Can we have the "master" spinning? Ugh.
    //Should we only send "output" files, or EVERYTHING the user's program completes? Note, of course SUCCESS files are needed. And they are same as
    //next's REQUIRED files.

    //Option --USE FS (accessible from all workers), or

    //Best option: Workers have a "scratch" space, they "know" to automatically copy everything to /tmp (or not), do their work, then copy back.
    //Furthermore, every worker (when created) has all the files he needs created. Note, user specifies "required" files, he can specify "data: X" to
    //tell to get it directly from DATA location, rather than typical copied location. That way it is success file.
    
    //How about normal files? They "require" from worker, but heck they'll all be copied. Make everything with a "worker-base" which can be modified.
    //In other words, um, all file paths are modified to be from worker base. At worker spin-off time I guess. I.e. the "overall" guy is already created,
    //then everything is copied and executed by worker.

    //PROBLEM:
    //In CMD, we need to construct at WORKER time!!!!! Which is the real problem, because we may need to specify to USER PROGRAM via CMD that
    // it should use FILE X FILE Y etc., which are IN /TMP or something, not the original locations in shared network drive.
    //So, although we get relative file locations from user, and it creates them in shared space, at actual execution time (CMD construction time?)
    //we modify the corresponding file paths to refer relatively to worker /tmp base.

    //How to do this? When a WORKER gets a PSET, it gets the raw unadultered one? All file references (required files) are COPIED (OK...fine?)
    //and all data files are already existing (likewise?). At /tmp locally. Problem is, we need to modify the REFERENCES in the script (specifically
    //in CMD) so that they refer to proper TMP locations. And this needs to be done BEFORE cmd is generated, i.e. before EXE happens.
    //Creation of dirs output of files might take time, so why not just delegate it to workers themselves. I.e. send the parampoint to the worker,
    //and have it do the required creation? Nah, pain in the butt. But, creation of the PITEM is good. Problem is e.g. there might be "required
    //input" from previous PSETS, thus we can't just copy the worker dir. One major problem is that multiple previous PSET PITEM might have overlapping
    //filenames!!!! E.g. if they are all named equivalently "output". Maintaining a copy of everything on each machine is out of the question I assume.
    //However, we can maintain local FILE SYSTEM reference locations? So, recall things like PREVIOUS_PSET( outputfile ) must appropriately resolve
    //AT RUNTIME to point to local TMP locations, where they were copied or something (i.e. dirs must be set). So, after all, we will create a whole
    //DUMMY directory sturcture of each PARAMPOINT at each WORKER, but will only FILL as necessary. How do we coalesce? Well, we "writeback" only the
    //CURRENT (this PITEM) directory to the global. I like that. So, all directory references must be constructed/generated online with respect
    //to PARAMPOINT-HEAD (or "DATAHEAD"), which in case of /TMP system will be temporarily reset to /TMP absolute file location BEFORE generation of
    //CMD (i.e. of that PITEM). However, it must be done globally first, so that all PITEMS write out their appropriate files in the first place? No,
    //that is not necessary. Just go one by one, creating and running. When we pass to a worker, we already are checking REQUIRED/SUCCESS files,
    //from the "already run" pitem creation, and we copy those accordingly to the corresponding /TMP file locations (automatically, if they exist).
    //Then, we RECONSTRUCT the PITEM at the target location (we don't bother creating the dir right??? We just need to change file locations in
    //variables/strings. That is the problem. Best to have them all "constructed" from a source at runtime to generate the CMD. We don't generate
    //CMD until the very end, after we may have changed "base" to appropriately reflect everything. I like that idea.

    //Still minor problem of how to copy/create all the "old" guys from previous PSETS at the /TMP location...
    //Also recall that we have some "minor" problems in that some functions can have other side effects, such as creation of things, running
    //programs etc.? So, require user to have a "setup" that specifies required/blah files before a "run" cmd? I.e. creation of the CMD variable.
    //Nono, all stuff to set CMD is set...shit. This doesn't work. User's thing will actually create teh CMD anyway ugh. So, yea, we need to
    //re-run it again, but this time, with all success files etc. locally, so that CMD resolves correctly.
    //If the script has side effects or something, that is a minor problem, as we will be running it twice...
    //Shit, it will be adding variables etc., that is the problem!
    
    //So, we will just execute the "set_CMD" command one more time at the very end, but now the passed varlist will have "basedir" set to something else.
    //Note everything is strings or shit, so it has no idea that the STRINGS are FILE PATHS. So that is just nasty, because we don't know when user
    //may set a string var to a file path and then use it later without calling the correct "function" to get it in a locally valid manner.
    //Let user make all file references in some way that is safe, i.e. give a function that pads the string at beginning with the base dir.
    //OK, and then hope user uses that if he knows what he is doing ;) Like PREFIX_BASE_DIR( blah ). of this pitem, or a previous psets Nth pitem, or
    //whatever. Note that an array of variables needs to be done in that way too..? For reading arrays of previous guys, ugh. So, we will SKIP all
    //SET_VARS? No, that won't work. At the end, our only option is to run the user script, get the REQUIRED/SUCCESS variables copied. Then,
    //we will NOT automatically parse everything to fix the paths... but we DO need to re-run everything to re-generate from scratch.
    //AH WE CAN DO THIS JUST CLEAR OUT THE HIER VARLIST AND START FROM SCRATCH. But now with everything set to /TMP. OKOKOKOKKOKOKOK. GO.
    //This assumes EVERYTHING IN VARLIST WAS CREATED BY THIS SCRIPT, i.e. there is nothing else in the varlist.
    //But we might be getting varlist stuff from elsewhere (e.g. named etc.?). Nah, disallow that.
    
    
    //So, todo is to make sure that user specifies files all as happening globally. E.g. required files and success files etc. Those are all COPIED
    //to a given location, and then file paths are set to those new locations I assume. Problem is of course before CMD generation, which may reference
    //those. Even if it's global, we still need to SEND to worker on different computer. Have that all happen at start (copying data files).
    //But that's a problem as we have to generate them? Nah...but its "required" for a given guy. I.e. specify "data" files. Don't recopy every time?
    //Assume that um, individual workers will have blah? Ah, for each worker. Based on script? Ugh. Make user specify everything in there?

    //At any rate, send it all over a port? Or MPI setting? I guess I can serialize it all as binary and send over MPI ports, then write to file
    //just like that. Seems a waste to send over MPI though..? But is there any other way? SSH or SCP or some shit? Either way it will go over network,
    //and be written/read to file. Specify the "data folder", and just leave it there or something? "Generate" name lists?

    //So, this is part of the "actually executing". First, I run it, and do all the copying etc. to the worker who has "hit" it. Passing the parampoint.
    //Build the structure at the worker's /TMP location. The worker has its "base directory" in some way, just like other guy. But all guys are copied
    //with relative path to TMP, e.g. in data files etc. If there are name clashes, we have a problem? E.g. multiple global dirs with sub-dirs with
    //overlapping names etc.? That is a pain in the ass. If there is a clash, I will re-name it. But user program might not know I changed it, it might
    //assume a certain filename? E.g. if I pass a "data dir" but not individual data file names or something? Hopefully user doesn't do that. Don't do it
    //unless you will use it policy! Forget it for now, but it can be resolved heh.

    //So, at initial position, when calling, we send the varlist along with it so it knows. But, on THIS side, we do the "sends" of all the guys to the
    //selected worker. Who is "waiting" for it. Gets the stuff, writes it.

    //At the same time I have changed the names of all the files to be different/local. I.e. for each file I have send/copied (based on var), I have
    //changed the name to some arbitrary local one at the target location. They may start in some diverse locations though... so I need a way to
    //copy them? Just name them arbitrary things? Assume user program does not expect some named file or else. Meh, depending on loc? We can give
    //it new local name too or something? For each? But then we must pass it to CMD.

    //Offer function that has underlying impl. It may be MPI, etc. We don't know. Hadoop? OK, we can do that later (DFS?)
    //Workers sit and wait.
    
    //Hadoop?
    



//We have the parampoint_gen constructed. We need to build (another?) one of these locally, in other words this is passed in memory (ugh).
//Should only pass the necessary things....no need to pass previous param points etc.? Passing contents of vectors etc. pain the but wow...
//Anyway, so I pass the constructor thing along with the filenames etc. So, first all the required files are sent with their new names and locations,
//and this side worker writes them to TMP or whatever.
//Note settings might be different per workstation? Ugh. Infiniband??? Ethernet speed times two at most... But disk write speed will be holdup.
//How long will it take to transfer what, 10 kb of data? At 10 Mbps that is already um, 10 ms, but 10 mbs, so 1/10 of that, i.e. 1 ms. Assuming we have
//6 K80s, with 2 each in them wow. That is 12 GPUs. So, with 12 GPU, we can be amazing. We could even run multiple on each (if we can). If I use same
//spawning worker, I could theoretically run multiple kernels on same GPU. But only if it is actually only executed from the same GUEST program!!!!
//I.e. the C++ simulation. So, that has to be executed from in the same thread even (!!!!!??). I.e. in concurrent. Ugh.

//Note, host program has to access GPUs automatically. Meh. Host # etc., they need to communicate which GPU is open! Pain in the butt I guess. And
//data about the GPUs. So I can get CUDA info into main program and use it to feed them stuff. Based on estimated size etc? But I don't know about that?
//Can I Know which are "open" and which not from CUDA probing?

//Either way I need to send it to the workers. So I have a worker, it signals its ready. I select it, I send it. The other side is waiting to receive
//some format?
//Just send as bytes I guess. Or ints. Or whatever. Must be bytes. Source and dest machine must match up I guess? Sending INT etc. Must send as chunks
//because file size might be very large (larger than INT_MAX). Make sure file write is not blocking? Have 2 threads or something...?
//

//Worker spins and executes only when got stuff. Same as other, send "ready" to master. Just as before, but now I send so much more. Send "new" filename

//dynamically load a function (model), pass data that way?

//User needs to include the header file of his lib to have it compute properly.

//I.e. force user to have his header etc. included, and then compile with .so included? No, can be after.
//Or force user to compile with???

//All nasty, still we're hitting LDAP too much or something. Is there a way to get current process to "switch" its identity? I.e. same thread runs,
//but we "override it" multiple times? Or we could spawn new one each time or something... We'd like to load everything at start time.

//I just realized I may be thinking of this backwards. Another highly likely way to do this is to create as a LIB, the sweep program, and then include
//that (as C++ include or shared lib) into my main C++ model thing. And then, each "rank" is just one of my main model things running.
//So, I include it, and then I in each thread, start the libs worker thing, which spins and waits? And when it gets it I simply call my own model code
//with the stuff from the main guy. Problem is that e.g. user must generate his own main function? Nah, just pass the function signature or something?
//At any rate, we are passing everything back as memory, which solves a big problem. This may be best/necessary for cuda implementation. Note that each
//worker can then execute SYSTEM call for non-C++ guys and communicate with files in that way. That is the best idea? It keeps C++ main while allowing
//other guys to happen. Problem is that user must modify his own code to use the library, and write the script I guess. Specify SYSTEM commands versus
//some other way of scripting things without modifying the user program main stuff. E.g. constructing CMD for other guys. For direct C++ guys, we need
//to specify what memory to pass to it? We know some things like variables, so we can just pass those directly. Force user to use the variables, but
//that's fine. We can pass them by name haha.


//At any rate, need to figure out how to communicate required files or whatever to target, as correct variable names (filenames)

