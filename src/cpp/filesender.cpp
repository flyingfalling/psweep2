
#include <filesender.h>



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




  //REV; TODO: at some point, build the fake MEM_FILESYSTEM, and furthermore, populate the FAKE_SYSTEM_CALLS if we want to...
  //Note when we construct and send PITEM, then we are writing to target, but we don't want to actually write out to local one unless we are executing
  //the stuff. In other words. only do it right before execute? Only if execute returns false? Execute takes the stuff. Hmm, we will be writing large numbers
  //of files possibly still, massive waste. So, I need a way to stop it from doing that...
  filesender::filesender()
  {
    MPI_Init(0, NULL);
    
    //Assume that world/env are automatically constructed?
    _workingworkers.resize( world.size(), true );
    //Wait, does this contain the info about everything e.g. -n 4??? Like ARGC and ARGV...?
    
    

    //REV: need to start true, since they're all "kind of" working (waiting for READY)
    
    
  }

filesender::filesender(fake_system& _fakesys, const bool& _todisk)
: fakesys( _fakesys ), todisk(_todisk)
  {
    MPI_Init(0, NULL);

    bool currworking=true;
    //Assume that world/env are automatically constructed?
    _workingworkers.resize( world.size(), currworking );
    //Wait, does this contain the info about everything e.g. -n 4??? Like ARGC and ARGV...?
    
    

    //REV: need to start true, since they're all "kind of" working (waiting for READY)
    
    
  }

  filesender::~filesender()
  {
    MPI_Finalize();
    //~world;
  }

  /*
  void filesender2()
  {
    //Assume that world/env are automatically constructed?

    //Wait, does this contain the info about everything e.g. -n 4??? Like ARGC and ARGV...?
    MPI_Init(0, NULL);

    //REV: need to start true, since they're all "kind of" working (waiting for READY)
    _workingworkers.resize( world.size(), true );
    
    if( world.rank() != 0 )
      {
	
	//REV: Only kind of OK because I expect that there are no other internal variables who will only be guaranteed to get their state
	//after CTOR finishes returning...
	execute_slave_loop();
	//Exits after it finishes, i.e. never returns control to
	//user...otherwise it will execute his "main program" stuff.
	//In other words, this never constructs?


	fprintf(stdout, "EXITING before destroying WORLD\n");
	//REV: FORCE finalization of WORLD... (since we never complete the constructor before exiting)
	~world;
	//Destory workingworkers? Or will it happen automatically?
	MPI_Finalize();
	//~env;

	fprintf(stdout, "EXITING after destroying WORLD\n");
	
	//REV: call destructor here? And exit?
	exit(0);
      }
    else
      {
	return;
      }
    
  }
*/
  
  /*~filesender()
  {
    if(!world.rank() == 0)
      {
	fprintf(stderr, "# # # weird ERROR in ~FILESENDER destructor, only root should actually call the destructor. Something is wrong...?\n");
      }
    std::string contents="EXIT";
    boost::mpi::broadcast(world, contents, 0);
  
    MPI_Finalize();
    
    //destruct
    }*/

  void filesender::send_varlist(  const int& targrank, const varlist<std::string>& v ) //, boost::mpi::communicator& world )
  {
    //*world.send( targrank, boost::mpi::any_tag, v );
    //world->send( targrank, boost::mpi::any_tag, v );
    world.send( targrank, 0, v );
  }


  void filesender::send_cmd( const std::string& cmd, const int& targrank ) //, boost::mpi::communicator& world )
  {
    //world->send( targrank, boost::mpi::any_tag, cmd );
    world.send( targrank, 0, cmd );
  }

  void filesender::send_pitem( const pitem& mypitem, const int& targrank ) //, boost::mpi::communicator& world )
  {
    //world->send( targrank, boost::mpi::any_tag, mypitem ); //will serialize it for me.
    world.send( targrank, 0, mypitem ); //will serialize it for me.
  }

  varlist<std::string> filesender::receive_varlist( const int& targrank ) //, boost::mpi::communicator& world  )
  {
    varlist<std::string> tmpv;
    world.recv( targrank, boost::mpi::any_tag, tmpv );
    return tmpv;
  }

  //I want to block but I want to get message from target before handling others.
  psweep_cmd filesender::receive_cmd_from_any( ) //boost::mpi::communicator& world )
  {
    boost::mpi::status msg = world.probe();
  
    std::string data;

    
    world.recv(msg.source(), boost::mpi::any_tag, data);
    
    psweep_cmd pc( msg.source(), data );

    return pc;
  }

  psweep_cmd filesender::receive_cmd_from_root( ) //boost::mpi::communicator& world )
  {
    boost::mpi::status msg = world.probe();
  
    std::string data;

    if ( msg.source() == 0 )
      {
	world.recv(msg.source(), boost::mpi::any_tag, data);
      }
    
    psweep_cmd pc( msg.source(), data );

    return pc;
  }

  pitem filesender::receive_pitem( const int& targrank ) //, boost::mpi::communicator& world  )
  {
    pitem newpitem;
    world.recv( targrank, boost::mpi::any_tag, newpitem );
    return newpitem;
  }

  int filesender::receive_int( const int& targrank ) //, boost::mpi::communicator& world )
  {
    int newint;
    world.recv( targrank, boost::mpi::any_tag, newint );
    return newint;
  }

  void filesender::send_int( const int& targrank, const int& tosend ) //, boost::mpi::communicator& world )
  {
    int newint;
    //world.send( targrank, boost::mpi::any_tag, tosend );
    world.send( targrank, 0, tosend );
  
  }

  //So, depending on the RANK, this will SEND or RECEIVE.
  //All SLAVES enter a LOOP of waiting for a MESG.

  //void rename_file( pitem& mypitem, const std::vector<mem_file>& mf, const std::vector<std::string> newfnames, const std::string& dir )
  //This will rename all files in MYPITEM (in SUCCESS only?), given a list of files? An array of files? Yea, it will tell which ones should be renamed.
  //I.e. gives list of indices to it.

 
  //compare two filenames in canonical thing? E.g. /.././../. etc. /. will remove current, i.e. can be safely removed. /.. will remove the previous
  //thing (if it starts with that /.. then error out). Can't handle things like symlinks anyway so whatever...

  //REV: BIG PROBLEM, I need to check whether it starts with a / or not.

  memfsys filesender::handle_pitem( pitem& mypitem, const std::string& dir )
  {
    //First, get an INT, number of files.
    int numfiles = receive_int( 0 ); //get_int( 0 );
    std::vector< memfile > mfs;
    std::vector< std::string > newfnames;
    std::vector< std::string > oldfnames;
    
    memfsys myfsys;
    
    std::string fnamebase= "reqfile";
    for(size_t f=0; f<numfiles; ++f)
      {
	memfile mf = receive_file( 0 );
	//mfs.push_back( mf );

#ifdef PRINTWORKER
	fprintf(stdout, "WORKER [%d]: Received file with fname [%s]\n", world.rank(), mf.filename.c_str() );
#endif
	
	//mfs.push_back( mf );
	//

	
	std::string myfname = fnamebase + std::to_string( f );
	newfnames.push_back( myfname );
	oldfnames.push_back( mf.filename ); //this is old fname, I guess I could get it elsewhere... myfname is the NEW one...
	
	//now mfs and newfnames contain orig and new fnames.
	//REV: make something to check base of file and make sure that it
	//is same file?

	//REV: It fucking renamed the INPUT file to reqfile0. So, I need to make sure to keep it named the same?
	//I.e. I rename something? Fuck...how does user know what to do? If it is named something different. That makes it difficult to know which
	//is which for the user haha... just have user reference certain file names specifically if he needs them. Yea...that works I guess. OK.
	//So, now what he does, is he searches for that variable's value *now*???? Not by filename but by variable? Ugh, that's so ugly. I.e.
	//print out INPUT FILE now... I.e. I need to re-work the command to replace any instances of OLD inputfile with NEW inputfile...meh.
#ifdef PRINTWORKER
	fprintf(stdout, "IN WORKER [%d]: GOT LOCAL MEM FILE: [%s]\n", world.rank(), std::string(dir+"/"+myfname).c_str());
#endif
	mf.filename = dir + "/" + myfname;
	
	if( todisk == true)
	  {
#ifdef PRINTWORKER
	    fprintf(stdout, "IN WORKER [%d]: PRINTING TO DISK LOCAL MEM FILE: [%s]\n", world.rank(), std::string(dir+"/"+myfname).c_str());
#endif
	    mf.tofile( dir + "/" + myfname );
	  }


	myfsys.add_file( mf );
	//write to file OK.
	//REV: Add to filesystem and/or push back...heh
	
	
	
	//memfile_ptr mfp( mf );
	//mfp.tofile( dir + "/" + myfname );
      
	//REV: so I now need to MODIFY mypitem for the required files,
	//and I also need to MODIFY for success, output, etc.
	//Do this all in its own function to make it easier?
      
      }
    
    mypitem.re_base_directory( mypitem.mydir, dir, oldfnames, newfnames);
    //REV: I will now create memfsys using that array as the thing...I will write them out if necessary.
        
    mypitem.mydir = dir;
    //mypitem will now be rebased appropriately, although hierarchical varlists etc. are not carried with it of course.
    

    return myfsys;
  }

  
  //void send_file( const int& targrank, const mem_file& memf )
  void filesender::send_file( const int& targrank, const memfile& memf )
  {
    //world.send( targrank, boost::mpi::any_tag, memf );
    world.send( targrank, 0, memf );
  }
  
  void filesender::send_file_from_disk( const int& targrank, const std::string& fname )
  {
    bool readfromdisk=true;
    memfile mf( fname, readfromdisk );
    send_file( targrank, mf );
  }
  
  
  memfile filesender::receive_file( const int& targrank )
  {
    //REV: crap I can't just cast to a string, I need to CONSTRUCT it.
    //std::vector<char> fname = receive_memory( targrank );

    memfile mf;
    world.recv( targrank, boost::mpi::any_tag, mf );
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
  std::vector< char > filesender::receive_memory( const int& targrank )
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

  bool filesender::cmd_is_exit(  const psweep_cmd& pcmd )
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
  pitem filesender::handle_cmd( const psweep_cmd& pcmd )
  {
    //get the CMD from it, return...
    if( pcmd.SRC != 0 )
      {
	fprintf(stderr, "ERROR worker [%d] got cmd from non-root rank...[%d]\n", world.rank(),  pcmd.SRC);
	exit(1); //EXIT more gracefully?
      }

    //if( strcmp( pcmd.CMD, "EXIT") == 0 )
    if( pcmd.CMD.compare("EXIT") == 0 )
      {
	//exit
	fprintf(stderr, "Worker [%d] received EXIT command from root rank\n", world.rank());
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
	fprintf(stderr, "ERROR, rank [%d] received unknown command [%s]\n", world.rank(), pcmd.CMD.c_str() );
	exit(1);
      }
  }





  void filesender::notify_finished( pitem& mypitem, memfsys& myfsys )
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
	std::string mfname = mypitem.success_files[x];

	memfile_ptr mfp = myfsys.open( mfname, todisk );
	//REV: This is where a problem could happen...
	//Send from disk if todisk is true...
	send_file( 0, mfp.get_memfile() );
	mfp.close();
      }
    
    varlist<std::string> resvar = mypitem.get_output( myfsys, todisk );
    
    send_varlist( 0, resvar );
    
    return;
    
  }

  
  

  //REV: this needs to "find" which PITEM was allocated to that worker/ thread.
  //REV: Note we could use todisk, but easier to do it as todisk?
  varlist<std::string> filesender::handle_finished_work( const psweep_cmd& pc, pitem& corresp_pitem, memfsys& myfsys, const bool& usedisk )
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

    //fprintf(stdout, "MASTER: receiving INT\n");
    size_t nfiles = receive_int( targrank );
    //fprintf(stdout, "MASTER: Received int [%ld]\n", nfiles);

    //std::vector<mem_file> mfs;
    for(size_t x=0; x<nfiles; ++x)
      {
	//fprintf(stdout, "MASTER: receiving file from WORKER [%d]\n", targrank);
	memfile mf = receive_file( targrank );
	myfsys.add_file( mf );
	
	//fprintf(stdout, "MASTER Received file from WORKER [%d]\n", targrank);
	//mfs.push_back( mf );
	//REV: need to figure out how to output these
	//I need to rebase them
	//Then, mf.tofile( DIR, FNAME );
	//Need to know what was the original PITEM????!!
	//Anyway, keep a PITEM around so we know MYDIR for that
	//executed worker, and just use that.
	
	std::string origdir = corresp_pitem.mydir;
	
	//fprintf(stdout, "MASTER: will rename file to origdir: [%s]\n", origdir.c_str() );
	
	std::string fname="ERRORFNAME";
	std::string dirofreceived = get_canonical_dir_of_fname( mf.filename, fname );
	std::string newlocal = origdir + "/" + fname;
	//This had sure has hell better exist in original SUCCESS too haha.
	//Check for sanity.

	//fprintf(stdout, "MASTER: searching for local copies of fname we would expect: [%s]\n", newlocal.c_str() );
	
	std::vector<bool> tmpmarked( corresp_pitem.success_files.size(), false);

	/*fprintf(stderr, "Will compare [%s] to following array:\n", newlocal.c_str());
	for(size_t z=0; z<corresp_pitem.success_files.size(); ++z)
	  {
	    fprintf(stderr, "[%s]\n", corresp_pitem.success_files[z].c_str() );
	    }*/
	
	std::vector<size_t> matched = find_matching_files( newlocal, corresp_pitem.success_files, tmpmarked );

	//fprintf(stdout, "MASTER: Renaming received file to [%s]\n", newlocal.c_str());
	
	//We expect it to find AT LEAST one.
	if( matched.size() == 0 )
	  {
	    fprintf(stderr, "ERROR, in receive_files from notify of success, did not find corresponding file in original PITEM SUCCESS array (fname: [%s])\n", newlocal.c_str() );
	    fprintf(stderr, "NOTE: corresp files:\n");
	    for(size_t z=0; z<corresp_pitem.success_files.size(); ++z)
	      {
		fprintf(stderr, "[%s]\n", corresp_pitem.success_files[z].c_str() );
	      }
	    exit(1);
	  }
	else if( matched.size() > 1 )
	  {
	    fprintf(stderr, "WARNING! In receive files from notify, got MORE than one match in SUCCESS array to file [%s]\n", newlocal.c_str() );
	  }

	//fprintf(stdout, "MASTER: Will attempt to write file to filename [%s]\n", newlocal.c_str());
	
	
	//mf.tofile( origdir, fname );
	
	//REV: ONLY OUTPUT ON MASTER SIDE IF WE NEED?!?!?!
	//Whatever, output it for safety...

	//REV: This is using local TODISK... I need to make it regular...
	//if( todisk )
	if( usedisk )
	  {
	    memfile_ptr mfp(mf);
	    mfp.tofile( origdir+"/"+fname );
	    mfp.close();
	  }
	//fprintf(stdout, "MASTER: wrote file\n");
      }

    //fprintf(stdout, "MASTER: RECEIVING VARLIST from worker [%d].\n", targrank);
    varlist<std::string> outputvlist = receive_varlist( targrank );
    //fprintf(stdout, "MASTER: **DONE rec VARLIST from worker [%d].\n", targrank);
    
    return outputvlist;
  }

  bool filesender::execute_work( pitem& mypitem, memfsys& myfsys )
  {
    //This should do all checks (locally?) and check satisfactory output too.
    
    //fprintf(stdout, "WORKER [%d] : Attempting to execute work...\n", world.rank());
    bool success = mypitem.execute_cmd( fakesys, myfsys );
    //fprintf(stdout, "WORKER [%d] : Finished execute work...?\n", world.rank());
    return success;
  }

  void filesender::cleanup_workspace( const pitem& mypitem )
  {
    //FINISHED? Destruct everything locally in TMP? I.e. remove PITEM's DIR?
    //Is there a better way to do this?
    //fprintf(stdout, "Worker [%d] removing recursively workspace [%s]\n", world.rank(), mypitem.mydir.c_str());
    boost::filesystem::path p( mypitem.mydir );
    uintmax_t removed = boost::filesystem::remove_all( p );

    //Another option is to move it to a tmp location...?
  
  }

void filesender::execute_slave_loop( const std::string& runtag)
  {
    bool loopslave=true;
    std::string LOCALDIR = "/tmp/" + runtag + "_" + std::to_string( world.rank() ); //will execute in local scratch. Note, might need to check we have enough memory etc.

    //MKDIR HERE?
    
    //fprintf(stderr, "REV: WORKER [%d] is executing slave loop. Local work dir is: [%s]\n", world.rank(), LOCALDIR.c_str() );


    //REV: need to send first guy to tell its ready
    std::string pcinit = "READY";
    send_cmd( pcinit, 0 );

    //fprintf(stdout, "WORKER [%d] send CMD READY to root\n", world.rank());
    
    while( loopslave == true )
      {
	make_directory( LOCALDIR );
	
	//fprintf(stdout, "WORKER [%d]: waiting for cmd\n", world.rank() );
	//WAIT for mesg from MASTER
	//psweep_cmd cmd = receive_cmd_from_any();  //wait_for_cmd();
	psweep_cmd cmd = receive_cmd_from_root();  //wait_for_cmd();
	
	//fprintf(stdout, "WORKER [%d]: GOT CMD [%s]. Will handle.\n", world.rank(), cmd.CMD.c_str() );
	
	if( cmd_is_exit( cmd ) == true )
	  {
	    fprintf(stderr, "REV: WORKER [%d] received EXIT\n", world.rank() );
	    loopslave = false;
	    break;
	  }

	//fprintf(stdout, "HANDLING CMD\n");
	pitem mypitem = handle_cmd( cmd ); //REV: may EXIT, or contain a PITEM (to execute).
	//fprintf(stdout, "WORKER [%d]: received pitem\n", world.rank());
	
	memfsys myfsys = handle_pitem( mypitem, LOCALDIR ); // This will do the receiving of all files and writing to LOCALDIR, it will also rename them and keep track
	//of the correspondence. Note the SENDING side will do the error out if it is no in the __MY_DIR. Do that on PARENT side I guess. OK.
	
	//REV: BUILD FAKE FYSYS HERE! (for example, we might not want to have written to disk, but rather received to the fake FS)

	
	//myfsys.enumerate();
	
	
	//	fprintf(stdout, "WORKER [%d]: handled PITEM\n", world.rank());
	
	//We now have PITEM updated, we will now EXECUTE it (until it fails, keep checking success).
	
	
	bool blah = execute_work( mypitem, myfsys );

	//fprintf(stdout, "WORKER [%d]: EXECUTED WORK\n", world.rank());

	//myfsys.enumerate();
	//NEED TO SEND RESULTS
	notify_finished( mypitem, myfsys ); //this includes the many pieces of "notifying, waiting for response, then sending results, then waiting, then sending files, etc..

	
	//fprintf(stdout, "WORKER [%d]: sent finished notify\n", world.rank());
	cleanup_workspace( mypitem );
	//fprintf(stdout, "WORKER [%d]: cleaned up\n", world.rank());
      }

    //fprintf(stdout, "WORKER [%d]: SLAVE LOOP: RETURNING from slave loop\n", world.rank());
  }



  bool filesender::is_finished_work( const psweep_cmd& pcmd )
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

  bool filesender::is_ready_for_work( const psweep_cmd& pcmd )
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

  


  //REV: Always read from files here...assume there will be none shared.
  void filesender::master_to_slave( const pitem& mypitem, const size_t& workeridx, memfsys& myfsys )
  {
    send_cmd( "PITEM", workeridx );

    send_pitem( mypitem, workeridx ); 

    int nfiles = mypitem.required_files.size();
    
    send_int( workeridx, nfiles );

    //REV: Read all required files into memory (?). Note, they may already be there haha... Note, if it already exists, just leave as is.
    
    
    
    //Then, send the files.
    for(size_t f=0; f<mypitem.required_files.size(); ++f)
      {
	//REV: Yea...just send files from here. Problem is, some of them might not be in MEMFSYS??? We should load them all at some point (required,
	//and input)
	memfile_ptr mfp = myfsys.open( mypitem.required_files[f], true ); //will attempt to read through if it does not exist. Note may be empty?
	
	//send_file_from_disk( workeridx, mypitem.required_files[f] );
	send_file( workeridx, mfp.get_memfile() );
	mfp.close();
      }

    //REV: And that is it, return
    return;
  }



  

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
void filesender::comp_pp_list( parampoint_generator& pg, std::vector<varlist<std::string>>& newlist, seedgen& sg, const bool& usedisk )
  {
    
    work_progress wprog( pg, newlist, _workingworkers.size(), sg, usedisk );

    //REV: these should never happen at the same time, i.e. it shouldn't
    //be done if there are any workers working...
    while( wprog.check_all_done() == false )
      {
	//fprintf(stdout, "Not all done of WPROG!\n");
	//Only accept messages if there are no available workers and
	//there's no work to do.
	//If there is available workers, but no work to do, accept messages
	//If there is no available workers, but work to do, accept messages
	//if( (wprog.avail_worker() == false && wprog.check_work_avail() == true) || (wprog.avail_worker() == true && wprog.check_work_avail() == false) )
	//As long as there are not both available workers and available work...
	if( !(wprog.avail_worker( _workingworkers ) == true &&
	      wprog.check_work_avail() == true )
	    )
	  {
	    //fprintf(stdout, "CASE: Either there is a worker available, OR there is work available. I will receive a response from a worker\n");
	    if( wprog.avail_worker( _workingworkers ) == true )
	      {
		//fprintf(stdout, " NOTE: there ARE workers available\n");
	      }

	    if(  wprog.check_work_avail() == true  )
	      {
		//fprintf(stdout, " NOTE: there IS work available\n");
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

	    //fprintf(stdout, "RECEIVED COMMAND (%s)\n", pcmd.CMD.c_str() );
	    if( is_finished_work( pcmd ) == true )
	      {
		
		
		//pcmd contains DONE cmd
		//I need to handle it based on corresponding worker.
		size_t workernum = pcmd.SRC;


		//REV: FIX THIS HERE FEB 2
		//fprintf(stdout, "IT WAS A FINISHED CMD from [%ld]\n", workernum);
		
		//farmed_status[workernum] should tell us where
		parampoint_coord pc = wprog.farmed_status[workernum];
		
		//fprintf(stdout, "&&& Recevied finished cmd from worker [%ld]. The received was coordinate: PPOINT [%ld], PSET [%ld], PITEM [%ld]\n", workernum,  pc.parampointn, pc.psetn, pc.pitemn );
		
		pitem handledpitem = wprog.get_corresponding_pitem_from_workernum( pg, workernum );
		//fprintf(stdout, "Got handled pitem from that coordinate\n");


		//Specifically, call it on the parampoint#, from pg.parampoint_memfsystems[ pc.parampointn ].
		//REV: THIS will write to files! Modify to use a local (temporary) memfsys
		varlist<std::string> result = handle_finished_work( pcmd, handledpitem, pg.parampoint_memfsystems[ pc.parampointn ], usedisk );
		
		//fprintf(stdout, "MASTER: got result from worker [%ld]. Now marking done...\n", workernum);
		//need to mark it DONE in pitem representation.
		wprog.mark_done( pc );
		
		//fprintf(stdout, "MASTER: Marked done! Will now set result in PPOINT list\n");
		pg.set_result( pc, result );
		
		
		//fprintf(stdout, "MASTER: Finished set done. Now setting workingworkers targ to false\n");
		_workingworkers[ pcmd.SRC ] = false;
		//fprintf(stdout, "MASTER: ALl done handling received DONE\n");
	      }
	    else if( is_ready_for_work( pcmd ) == true )
	      {
		//fprintf(stdout, "NO, IT WAS JUST READY (setting worker [%d] to FALSE)\n", pcmd.SRC);
		//Contains READY cmd, just mark to not working.
		_workingworkers[ pcmd.SRC ] = false;
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
	    //fprintf(stdout, "CASE: BOTH WORK AND A WORKER ARE AVAILABLE. WILL FARM\n");

	    //REV: This will only modify local worker-tabulating structs,
	    //it will not do any actual work or communicate over MPI.
	    size_t farmedworker = wprog.farm_work(_workingworkers);

	    //fprintf(stdout, "FARMED IT LOCALLY. Now will send data to slave\n");
	    parampoint_coord pc = wprog.farmed_status[ farmedworker ];

	    //FARM TO THAT WORKER, i.e. send message to that rank.
	    //REV: This actually does the reading from files and sending. Modify to use (local) memfsys. Uses SEND_FILE_FROM_DISK.

	    //Finally, at PITEM construction time, it may have written
	    //INPUT. I really should create the (local) pitem memfile
	    //system at that point...mmm I kind of like that.
	    //OK, final thing to do is to make sure that I delete
	    //memfsys after theyre done (perhaps logging them).
	    //Or else they may grow VERY large...
	    //For example, after a whole PP is finished, how do I know what to do with it?
	    master_to_slave( wprog.get_corresponding_pitem( pg, pc),
			     farmedworker,
			     pg.parampoint_memfsystems[pc.parampointn] );
	    //fprintf(stdout, "DONE master to slave.\n");
	  }
      }
    //fprintf(stdout, "$ $ $ $ $ $ YOLOLOLOLO ALL DONE WITH WPROG!!! WOW!\n");
  }
