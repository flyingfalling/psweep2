//REV: Todo: log/offput memory of PARAMPOINT data from PARAMPOINT_GENERATOR as we finish comp_pp_list.
//Otherwise, memory will become very large. Especially memfsys, and results, and parampoints, etc.
//For grid search etc., we need to make sure to feed them in "generations" of some kind, which are cleaned up afterwards.
//Otherwise, we will run into memory problems on MASTER.


//REV: Modify this so I can specify WHICH files are actually read/written to file and which aren't. Of course initially all are read in (on master side).
//However some might be purely memory-files, so we go through there anyway.

//On client side, we can specify which to write out to actual files.
//on client side, we want to read-back in data, we need to know which ones to read files with.

//Finally, we need to be able to specify that when building CMD.

//We also want to use FAKE_SYSTEM to call user function.

//So, I need to redo everything pretty much to make sure it works with fake_system.

//really necessary to re-base the filesystem? Sending the list of files kind of pointless? It is read out from user side...add them all to file system
//on that side?

//REV: OK, this is filesender.
//We have one of these (a single one for a sweep). Problem is, this has a fakesys in it. Which has a memfsys in it. That means that all MEMFILES
//will REMAIN, even after I'm done with them? At least on global side? Crap. Ah, but "close" will close it? I can keep it open though....
//When user tries to read from it? Each worker will have the fakesys() over on that side too? Yea, but problem is that memfsys won't be updated...
//fakesys() is sent over? It's created ONLY ON USER SIDE (crap). But user funct is there... No, FAKESYS is created on all workers? It's not related
//at all to script we read etc. We obviously can't send the pointer... 

//Shit, this is a problem. Because, remember, filesender when constructed, it calls the worker on each side without host.

#pragma once

#include <parampoint.h>
#include <variable.h> //this is recursively from parampoint.h I think...

#include <iostream>
#include <string>

#include <boost/mpi.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/filesystem.hpp>

#include <utility_functs.h>

#include <string_manip.h>


#include <memfsys.h>
#include <fake_system.h>

#include <mutex>
#include <thread>

#include <psweep2_cuda_utils.h>


struct psweep_cmd
{
  int SRC;
  std::string CMD;
  psweep_cmd( const int& srcworker, const std::string& cm );
  psweep_cmd()
  {
  }
};

struct filesender
{

  //std::shared_ptr< boost::mpi::communicator > world;
  //std::shared_ptr< boost::mpi::environment > env;

  //REV: This CONTAINS the memfile system thing. So...on other side, I need to re-construct it? Note, user will need to "construct" on this side
  //in real time?
  fake_system fakesys;

  bool todisk=false;

  //REV: I actually only need one copy of this...on ROOT.
  std::vector<bool> _workingworkers;

  size_t workersperrank=1;
  std::mutex mpimux; //this mux is used for MPI communications of the different threads. In case parallel MPI support is not compiled?
  
  boost::mpi::communicator world;
  
  size_t mylocalidx=0;
  int myrank;
  
  //#ifdef CUDA_SUPPORT
  size_t mygpuidx=0;
  //#endif
  
  struct pitem_rep
  {  
    bool farmed=false;
    bool done=false;
    size_t farmed_worker=0;

    bool checkavail()
    {
      if( farmed == false && done == false )
	{
	  return true;
	}
      else
	{
	  return false;
	}
    }
    
    pitem_rep()
    {
      //nothing to do
    }
    
    bool checkdone()
    {
      return done; //actually check it?
    }
  };
    
  
  struct pset_rep
  {
    bool done=false;
    std::vector< pitem_rep > pitems;

    pset_rep()
    {
      
    }
    
    bool checkdone()
    {
      if(done)
	{
	  return done;
	}
      else
	{
	  done=true;
	  for(size_t p=0; p<pitems.size(); ++p )
	    {
	      if( pitems[p].checkdone() == false )
		{
		  done=false;
		  return done;
		}
	    }
	}
      return done;
    }
  };
    
  struct parampoint_rep
  {
    size_t myidx;
    bool done=false;

    size_t current_pset=0;
    std::vector< pset_rep > psets;

    parampoint_rep()
    {
    }
    
    parampoint_rep( const parampoint& pp, const size_t& idx )
    {
      myidx = idx;
      psets.resize( pp.psets.size() );
      for(size_t x=0; x<psets.size(); ++x)
	{
	  psets[x].pitems.resize( pp.psets[x].pitems.size() );
	}
    }
    
    
    void markdone( const parampoint_coord& pc )
    {
      psets[ pc.psetn ].pitems[ pc.pitemn ].done = true;
      bool psetdone=true;
      for(size_t x=0; x<psets[ pc.psetn ].pitems.size(); ++x)
	{
	  if( psets[ pc.psetn ].pitems[x].checkdone() == false )
	    {
	      psetdone = false;
	    }
	}
      psets[ pc.psetn ].done = psetdone;
      if(psetdone==true)
	{
	  ++current_pset;
	}
      
      
      bool parampointdone = true;
      for(size_t x=0; x<psets.size(); ++x)
	{
	  if( psets[x].checkdone() == false )
	    {
	      parampointdone = false;
	    }
	}

      done = parampointdone;


      return;
    }
    
    bool checkdone()
    {
      if(done)
	{
	  return done;
	}
      else
	{
	  done=true;
	  for(size_t p=0; p<psets.size(); ++p)
	    {
	      if( psets[p].checkdone() == false )
		{
		  done=false;
		  return done;
		}
	    }
	}
      return done;
    }
    
  };

  

  //REV: I need to create the FAKE_SYSTEM **before** I actually make the separation to slave loop...
  //static filesender* Create( const std::string& runtag, fake_system& _fakesys, const bool& _todisk=false );

    //REV: I need to create the FAKE_SYSTEM **before** I actually make the separation to slave loop...
  static filesender* /*filesender::*/Create( const std::string& runtag, fake_system& _fakesys, const size_t& _wrkperrank , const bool& _todisk )
  {
    filesender* fs = new filesender(_fakesys,  _wrkperrank, _todisk);
    
    if( fs->getrank() == 0 )
      {
	return(fs);
	//return
      }
    else
      {
	fs->start_worker_loop(runtag);
	
	delete(fs);
	//execute slave loop
	
	exit(0);
      }
    // delete(fs);
    

    fprintf(stderr, "REV: MASSIVE ERROR in filesender CREATOR: I reached end of function, which NEVER SHOULD HAPPEN\n");
    //REV: the other one should naturally delete it here.
  }
  size_t getworker( const size_t& rank, const size_t& tag );
  size_t getworkerrank( const size_t& wnum );
  size_t getworkertag( const size_t& wnum );
  
  void initfilesender();
  bool checkroot();
  void lmux();

  void ulmux();
  int getrank();

  
  void init_local_worker_idx();
  void broadcast_cmd( const std::string& cmd );
  //REV; TODO: at some point, build the fake MEM_FILESYSTEM, and furthermore, populate the FAKE_SYSTEM_CALLS if we want to...
  //Note when we construct and send PITEM, then we are writing to target, but we don't want to actually write out to local one unless we are executing
  //the stuff. In other words. only do it right before execute? Only if execute returns false? Execute takes the stuff. Hmm, we will be writing large numbers
  //of files possibly still, massive waste. So, I need a way to stop it from doing that...
  filesender();
  
  filesender(fake_system& _fakesys, const size_t& _wrkperrank = 1, const bool& _todisk = false);
  ~filesender();

  void send_varlist_to_root( const varlist<std::string>& v, const int& mytag );
  void send_varlist_to_worker(  const varlist<std::string>& v, const int& targworker);
  //void send_varlist(  const int& targrank, const varlist<std::string>& v );

  void send_cmd_to_worker( const std::string& cmd, const int& targworker );
  void send_cmd_to_root( const std::string& cmd, const int& mytag );
  //  void send_cmd( const std::string& cmd, const int& targrank );
  

  void send_pitem_to_worker( const pitem& mypitem, const int& targworker );
  void send_pitem_to_root( const pitem& mypitem, const int& mytag );
  //void send_pitem( const pitem& mypitem, const int& targrank );

  varlist<std::string> receive_varlist_from_root( const int& targworker, const int& mytag );
  varlist<std::string> receive_varlist_from_worker( const int& targworker );
  //varlist<std::string> receive_varlist( const int& targrank );


  //REV: FOr intiial setup
  psweep_cmd receive_cmd_from_root( );

  
  psweep_cmd receive_cmd_from_root( const int& mytag );
  psweep_cmd receive_cmd_from_any_worker( );
  //psweep_cmd receive_cmd_from_any( );
  //psweep_cmd receive_cmd_from_root( );
  
  
  pitem receive_pitem_from_root( const int& mytag ) ;
  pitem receive_pitem_from_worker( const int& targworker ) ;
  //pitem receive_pitem( const int& targrank );
  
  int receive_int_from_root( const int& mytag );
  int receive_int_from_worker( const int& targworker ) ;
  //int receive_int( const int& targrank );
 
  void send_int_to_root( const int& tosend, const int& mytag ) ;
  void send_int_to_worker( const int& tosend, const int& targworker );
    //void send_int( const int& targrank, const int& tosend ); //, boost::mpi::communicator& world )
  

  //So, depending on the RANK, this will SEND or RECEIVE.
  //All SLAVES enter a LOOP of waiting for a MESG.

  //void rename_file( pitem& mypitem, const std::vector<mem_file>& mf, const std::vector<std::string> newfnames, const std::string& dir )
  //This will rename all files in MYPITEM (in SUCCESS only?), given a list of files? An array of files? Yea, it will tell which ones should be renamed.
  //I.e. gives list of indices to it.

  void start_worker_loop(const std::string& runtag);
  //compare two filenames in canonical thing? E.g. /.././../. etc. /. will remove current, i.e. can be safely removed. /.. will remove the previous
  //thing (if it starts with that /.. then error out). Can't handle things like symlinks anyway so whatever...

  //REV: BIG PROBLEM, I need to check whether it starts with a / or not.

  memfsys worker_handle_pitem( pitem& mypitem, const std::string& dir, const int& mytag );
    //memfsys handle_pitem( pitem& mypitem, const std::string& dir, const int& mytag );

  void send_file_to_worker( const memfile& memf, const int& targworker);
  void send_file_to_root( const memfile& memf, const int& mytag );
  void send_file_to_root_from_disk( const std::string& fname, const int& mytag );
  void send_file_to_worker_from_disk( const std::string& fname, const int& targworker );
  
  //void send_file( const int& targrank, const mem_file& memf )
  //void send_file( const int& targrank, const memfile& memf );
  
  //void send_file_from_disk( const int& targrank, const std::string& fname );
  
  memfile receive_file_from_worker( const int& targworker );
  memfile receive_file_from_root( const int& mytag );
    //memfile receive_file( const int& targrank );

  //arbitrary byte array as VECTOR, and we can cast it to a target TYPE in some way? Like...static_cast? whoa...blowin my mind haha.
  //std::vector< char > receive_memory( const int& targrank );
 
  bool cmd_is_exit(  const psweep_cmd& pcmd );

  //REV: This is only for WORKERS
  pitem handle_cmd( const psweep_cmd& pcmd, const int& mytag );

  void worker_notify_finished( pitem& mypitem, memfsys& myfsys, const int& mytag );
  //void notify_finished( pitem& mypitem, memfsys& myfsys );

  //REV: this needs to "find" which PITEM was allocated to that worker/ thread.
  //REV: Note we could use todisk, but easier to do it as todisk?
  varlist<std::string> handle_finished_work( const psweep_cmd& pc, pitem& corresp_pitem, memfsys& myfsys, const bool& usedisk=false );
  size_t get_local_rank( );
  void mangle_with_local_worker_idx( pitem& mypitem );
  
  bool execute_work( pitem& mypitem, memfsys& myfsys );
 

  void cleanup_workspace( const pitem& mypitem );
  
  void execute_slave_loop( const size_t mytag, const std::string runtag="scratch" );
 
  bool is_finished_work( const psweep_cmd& pcmd );
 
  bool is_ready_for_work( const psweep_cmd& pcmd );
 

  

  struct pprep
  {
    size_t pset_idx;
    size_t pitem_idx;
    //a pointer or marker?

  pprep( const size_t& psi, const size_t& pii )
  : pset_idx(psi), pitem_idx(pii)
    {
    }

    //inline bool operator==(const pprep& lhs, const pprep& rhs)
    inline bool operator==(const pprep& rhs)
    {
      if( this->pset_idx == rhs.pset_idx
	  &&
	  this->pset_idx == rhs.pitem_idx )
	{
	  return true;
	}
      else
	{
	  return false;
	}
    }

    inline bool operator!=(const pprep& rhs)
    {
      if( *this == rhs )
	{
	  return false;
	}
      else
	{
	  return true;
	}
    }
  };

  //REV: This keeps track of where there is WORK, which ones have already been farmed, which ones need to be farmed, etc.
  //active means farmed
  struct completion_struct
  {
    std::deque<pprep> farmed_pps;
  
    parampoint_rep state;

  
    //contains um, list of pps, as well as each "index" of each, i.e. where it is in the pg struct?
    //Fuck, so much better to just include this in the parampoint_gen thing. But then we don't know what format workers are etc. So better to
    //separate it out.
    //But we "know" what format of parampoint etc is, i.e. list of psets, and each pset is a list of pitems. OK.
    //I.e. what is the "next one" to farm out in each?
  
    completion_struct( parampoint_generator& pg, const size_t& idx )
    {
      state = parampoint_rep( pg.parampoints[idx], idx );
    }

    bool check_work_avail()
    {
      if( !state.checkdone() &&
	  !state.psets[ state.current_pset ].checkdone() )
	{
	
	  for(size_t x=0; x<state.psets[ state.current_pset ].pitems.size() ; ++x )
	    {
	      if( state.psets[ state.current_pset ].pitems[ x ].checkavail() == true)
		{
		  return true;
		}
	    }
	}
      return false;
    }

    pprep get_next_work()
    {
      if( !state.checkdone() &&
	  !state.psets[ state.current_pset ].checkdone() )
	{
	
	  for(size_t x=0; x<state.psets[ state.current_pset ].pitems.size() ; ++x )
	    {
	      if( state.psets[ state.current_pset ].pitems[ x ].checkavail() == true)
		{
		  //MARK IT FARMED...
		  state.psets[ state.current_pset ].pitems[ x ].farmed = true;

		  //REV: pushing it back here. This is erased OUTSIDE
		  //of it, at a kind of "MARK DONE" type thing?
		  pprep pp(state.current_pset, x);
		  farmed_pps.push_back(  pp );
		  

		  return pp;
		}
	    }
	}
      fprintf(stderr, "REV: THIS (get next work) SHOULD NOT HAVE BEEN CALLED IF THERE IS NO WORK!\n");
      exit(1);
    }
  

    //REV: how to check if all done?
    bool check_all_done()
    {
      //for state, check if all done.
      //go through and update it?
      return state.checkdone();
    }
    
  
  };



  //REV: Always read from files here...assume there will be none shared.
  void master_to_slave( const pitem& mypitem, const size_t& workeridx, memfsys& myfsys );
  


  
  struct work_progress
  {
    //REV: I would delete from this deque as I go???
    std::deque< completion_struct > progress;
    std::vector<parampoint_coord> farmed_status;
    //std::deque< size_t > working_workers;

    bool avail_worker( std::vector<bool>& workingworkers )
    {
      //for(size_t x=0; x<farmed_status.size(); ++x)
      //REV: START FROM 1 BECAUSE ROOT RANK SHOULD BE ZERO!
      for(size_t x=1; x<workingworkers.size(); ++x)
	{
	  if(workingworkers[x] == false)
	    {
	      return true;
	    }
	}
      return false;
    }

    size_t get_next_worker(std::vector<bool>& workingworkers)
    {
      //for(size_t x=0; x<farmed_status.size(); ++x)
      //REV: START FROM 1 bc root rank is 0.
      for(size_t x=1; x<workingworkers.size(); ++x)
	{
	  if(workingworkers[x] == false)
	    {
	      workingworkers[x] = true;
	      return x;
	    }
	}
      fprintf(stderr, "REV: this get next worker should only happen if available!\n");
      exit(1);
    }

    //REV: This is a CONSTRUCTOR. I need to keep around all my memfsys? OK.
    work_progress( parampoint_generator& pg, std::vector<varlist<std::string>>& newlist, const size_t& nworkers, seedgen& sg, const bool& usedisk=false )
    {
      farmed_status.resize( nworkers );
    
      for( size_t n=0; n<newlist.size(); ++n )
	{
	  //make the parampoint (i.e. generate the hierarchical varlists etc. locally).
	  size_t idx = pg.generate( newlist[n], sg, usedisk );
	  
	  add_parampoint( pg, idx );
	}

      return;
    }
		 
  
    //REV: this really needs a POINTER, not a reference orz.
    void add_parampoint( parampoint_generator& pg, const size_t& idx )
    {
      completion_struct cs(pg, idx);
      progress.push_back( cs );
    }
  
    size_t check_any_working( )
    {
      size_t res=0;
      for(size_t x=0; x<progress.size(); ++x)
	{
	  res += progress[x].farmed_pps.size();
	}
      return res;
    }

  
    pitem get_corresponding_pitem_from_workernum( parampoint_generator& pg, const size_t& workernum )
    {
    
      parampoint_coord pc = farmed_status[ workernum ];
      return get_corresponding_pitem( pg, pc );
    }
  
    pitem get_corresponding_pitem( parampoint_generator& pg, const parampoint_coord& pc )
    {

      
      return (pg.parampoints[ pc.parampointn ].psets[ pc.psetn ].pitems[ pc.pitemn ]);
    }
  
    bool check_all_done( )
    {
      bool done=true;
      for(size_t x=0; x<progress.size(); ++x )
	{
	  if( progress[x].check_all_done() == false )
	    {
	      done = false;
	    }
	}
      return done;
    }

    bool check_work_avail( )
    {
      bool workavail=false;
      for(size_t x=0; x<progress.size(); ++x)
	{
	  if( progress[x].check_work_avail() == true )
	    {
	      workavail = true;
	    }
	}
      return workavail;
    }

    parampoint_coord find_first_avail_work()
    {
      for(size_t x=0; x<progress.size(); ++x)
	{
	  if( progress[x].check_work_avail() == true )
	    {
	      pprep firstwork = progress[x].get_next_work( );
	      parampoint_coord pc(x, firstwork.pset_idx, firstwork.pitem_idx);
	    
	      return pc;
	    }
	}
      fprintf(stderr, "REV: we should never have got here in find_first_avail_work()\n"); exit(1);
    }
  
    //Marks it as if it is farmed
    size_t farm_work( std::vector<bool>& workingworkers )
    {
      parampoint_coord pc = find_first_avail_work();

      //This isn't going to work. farmed_pps is in COMPLETION STRUCT.
      //This is WORK PROGRESS. Our progress[] array is list of
      //  COMPLETION STRUCT. We can set individual ones of those.

      size_t workeridx = get_next_worker( workingworkers );
      farmed_status[ workeridx ] = pc;

      return workeridx; //this needs to be marked for other guy...
    }

    void mark_done( const parampoint_coord& pc )
    {
      pprep pr( pc.psetn, pc.pitemn );
      size_t found=0;
      size_t loc=0;
      for( size_t x=0; x<progress[ pc.parampointn ].farmed_pps.size(); ++x )
	{
	  if( progress[ pc.parampointn ].farmed_pps[ x ] == pr )
	    {
	      ++found;
	      loc = x;
	    }
	}
      if( found != 1 )
	{
	  fprintf(stderr, "REV: error in markdone, more than one found!\n");
	  exit(1);
	}

      //Erase the one that is currently working.
      //Mark it done.
      progress[ pc.parampointn ].farmed_pps.erase( progress[ pc.parampointn ].farmed_pps.begin() + loc );

      progress[ pc.parampointn ].state.markdone( pc );
    }
  }; //end work_progress struct...

  //struct farmer
  //{
    //This does everything necessary for it. So on other side, it will do all this. This will include the MPI guys?
    //List of workers, note it is indexed from 1...i.e. 0 is worker 1...
  
  //REV: This is the entry point for MASTER
  //Need to modify for MEMFILE use:
  //1 ) Master to slave
  //2 ) Handle finished work
  //3 ) pitem creation (making input_file)
  //Question is what to do with this stuff.
  //Easiest to just have a memfsys for each parampoint, problem is with
  //things that will be needed by future guys...they must exist.
  //Easiest solution is to have a memfsys for each PARAMPOINT. Ah, that is
  //the best situation... But, then I need to (when I create things)
  //have access to that from PITEM. I.e. I need to know my PSET and PPOINT.
  //At any rate, at PITEM creation time (construction), I would like a
  //pointer to it...
  void comp_pp_list( parampoint_generator& pg, std::vector<varlist<std::string>>& newlist, seedgen& sg, const bool& usedisk=false );
 
  
};
