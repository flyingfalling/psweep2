

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





struct psweep_cmd
{
  int SRC;
  std::string CMD;

psweep_cmd( const int srcr, const std::string& cm )
: SRC( srcr ), CMD( cm )
  {
    //NOTHING
  }
};



struct filesender
{

  std::shared_ptr< boost::mpi::communicator > world;
  std::shared_ptr< boost::mpi::environment > env;
  
  
  struct mem_file
  {
    std::string fname;
    std::vector<char> contents;


    mem_file()
    : fname("ERRORNOFNAME")
    {
    }
    
  mem_file( const std::string& fn, const std::vector<char> cont )
  : fname( fn ), contents (cont )
    {
      //REV: NOTHING
    }

    //Automatically reads from disk.
    mem_file( const std::string& fn )
    {
      fname = fn;
      
      
      // open the file:
      std::streampos fileSize;
      std::ifstream file(fname, std::ios::binary);
      
      // get its size:
      file.seekg(0, std::ios::end);
      fileSize = file.tellg();
      file.seekg(0, std::ios::beg);
      
      // read the data:
      //std::vector<char> fileData(fileSize);
      contents.resize(fileSize);
      file.read(contents.data(), fileSize);
    }
    
    
    void tofile(const std::string& dir, const std::string& myfname )
    {
      //REV: this does not use the FNAME of this mem_file...? Ugh.
      std::string fullfname = dir + "/" + myfname;

      std::ofstream ofs;

      open_ofstream( fullfname, ofs );
    
      ofs.write( contents.data(), contents.size() );

      //REV: does it work? Need to check sanity before I go?
    }
    
    //REV: REQUIRED for boost serialization (to send over MPI)
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
      ar & fname;
      ar & contents;
    }
  };


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


  //REV: Now I'm getting lazy..lala
  filesender( const std::shared_ptr<boost::mpi::environment>& tenv, const std::shared_ptr<boost::mpi::communicator>& tworld )
  
  {
    //gonna guess that references can't be dereferenced...
    world = tworld;
    env = tenv;
  }

  filesender()
  {
  }

  void send_varlist(  const int& targrank, const varlist<std::string>& v ) //, boost::mpi::communicator& world )
  {
    //*world.send( targrank, boost::mpi::any_tag, v );
    //world->send( targrank, boost::mpi::any_tag, v );
    world->send( targrank, 0, v );
  }


  void send_cmd( const std::string& cmd, const int& targrank ) //, boost::mpi::communicator& world )
  {
    //world->send( targrank, boost::mpi::any_tag, cmd );
    world->send( targrank, 0, cmd );
  }

  void send_pitem( const pitem& mypitem, const int& targrank ) //, boost::mpi::communicator& world )
  {
    //world->send( targrank, boost::mpi::any_tag, mypitem ); //will serialize it for me.
    world->send( targrank, 0, mypitem ); //will serialize it for me.
  }

  varlist<std::string> receive_varlist( const int& targrank ) //, boost::mpi::communicator& world  )
  {
    varlist<std::string> tmpv;
    world->recv( targrank, boost::mpi::any_tag, tmpv );
    return tmpv;
  }

  //I want to block but I want to get message from target before handling others.
  psweep_cmd receive_cmd_from_any( ) //boost::mpi::communicator& world )
  {
    boost::mpi::status msg = world->probe();
  
    std::string data;

    
    world->recv(msg.source(), boost::mpi::any_tag, data);
    
    psweep_cmd pc( msg.source(), data );

    return pc;
  }

  psweep_cmd receive_cmd_from_root( ) //boost::mpi::communicator& world )
  {
    boost::mpi::status msg = world->probe();
  
    std::string data;

    if ( msg.source() == 0 )
      {
	world->recv(msg.source(), boost::mpi::any_tag, data);
      }
    
    psweep_cmd pc( msg.source(), data );

    return pc;
  }

  pitem receive_pitem( const int& targrank ) //, boost::mpi::communicator& world  )
  {
    pitem newpitem;
    world->recv( targrank, boost::mpi::any_tag, newpitem );
    return newpitem;
  }

  int receive_int( const int& targrank ) //, boost::mpi::communicator& world )
  {
    int newint;
    world->recv( targrank, boost::mpi::any_tag, newint );
    return newint;
  }

  void send_int( const int& targrank, const int& tosend ) //, boost::mpi::communicator& world )
  {
    int newint;
    //world->send( targrank, boost::mpi::any_tag, tosend );
    world->send( targrank, 0, tosend );
  
  }

  //So, depending on the RANK, this will SEND or RECEIVE.
  //All SLAVES enter a LOOP of waiting for a MESG.

  //void rename_file( pitem& mypitem, const std::vector<mem_file>& mf, const std::vector<std::string> newfnames, const std::string& dir )
  //This will rename all files in MYPITEM (in SUCCESS only?), given a list of files? An array of files? Yea, it will tell which ones should be renamed.
  //I.e. gives list of indices to it.

 
  //compare two filenames in canonical thing? E.g. /.././../. etc. /. will remove current, i.e. can be safely removed. /.. will remove the previous
  //thing (if it starts with that /.. then error out). Can't handle things like symlinks anyway so whatever...

  //REV: BIG PROBLEM, I need to check whether it starts with a / or not.

  void handle_pitem( pitem& mypitem, const std::string& dir )
  {
    //First, get an INT, number of files.
    int numfiles = receive_int( 0 ); //get_int( 0 );
    std::vector< mem_file > mfs;
    std::vector< std::string > newfnames;
    std::vector< std::string > oldfnames;
    
    std::string fnamebase= "reqfile";
    for(size_t f=0; f<numfiles; ++f)
      {
	mem_file mf = receive_file( 0 );
	mfs.push_back( mf );
	std::string myfname = fnamebase + std::to_string( f );
	newfnames.push_back( myfname );
	oldfnames.push_back( mf.fname ); //this is old fname, I guess I could get it elsewhere... myfname is the NEW one...

	//now mfs and newfnames contain orig and new fnames.
	//REV: make something to check base of file and make sure that it
	//is same file?
      
	//write to file OK.
	mf.tofile( dir, myfname );
      
	//REV: so I now need to MODIFY mypitem for the required files,
	//and I also need to MODIFY for success, output, etc.
	//Do this all in its own function to make it easier?
      
      }

    mypitem.re_base_directory( mypitem.mydir, dir, oldfnames, newfnames);
    //mypitem will now be rebased appropriately, although hierarchical varlists etc. are not carried with it of course.

  
  
  }

  
  //void send_file( const int& targrank, const mem_file& memf )
  void send_file( const int& targrank, const mem_file& memf )
  {
    //world->send( targrank, boost::mpi::any_tag, memf );
    world->send( targrank, 0, memf );
  }
  
  void send_file_from_disk( const int& targrank, const std::string& fname )
  {
    mem_file mf( fname );
    send_file( targrank, mf );
  }
  
  
  mem_file receive_file( const int& targrank )
  {
    //REV: crap I can't just cast to a string, I need to CONSTRUCT it.
    //std::vector<char> fname = receive_memory( targrank );

    mem_file mf;
    world->recv( targrank, boost::mpi::any_tag, mf );
    return mf;
  }


  /*
  pitem get_pitem_from_targ_rank( const int& targrank )
  {
    std::vector< char > tmpmem = receive_memory( targrank );
    
    pitem tmppitem = cast_to_type< pitem >( tmpmem );

    return tmppitem;
  }
  */

  //arbitrary byte array as VECTOR, and we can cast it to a target TYPE in some way? Like...static_cast? whoa...blowin my mind haha.
  std::vector< char > receive_memory( const int& targrank )
  {

    std::vector< char > mem;
  
    MPI_Status s;
    MPI_Probe(targrank, MPI_ANY_TAG, MPI_COMM_WORLD, &s);
  
    //first, get the size.. Note, we will execute this as a SINGLE send, in future we might want to do another if there is a max MPI buffer?
    int mesgsize;
    int sourcerank = s.MPI_SOURCE;
  
    MPI_Get_count(&s, MPI_CHAR, &mesgsize);
  
    mem.resize( mesgsize );

    //REV: thsould this be MPI_INT? Or char? for 3rd arg?
    MPI_Recv(mem.data(), mesgsize, MPI_INT, 0, 0,
	     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    return mem;
  }

  /*
  template <typename T>
  T cast_to_type( std::vector< char > membuff )
  {
    //Note, I am COPYING it in this case? meh.
    T tmpitem = (T)membuff.data();
    return tmpitem;
  }
  */

  /*
  int get_int( const int& targrank )
  {
    std::vector<char> mem = receive_memory( targrank );

    int myint = cast_to_type< int >( mem ); //int POINTER?? wtf? Ugh.

    return myint;

  
    }*/

  /*
    psweep_cmd wait_for_cmd()
    {
    //Only receive mesgs from ROOT. Blocking.
    MPI_Status s;
    MPI_probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &s);
    int mesgsize;
    int sourcerank = s.MPI_SOURCE;
    MPI_Get_count(&s, MPI_CHAR, &mesgsize);
    char mesg_buf[mesgsize];
    MPI_Recv(mesg_buf, mesgsize, MPI_INT, 0, 0,
    MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    std::string mystr( mesg_buf ); //make an std string out of char array

    psweep_cmd mypcmd( sourcerank, mystr );

    return mypcmd;
    }

  */

  bool cmd_is_exit(  const psweep_cmd& pcmd )
  {
    //if( strcmp( pcmd.CMD, "EXIT") == 0 )
    if( pcmd.CMD.compare( "EXIT" ) == 0 )
      {
	return true;
      }
    else
      {
	return false;
      }
  }

  //REV: This is only for WORKERS
  pitem handle_cmd( const psweep_cmd& pcmd )
  {
    //get the CMD from it, return...
    if( pcmd.SRC != 0 )
      {
	fprintf(stderr, "ERROR worker [%d] got cmd from non-root rank...[%d]\n", world->rank(),  pcmd.SRC);
	exit(1); //EXIT more gracefully?
      }

    //if( strcmp( pcmd.CMD, "EXIT") == 0 )
    if( pcmd.CMD.compare("EXIT") == 0 )
      {
	//exit
	fprintf(stderr, "Worker [%d] received EXIT command from root rank\n", world->rank());
	exit(1);
      }
    //else if( strcmp( pcmd.CMD, "PITEM") == 0 )
    else if( pcmd.CMD.compare( "PITEM") == 0 )
      {
	//will process this pitem.
	pitem mypitem = receive_pitem( 0 ); //get_pitem_from_targ_rank( 0 );

	return mypitem;
      }
    else
      {
	fprintf(stderr, "ERROR, rank [%d] received unknown command [%s]\n", world->rank(), pcmd.CMD.c_str() );
	exit(1);
      }
  }





  void notify_finished( pitem& mypitem )
  {
    //REV: this needs to notify master, wait for it, then send back
    //any results. Master needs to wait for this too (i.e. handle a "finished"
    //state, which will require what, rebasing the files over there too?)

    send_cmd( "DONE", 0 );

    //Then send what? A "finished" struct? No, just send the "results"
    //I guess...
    //Files are: Number of SUCCESS and OUTPUT files? Note INPUT is also
    //in REQUIRED by default, so OK.
    //Note, there may be DOUBLES between OUTPUT and SUCCESS files, in fact
    //I know there are? Only send OUTPUT files then, much easier ;) Even
    //better just send a varlist...containing the output.
    //Check OUTPUT and SUCCESS separately.
    //Check also INPUT and REQUIRED separately. Make sure there are no
    //doubles...

    //Note, all OUTPUT are automatically appended to SUCCESS, so just
    //return all SUCCESS files.
    size_t nsuccess = mypitem.success_files.size();
    send_int( 0, nsuccess );

    for(size_t x=0; x<nsuccess; ++x)
      {
	send_file( 0, mypitem.success_files[x] );
      }

    varlist<std::string> resvar = mypitem.get_output();

    send_varlist( 0, resvar );


  
    return;
    
  }

  
  

  //REV: this needs to "find" which PITEM was allocated to that worker/ thread.
  varlist<std::string> handle_finished_work( const psweep_cmd& pc, pitem& corresp_pitem )
  {
    int targrank = pc.SRC;
    std::string cmd = pc.CMD;
    if( cmd.compare("DONE") != 0 )
      {
	fprintf(stderr, "WHOA, got a non-DONE cmd?!! From rank [%d]. CMD: [%s]\n", targrank, cmd.c_str() );
	exit(1);
      }
    //else, do all receiving for that CMD blocking all others for now.
    //I.e. only receive messages from others... Assume it will leave
    //(received) buffers from other guys in place while I'm doing this?
    //Or, allow it to do non-blocking, i.e. spin off threads? That
    //seems best.

    size_t nfiles = receive_int( targrank );

    //std::vector<mem_file> mfs;
    for(size_t x=0; x<nfiles; ++x)
      {
	mem_file mf = receive_file( targrank );
	//mfs.push_back( mf );
	//REV: need to figure out how to output these
	//I need to rebase them
	//Then, mf.tofile( DIR, FNAME );
	//Need to know what was the original PITEM????!!
	//Anyway, keep a PITEM around so we know MYDIR for that
	//executed worker, and just use that.

	std::string origdir = corresp_pitem.mydir;

	std::string fname="ERRORFNAME";
	std::string dirofreceived = get_canonical_dir_of_fname( mf.fname, fname );
	std::string newlocal = origdir + "/" + fname;
	//This had sure has hell better exist in original SUCCESS too haha.
	//Check for sanity.

	std::vector<bool> tmpmarked;
	std::vector<size_t> matched = find_matching_files( newlocal, corresp_pitem.success_files, tmpmarked);

	//We expect it to find AT LEAST one.
	if( matched.size() == 0 )
	  {
	    fprintf(stderr, "ERROR, in receive_files from notify of success, did not find corresponding file in original PITEM SUCCESS array (fname: [%s])\n", newlocal.c_str() );
	    exit(1);
	  }
	else if( matched.size() > 1 )
	  {
	    fprintf(stderr, "WARNING! In receive files from notify, got MORE than one match in SUCCESS array to file [%s]\n", newlocal.c_str() );
	  }

	mf.tofile( origdir, fname );
      }
  
    varlist<std::string> outputvlist = receive_varlist( targrank );
    
    return outputvlist;
  }

  bool execute_work( pitem& mypitem )
  {
    //This should do all checks (locally?) and check satisfactory output too.
    bool success = mypitem.execute_cmd();
    return success;
  }

  void cleanup_workspace( const pitem& mypitem )
  {
    //FINISHED? Destruct everything locally in TMP? I.e. remove PITEM's DIR?
    //Is there a better way to do this?

    boost::filesystem::path p( mypitem.mydir );
    uintmax_t removed = boost::filesystem::remove_all( p );

    //Another option is to move it to a tmp location...?
  
  }

  void execute_slave_loop()
  {
    bool loopslave=true;
    std::string LOCALDIR = "/tmp/scratch" + std::to_string( world->rank() ); //will execute in local scratch. Note, might need to check we have enough memory etc.
    
    fprintf(stderr, "REV: WORKER [%d] is executing slave loop. Local work dir is: [%s]\n", world->rank(), LOCALDIR.c_str() );


    //REV: need to send first guy to tell its ready
    std::string pcinit = "READY";
    send_cmd( pcinit, 0 );

    fprintf(stdout, "WORKER [%d] send CMD READY to root\n", world->rank());
    
    while( loopslave == true )
      {
	fprintf(stdout, "WORKER [%d]: waiting for cmd\n", world->rank() );
	//WAIT for mesg from MASTER
	//psweep_cmd cmd = receive_cmd_from_any();  //wait_for_cmd();
	psweep_cmd cmd = receive_cmd_from_root();  //wait_for_cmd();
	
	fprintf(stdout, "WORKER [%d]: GOT CMD [%s]. Will handle.\n", world->rank(), cmd.CMD.c_str() );
	
	if( cmd_is_exit( cmd ) == true )
	  {
	    fprintf(stderr, "REV: WORKER [%d] received EXIT\n", world->rank() );
	    loopslave = false;
	    break;
	  }

	fprintf(stdout, "HANDLING CMD\n");
	pitem mypitem = handle_cmd( cmd ); //REV: may EXIT, or contain a PITEM (to execute).
	fprintf(stdout, "WORKER [%d]: received pitem\n", world->rank());
	
	handle_pitem( mypitem, LOCALDIR ); // This will do the receiving of all files and writing to LOCALDIR, it will also rename them and keep track
	//of the correspondence. Note the SENDING side will do the error out if it is no in the __MY_DIR. Do that on PARENT side I guess. OK.

	fprintf(stdout, "WORKER [%d]: handled PITEM\n", world->rank());

	//We now have PITEM updated, we will now EXECUTE it (until it fails, keep checking success).

	execute_work( mypitem );

	fprintf(stdout, "WORKER [%d]: EXECUTED WORK\n", world->rank());

	//NEED TO SEND RESULTS
	notify_finished( mypitem ); //this includes the many pieces of "notifying, waiting for response, then sending results, then waiting, then sending files, etc..


	fprintf(stdout, "WORKER [%d]: sent finished notify\n", world->rank());
	cleanup_workspace( mypitem );
	fprintf(stdout, "WORKER [%d]: cleaned up\n", world->rank());
      }
  }



  bool is_finished_work( const psweep_cmd& pcmd )
  {
    if( pcmd.CMD.compare( "DONE" )  == 0 )
      {
	return true;
      }
    else
      {
	return false;
      }
  }

  bool is_ready_for_work( const psweep_cmd& pcmd )
  {
    if( pcmd.CMD.compare( "READY" )  == 0 )
      {
	return true;
      }
    else
      {
	return false;
      }
  }

  

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


  
  void master_to_slave( const pitem& mypitem, const size_t& workeridx )
  {
    send_cmd( "PITEM", workeridx );

    send_pitem( mypitem, workeridx ); 

    int nfiles = mypitem.required_files.size();
    
    send_int( workeridx, nfiles );
  
    //Then, send the files.
    for(size_t f=0; f<mypitem.required_files.size(); ++f)
      {
	send_file_from_disk( workeridx, mypitem.required_files[f] );
      }

    //REV: And that is it, return
    return;
  }



  
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
  
    work_progress( parampoint_generator& pg, std::vector<varlist<std::string>>& newlist, const size_t& nworkers )
    {
      farmed_status.resize( nworkers );
    
      for( size_t n=0; n<newlist.size(); ++n )
	{
	  //make the parampoint (i.e. generate the hierarchical varlists etc. locally).
	  size_t idx = pg.generate( newlist[n] );
	
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
  };

  //struct farmer
  //{
    //This does everything necessary for it. So on other side, it will do all this. This will include the MPI guys?
    //List of workers, note it is indexed from 1...i.e. 0 is worker 1...
  void comp_pp_list( parampoint_generator& pg, std::vector<varlist<std::string>>& newlist, std::vector<bool>& workingworkers )
  {

    work_progress wprog( pg, newlist, workingworkers.size() );

    //REV: these should never happen at the same time, i.e. it shouldn't
    //be done if there are any workers working...
    while( wprog.check_all_done() == false )
      {
	fprintf(stdout, "Not all done of WPROG!\n");
	//Only accept messages if there are no available workers and
	//there's no work to do.
	//If there is available workers, but no work to do, accept messages
	//If there is no available workers, but work to do, accept messages
	//if( (wprog.avail_worker() == false && wprog.check_work_avail() == true) || (wprog.avail_worker() == true && wprog.check_work_avail() == false) )
	//As long as there are not both available workers and available work...
	if( !(wprog.avail_worker( workingworkers ) == true &&
	      wprog.check_work_avail() == true )
	    )
	  {
	    fprintf(stdout, "CASE: Either there is a worker available, OR there is work available. I will receive a response from a worker\n");
	    if( wprog.avail_worker( workingworkers ) == true )
	      {
		fprintf(stdout, " NOTE: there ARE workers available\n");
	      }

	    if(  wprog.check_work_avail() == true  )
	      {
		fprintf(stdout, " NOTE: there IS work available\n");
	      }
	    //ACCEPT MESSAGES FROM WORKERS AND HANDLE.
	    //Note, we want to keep it so that this array will carry over
	    //to the next loop??? Fuck. They sent DONE or READY or
	    //whatever, we said OK to that, and marked our guy NOT WORKING.
	    //Because of that, there is no more message from the
	    //WORKER. So, I need to remember which are which, i.e.
	    //pass around a WORKING thing separate from the list of PP.
	    //OK, do it.

	    psweep_cmd pcmd = receive_cmd_from_any();

	    fprintf(stdout, "RECEIVED COMMAND (%s)\n", pcmd.CMD.c_str() );
	    if( is_finished_work( pcmd ) == true )
	      {
		
		
		//pcmd contains DONE cmd
		//I need to handle it based on corresponding worker.
		size_t workernum = pcmd.SRC;
		
		fprintf(stdout, "IT WAS A FINISHED CMD from [%ld]\n", workernum);
		
		//farmed_status[workernum] should tell us where
		parampoint_coord pc = wprog.farmed_status[workernum];
		
		pitem handledpitem = wprog.get_corresponding_pitem_from_workernum( pg, workernum );
		
		varlist<std::string> result = handle_finished_work( pcmd, handledpitem );
		
		//need to mark it DONE in pitem representation.
		wprog.mark_done( pc );

		pg.set_result( pc, result );

		workingworkers[ pcmd.SRC ] = false;
	      }
	    else if( is_ready_for_work( pcmd ) == true )
	      {
		fprintf(stdout, "NO, IT WAS JUST READY (setting worker [%d] to FALSE)\n", pcmd.SRC);
		//Contains READY cmd, just mark to not working.
		workingworkers[ pcmd.SRC ] = false;
	      }
	    else
	      {
		//should be an error
		fprintf(stderr, "In root: unrecognized cmd [%s]\n", pcmd.CMD.c_str() );
		exit(1);
		
	      }
	  }
	//both work and worker available: farm it.
	else
	  {
	    fprintf(stdout, "CASE: BOTH WORK AND A WORKER ARE AVAILABLE. WILL FARM\n");
	    size_t farmedworker = wprog.farm_work(workingworkers);

	    fprintf(stdout, "FARMED IT LOCALLY. Now will send data to slave\n");
	    parampoint_coord pc = wprog.farmed_status[ farmedworker ];
	    //FARM TO THAT WORKER, i.e. send message to that rank.
	    master_to_slave( wprog.get_corresponding_pitem( pg, pc),
			     farmedworker );
	    fprintf(stdout, "DONE master to slave.\n");
	  }
      }
    fprintf(stdout, "ALL DONE WITH WPROG!!! WOW!\n");
  }

  
};
