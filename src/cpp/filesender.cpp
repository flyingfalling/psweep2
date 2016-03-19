
#include <filesender.h>




psweep_cmd::psweep_cmd( const int& srcworker, const std::string& cm )
  : SRC( srcworker ), CMD( cm )
{
  //NOTHING
}


void filesender::start_worker_loop(const std::string& runtag)
{
  //REV: In here, I will mess around with threads!
  //Each filesender instance will only have a single dev#, etc.
  //but multiple threads of course. First thread is one of them.
  //std::thread mythread();
  std::vector< std::thread > thrs;
  if(workersperrank > 1)
    {
      thrs.resize( workersperrank-1 );
      for(size_t tag=0; tag<thrs.size(); ++tag)
      {
	//REV: This is issue.
	//If I call method, how does it know which "base" class to use? Copy of whole "this" filesender?
	//Need to use this as second, due to the execute thing.
	thrs[tag] = std::thread( &filesender::execute_slave_loop, this, tag+1, runtag );
      }
    }
  //std::thread thr( execute_slave_loop(runtag, 0) );
  execute_slave_loop(0, runtag);
  
  for(size_t tag=0; tag<thrs.size(); ++tag)
    {
      thrs[tag].join();
    }
}

//REV: This will get processor name (only for the root). All those with same number as ROOT remember that they need to add 1.
//This way I only compute it once, a bit better this way... I could have user-side store a GPU associate counter for speed though ;)
void filesender::init_local_worker_idx()
{
  //This is stored locally in each rank of course
  //So, I really only need to compute it once.
  //local_worker_idx.resize( world.size(), 0 );
  std::vector<char> name( MPI_MAX_PROCESSOR_NAME );
  int namelen=-1;
  MPI_Get_processor_name(name.data(), &namelen);
  std::string myname(name.begin(), name.begin()+namelen); //will this work?
  //Now, if my rank is 0, send, else, receive.
  std::string retval=std::getenv( "OMPI_COMM_WORLD_LOCAL_RANK" );
  mylocalidx = std::stol(retval);
  std::string cmdname = "ROOTNAME";
  
  if( world.rank() == 0 )
    {
      fprintf(stdout, "ROOT host is [%s], root local idx is [%s]([%ld])\n", myname.c_str(), retval.c_str(), mylocalidx);

      //Rank zero had sure as heck better be the 0th in its guy.
      //I guess it really doesn't matter (?) but that means I need
      //to figure out what to do. If I send my number as well, other
      //guys can compute how many they need to go "down", but
      //that's a PITA.
      if( mylocalidx != 0 )
	{
	  fprintf(stderr, "REV: WHOA, rank 0 is not index 0!!!\n  It is rank(string) [%s] (translated to [%ld]\n\n", retval.c_str(), mylocalidx);
	  exit(1);
		  
	  //Error?
	}
      //send to all, my name
      //REV* OOP, everyone is supposed to call this haha! Apparently it works with recv lol....
      broadcast_cmd(cmdname);
      broadcast_cmd(myname);
    }
  else
    {
      psweep_cmd c = receive_cmd_from_root();
      if( c.CMD.compare( cmdname ) != 0 )
	{
	  fprintf(stderr, "REV: MAJOR ERROR, worker [%d] recieved cmd from root, expecting [%s] but got [%s]\n", world.rank(), cmdname.c_str(), c.CMD.c_str() );
	  exit(1);
	}

      psweep_cmd c2 = receive_cmd_from_root();
      

      if( myname.compare( c2.CMD ) == 0 )
	{
	  //#if DEBUG>5
	  fprintf(stdout, "RANK [%d] hostname is [%s], my local idx is [%s]([%ld]). From root I received [%s] (I MATCHED)\n", world.rank(), myname.c_str(), retval.c_str(), mylocalidx, c2.CMD.c_str());
	  //#endif
	  //I am in same as root rank, need to subtract 1!
	  if( mylocalidx > 0 )
	    {
	      mylocalidx -= 1;
	    }
	  else
	    {
	      fprintf(stderr, "ERROR: RANK [%d]: Attempting to subtract 1 from my local index, but my local index is already ZERO. Wat?\n", world.rank());
	    }
	}
      else
	{
	  //#if DEBUG>5
	  fprintf(stdout, "RANK [%d] hostname is [%s], my local idx is [%s]([%ld]). From root I received [%s] (I *DID NOT* MATCH)\n", world.rank(), myname.c_str(), retval.c_str(), mylocalidx, c2.CMD.c_str());
	  //#endif
	}

      //receive it and check if its same. If so, subtract 1 from retval
      //
    }
  fprintf(stdout, "FINISHED init local worker idx!!!\n");
  
} //init_local_worker_idx done.


//REV; TODO: at some point, build the fake MEM_FILESYSTEM, and furthermore, populate the FAKE_SYSTEM_CALLS if we want to...
//Note when we construct and send PITEM, then we are writing to target, but we don't want to actually write out to local one unless we are executing
//the stuff. In other words. only do it right before execute? Only if execute returns false? Execute takes the stuff. Hmm, we will be writing large numbers
//of files possibly still, massive waste. So, I need a way to stop it from doing that...
size_t filesender::getworker( const size_t& rank, const size_t& tag )
{
  if(rank==0)
    {
      return 0;
    }
  //integer division...+1 for root rank
  size_t myworker = ((rank-1) * workersperrank) + tag + 1;
  return myworker;
}

size_t filesender::getworkerrank( const size_t& wnum )
{
  if(wnum == 0)
    {
      return 0;
    }
  //integer division...+1 for root rank
  size_t myr = ((wnum-1) / workersperrank) + 1;
  return myr;
}

size_t filesender::getworkertag( const size_t& wnum )
{
  if( wnum == 0 )
    { return 0; }
  //integer division...+1 for root rank
  size_t mytag = ((wnum-1) % workersperrank);
  return mytag;
}

void filesender::initfilesender()
{
  //MPI_init_thread(argc, argv, int required, int* provited)
  MPI_Init(0, NULL);
  
  myrank = world.rank();
  
  //Assume that world/env are automatically constructed?
  init_local_worker_idx();
  
  //I actually don't need to do this
  //#ifdef CUDA_SUPPORT
  mygpuidx=compute_gpu_idx( mylocalidx, workersperrank, getrank() );
  //#endif
  
  bool currworking=true;
  //Rank 1 has only 1 worker (the root master), all others have workersperrank.
  size_t nworkers = 1 + (world.size()-1)*workersperrank;
  _workingworkers.resize( nworkers, true );

  fprintf(stdout, "Rank [%ld] finished initialize file sender, will now spawn threads...?\n", getrank());
}

filesender::filesender()
{
  initfilesender();
}

filesender::filesender(fake_system& _fakesys, const size_t& _wrkperrank, const bool& _todisk )
  : fakesys( _fakesys ), todisk(_todisk), workersperrank( _wrkperrank )
{
  initfilesender();
}

filesender::~filesender()
{
  MPI_Finalize();
  //~world;
}

bool filesender::checkroot()
{
  if(getrank() == 0 )
    {
      return true;
    }
  return false;
}

void filesender::lmux()
{
  mpimux.lock();
}

void filesender::ulmux()
{
  mpimux.unlock();
}

int filesender::getrank()
{
  /*lmux();
  return world.rank();
  ulmux;*/
  return myrank;
}

void filesender::send_varlist_to_worker(  const varlist<std::string>& v, const int& targworker)
{
  if( checkroot() == false )
    {
      fprintf(stderr, "ERROR: REV: send varlist to worker, should only be called from ROOT rank 0, but calling from [%d]\n", getrank());
      exit(1);
    }
  lmux();
  world.send( getworkerrank(targworker), getworkertag(targworker), v );
  ulmux();
}

void filesender::send_varlist_to_root( const varlist<std::string>& v, const int& mytag )
{
  if( checkroot() == true )
    {
      fprintf(stderr, "ERROR: REV: send varlist to root, should only be called from non-ROOT rank >0, but calling from [%d]\n", getrank());
      exit(1);
    }
  lmux();
  world.send( 0, mytag, v ); //Rank will be implicitly known by root based on SRC
  ulmux();
}

void filesender::broadcast_cmd( const std::string& cmd ) 
{
  //world.broadcast( MPI_COMM_WORLD, cmd, world.rank() );
  std::string c=cmd;
  boost::mpi::broadcast( world, c, world.rank() );
  return;
}

void filesender::send_cmd_to_worker( const std::string& cmd, const int& targworker )
{
  if( checkroot() == false )
    {
      fprintf(stderr, "ERROR: REV: send cmd to worker, should only be called from ROOT rank 0, but calling from [%d]\n", getrank());
      exit(1);
    }
  lmux();
  world.send( getworkerrank(targworker), getworkertag(targworker), cmd );
  ulmux();
}

void filesender::send_cmd_to_root( const std::string& cmd, const int& mytag )
{
  if( checkroot() == true )
    {
      fprintf(stderr, "ERROR: REV: send varlist to root, should only be called from non-ROOT rank >0, but calling from [%d]\n", getrank());
      exit(1);
    }

  //REV: KNOW MY PROBLEM, Im using the same mux for send and recv. So I lock it and wait for recv, but other guys cant send! So need to probe/tag etc.
  fprintf(stdout, "WORKER [%ld]: ATTEMPTING TO LOCK MUX for send CMD to ROOT\n", getworker( getrank(), mytag) );
  lmux();
  fprintf(stdout, "WORKER [%ld]: SUCCEEDED TO LOCK MUX for send CMD to ROOT\n", getworker( getrank(), mytag) );
  world.send( 0, mytag, cmd );
  fprintf(stdout, "WORKER [%ld]: FINISHED send CMD to ROOT, will unlock mux\n", getworker( getrank(), mytag) );
  ulmux();
}


void filesender::send_pitem_to_worker( const pitem& mypitem, const int& targworker )
{
  if( checkroot() == false )
    {
      fprintf(stderr, "ERROR: REV: send pitem to worker, should only be called from ROOT rank 0, but calling from [%d]\n", getrank());
      exit(1);
    }
  lmux();
  world.send( getworkerrank(targworker), getworkertag(targworker), mypitem ); //will serialize it for me.
  ulmux();
}

void filesender::send_pitem_to_root( const pitem& mypitem, const int& mytag )
{
  if( checkroot() == true )
    {
      fprintf(stderr, "ERROR: REV: send pitem to root, should only be called from ROOT rank 0, but calling from [%d]\n", getrank());
      exit(1);
    }
  lmux();
  world.send( 0, mytag, mypitem );
  ulmux();
}

varlist<std::string> filesender::receive_varlist_from_worker( const int& targworker )
{
  if( checkroot() == false )
    {
      fprintf(stderr, "ERROR: REV: recv varlist worker, should only be called from ROOT rank 0, but calling from [%d]\n", getrank());
      exit(1);
    }
  varlist<std::string> tmpv;
  lmux();
  world.recv( getworkerrank(targworker), getworkertag( targworker ), tmpv );
  ulmux();
  return tmpv;
}

varlist<std::string> filesender::receive_varlist_from_root( const int& targworker, const int& mytag )
{
  if( checkroot() == true )
    {
      fprintf(stderr, "ERROR: REV: recv varlist root, should only be called from non ROOT rank >0, but calling from [%d]\n", getrank());
      exit(1);
    }
  
  varlist<std::string> tmpv;
  lmux();
  world.recv( 0, mytag, tmpv );
  ulmux();
  return tmpv;
}

psweep_cmd filesender::receive_cmd_from_any_worker( )
{
  if( checkroot() == false )
    {
      fprintf(stderr, "ERROR: REV: recv cmd from any worker, should only be called from ROOT rank 0, but calling from [%d]\n", getrank());
      exit(1);
    }
  std::string data;
  
  lmux();
  boost::mpi::status msg = world.probe();
  //world.recv(msg.source(), boost::mpi::any_tag, data);
  world.recv(msg.source(), msg.tag(), data);
  ulmux();
  
  psweep_cmd pc( getworker( msg.source(), msg.tag() ),  data );
  return pc;
}

psweep_cmd filesender::receive_cmd_from_root( )
{
  if( checkroot() == true )
    {
      fprintf(stderr, "ERROR: REV: recv cmd from ROOT (noarg), should only be called from non-ROOT rank >0, but calling from [%d]\n", getrank());
      exit(1);
    }
  
  std::string data;
  //I literally block until there is a mesg from root?
  lmux();
  world.recv(0, boost::mpi::any_tag, data);
  ulmux();

  psweep_cmd pc( 0, data );

  return pc;
}

psweep_cmd filesender::receive_cmd_from_root( const int& mytag )
{
  if( checkroot() == true )
    {
      fprintf(stderr, "ERROR: REV: recv cmd from ROOT, should only be called from non-ROOT rank >0, but calling from [%d]\n", getrank());
      exit(1);
    }
  
  std::string data;
  //I literally block until there is a mesg from root?
  lmux();
  world.recv(0, mytag, data);
  ulmux();

  /*
  boost::mpi::status msg = world.probe();
  
  

  if ( msg.source() == 0 )
    {
      
    }
  else
    {
      fprintf(stderr, "ERROR, rank [%d] got a non-root message (from [%d]) even though I'm requesting receive_cmd_from_root\n", world.rank(), msg.source() );
      exit(1);
    }
  */
  psweep_cmd pc( 0, data );

  return pc;
}

pitem filesender::receive_pitem_from_root( const int& mytag ) 
{
  if( checkroot() == true )
    {
      fprintf(stderr, "ERROR: REV: recv pitem from ROOT, should only be called from non-ROOT rank >0, but calling from [%d]\n", getrank());
      exit(1);
    }
  pitem newpitem;
  lmux();
  world.recv( 0, mytag, newpitem );
  ulmux();
  return newpitem;
}

pitem filesender::receive_pitem_from_worker( const int& targworker ) 
{
  if( checkroot() == false )
    {
      fprintf(stderr, "ERROR: REV: recv pitem from worker, should only be called from ROOT rank 0, but calling from [%d]\n", getrank());
      exit(1);
    }
  pitem newpitem;
  lmux();
  world.recv( getworkerrank(targworker), getworkertag(targworker), newpitem );
  ulmux();
  return newpitem;
}

int filesender::receive_int_from_root( const int& mytag )
{
  if( checkroot() == true )
    {
      fprintf(stderr, "ERROR: REV: recv int from ROOT, should only be called from non-ROOT rank >0, but calling from [%d]\n", getrank());
      exit(1);
    }
  int newint;
  lmux();
  world.recv( 0, mytag, newint );
  ulmux();
  return newint;
}

int filesender::receive_int_from_worker( const int& targworker ) 
{
  if( checkroot() == false )
    {
      fprintf(stderr, "ERROR: REV: recv int from worker, should only be called from ROOT rank 0, but calling from [%d]\n", getrank());
      exit(1);
    }
  int newint;
  lmux();
  world.recv( getworkerrank(targworker), getworkertag(targworker), newint );
  ulmux();
  return newint;
}


void filesender::send_int_to_root( const int& tosend, const int& mytag ) 
{
  if( checkroot() == true )
    {
      fprintf(stderr, "ERROR: REV: send int to ROOT, should only be called from non-ROOT rank >0, but calling from [%d]\n", getrank());
      exit(1);
    }
  lmux();
  world.send( 0, mytag, tosend );
  ulmux();
}

void filesender::send_int_to_worker( const int& tosend, const int& targworker )
{
  if( checkroot() == false )
    {
      fprintf(stderr, "ERROR: REV: send int to worker, should only be called from ROOT rank 0, but calling from [%d]\n", getrank());
      exit(1);
    }
  lmux();
  world.send( getworkerrank(targworker), getworkertag(targworker), tosend );
  ulmux();
}


void filesender::send_file_to_worker( const memfile& memf, const int& targworker)
{
  if( checkroot() == false )
    {
      fprintf(stderr, "ERROR: REV: send file to worker, should only be called from ROOT rank 0, but calling from [%d]\n", getrank());
      exit(1);
    }
  lmux();
  world.send( getworkerrank(targworker), getworkertag(targworker), memf );
  ulmux();
}

void filesender::send_file_to_root( const memfile& memf, const int& mytag )
{
  if( checkroot() == true )
    {
      fprintf(stderr, "ERROR: REV: send file to ROOT, should only be called from non-ROOT rank >0, but calling from [%d]\n", getrank());
      exit(1);
    }
  lmux();
  world.send( 0, mytag, memf );
  ulmux();
}
  
void filesender::send_file_to_root_from_disk( const std::string& fname, const int& mytag )
{
  bool readfromdisk=true;
  memfile mf( fname, readfromdisk );
  send_file_to_root( mf, mytag );
}

void filesender::send_file_to_worker_from_disk( const std::string& fname, const int& targworker )
{
  bool readfromdisk=true;
  memfile mf( fname, readfromdisk );
  send_file_to_worker( mf, targworker );
}

memfile filesender::receive_file_from_worker( const int& targworker )
{
  if( checkroot() == false )
    {
      fprintf(stderr, "ERROR: REV: recv file from worker, should only be called from ROOT rank 0, but calling from [%d]\n", getrank());
      exit(1);
    }
  memfile mf;
  lmux();
  world.recv( getworkerrank(targworker), getworkertag(targworker), mf );
  ulmux();
  return mf;
}

memfile filesender::receive_file_from_root( const int& mytag )
{
  if( checkroot() == true )
    {
      fprintf(stderr, "ERROR: REV: recv file from ROOT, should only be called from non-ROOT rank >0, but calling from [%d]\n", getrank());
      exit(1);
    }
  memfile mf;
  lmux();
  world.recv( 0, mytag, mf );
  ulmux();
  return mf;
}


//Handle pitem is called by WORKER
memfsys filesender::worker_handle_pitem( pitem& mypitem, const std::string& dir, const int& mytag )
{
  //First, get an INT, number of files.
  int numfiles = receive_int_from_root( mytag ); //get_int( 0 );
  std::vector< memfile > mfs;
  std::vector< std::string > newfnames;
  std::vector< std::string > oldfnames;
    
  memfsys myfsys;
    
  std::string fnamebase= "reqfile";
  for(size_t f=0; f<numfiles; ++f)
    {
      memfile mf = receive_file_from_root( mytag );
      //mfs.push_back( mf );

#ifdef PRINTWORKER
      fprintf(stdout, "RANK [%d] TAG [%d] (worker [%d]): Received file with fname [%s]\n", getrank(), mytag, (size_t)getworker(getrank, mytag), mf.filename.c_str() );
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
      fprintf(stdout, "IN RANK [%d] TAG [%d] (worker [%ld]): GOT LOCAL MEM FILE: [%s]\n", getrank(), mytag, getworker(getrank(), mytag), std::string(dir+"/"+myfname).c_str());
#endif
      mf.filename = dir + "/" + myfname;
	
      if( todisk == true)
	{
#ifdef PRINTWORKER
	  fprintf(stdout, "IN RANK [%d] TAG [%d] (worker [%ld]): PRINTING TO DISK LOCAL MEM FILE: [%s]\n", world.rank(), , mytag, getworker(world.rank(), mytag), std::string(dir+"/"+myfname).c_str());
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
pitem filesender::handle_cmd( const psweep_cmd& pcmd, const int& mytag )
{
  //get the CMD from it, return...
  if( pcmd.SRC != 0 )
    {
      fprintf(stderr, "ERROR rank [%d] tag [%d] (worker: [%ld]) got cmd from non-root rank...[%d]\n", getrank(), mytag, getworker(getrank(), mytag),  pcmd.SRC);
      exit(1); //EXIT more gracefully?
    }

  //if( strcmp( pcmd.CMD, "EXIT") == 0 )
  if( pcmd.CMD.compare("EXIT") == 0 )
    {
      //exit
      fprintf(stderr, "::: rank [%d] tag [%d] (worker: [%ld]) received EXIT command from root rank\n", getrank(), mytag, (size_t)getworker(getrank(), mytag));
      exit(1);
    }
  //else if( strcmp( pcmd.CMD, "PITEM") == 0 )
  else if( pcmd.CMD.compare( "PITEM") == 0 )
    {
      //will process this pitem.
      pitem mypitem = receive_pitem_from_root( mytag ); //get_pitem_from_targ_rank( 0 );

      return mypitem;
    }
  else
    {
      fprintf(stderr, "ERROR::: rank [%d] tag [%d] (worker: [%ld]) received unknown command [%s]\n", getrank(), mytag, getworker(getrank(), mytag), pcmd.CMD.c_str() );
      exit(1);
    }
}


void filesender::worker_notify_finished( pitem& mypitem, memfsys& myfsys, const int& mytag )
{

  fprintf(stdout, "WORKER [%ld] sending DONE to root\n", getworker( getrank(), mytag ) );
  send_cmd_to_root( "DONE", mytag );

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

  fprintf(stdout, "WORKER [%ld] done sending DONE to root, will send INT\n", getworker( getrank(), mytag ) );
  
  //Note, all OUTPUT are automatically appended to SUCCESS, so just
  //return all SUCCESS files.
  size_t nsuccess = mypitem.success_files.size();


  send_int_to_root( nsuccess, mytag);
  fprintf(stdout, "WORKER [%ld] done sending INT to root. Will now send [%ld] files\n", getworker( getrank(), mytag ), nsuccess );  
  for(size_t x=0; x<nsuccess; ++x)
    {
      std::string mfname = mypitem.success_files[x];
      
      memfile_ptr mfp = myfsys.open( mfname, todisk );
      //REV: This is where a problem could happen...
      //Send from disk if todisk is true...
      send_file_to_root( mfp.get_memfile(), mytag );
      mfp.close();
    }


  fprintf(stdout, "WORKER [%ld] done sending FILES to root. Will send VARLIST\n", getworker( getrank(), mytag ) );
  varlist<std::string> resvar = mypitem.get_output( myfsys, todisk );
    
  send_varlist_to_root( resvar, mytag );
  fprintf(stdout, "WORKER [%ld] done sending VARLIST to root\n", getworker( getrank(), mytag ) );
    
  return;
}


//REV: this needs to "find" which PITEM was allocated to that worker/ thread.
//REV: Note we could use todisk, but easier to do it as todisk?
varlist<std::string> filesender::handle_finished_work( const psweep_cmd& pc, pitem& corresp_pitem, memfsys& myfsys, const bool& usedisk )
{
  int targworker = pc.SRC;
  std::string cmd = pc.CMD;
  if( cmd.compare("DONE") != 0 )
    {
      fprintf(stderr, "WHOA, got a non-DONE cmd?!! From worker [%d]. CMD: [%s]\n", targworker, cmd.c_str() );
      exit(1);
    }
  //else, do all receiving for that CMD blocking all others for now.
  //I.e. only receive messages from others... Assume it will leave
  //(received) buffers from other guys in place while I'm doing this?
  //Or, allow it to do non-blocking, i.e. spin off threads? That
  //seems best.
  fprintf(stdout, "ROOT: handling finished work from worker [%d]. Getting INT\n", targworker);
  size_t nfiles = receive_int_from_worker( targworker );
  fprintf(stdout, "ROOT: handling finished work from worker [%d]. Got INT (nfiles=[%ld])\n", targworker, nfiles);

  for(size_t x=0; x<nfiles; ++x)
    {
      memfile mf = receive_file_from_worker( targworker );
      myfsys.add_file( mf );
	
      std::string origdir = corresp_pitem.mydir;
      
      std::string fname="ERRORFNAME";
      std::string dirofreceived = get_canonical_dir_of_fname( mf.filename, fname );
      std::string newlocal = origdir + "/" + fname;
            
      std::vector<bool> tmpmarked( corresp_pitem.success_files.size(), false);
      
      std::vector<size_t> matched = find_matching_files( newlocal, corresp_pitem.success_files, tmpmarked );

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

      if( usedisk )
	{
	  memfile_ptr mfp(mf);
	  mfp.tofile( origdir+"/"+fname );
	  mfp.close();
	}
    }
  fprintf(stdout, "ROOT: handling finished work from worker [%d]. Finished receiving all files (nfiles=[%ld]). Now getting varlist\n", targworker, nfiles);

  varlist<std::string> outputvlist = receive_varlist_from_worker( targworker );
  fprintf(stdout, "ROOT: handling finished work from worker [%d]. GOT varlist\n", targworker);
  
  return outputvlist;
}

size_t filesender::get_local_rank()
{
  return mylocalidx;
}

void filesender::mangle_with_local_worker_idx( pitem& mypitem )
{
  if( mypitem.setlocalidx.size() > 0 )
    {
      for(size_t x=0; x<mypitem.setlocalidx.size(); ++x)
	{
	  if(mypitem.setlocalidx[x] >= mypitem.mycmd.size())
	    {
	      fprintf(stderr, "REV: ERROR: in mangle_with_local_worker_idx: requested index of arg in mycmd [%ld] is larger than size of array [%ld]\n", mypitem.setlocalidx[x], mypitem.mycmd.size() );
	      exit(1);
	    }
	  
	  mypitem.mycmd[ mypitem.setlocalidx[x] ] = std::to_string( get_local_rank() );
	}
    }
}


//REV: Updated 11 Mar 2016 to do a "hard" search to overwrite at RECEIVE time my specific local worker number
bool filesender::execute_work( pitem& mypitem, memfsys& myfsys )
{
  mangle_with_local_worker_idx( mypitem );
  
  bool success = mypitem.execute_cmd( fakesys, myfsys );
  
  return success;
}

void filesender::cleanup_workspace( const pitem& mypitem )
{
  boost::filesystem::path p( mypitem.mydir );
  uintmax_t removed = boost::filesystem::remove_all( p );
}

//REV: 11 Mar 2016: MODIFIED TO GET MY OWN RANK IN HERE ;)
//void filesender::execute_slave_loop( const size_t& mytag, const std::string& runtag
//REV: Can't use refs b/c I start with thread
void filesender::execute_slave_loop( const size_t mytag, const std::string runtag )
{
  bool loopslave=true;
  std::string LOCALDIR = "/tmp/" + runtag + "_" + std::to_string( getworker( getrank(), mytag ) );
  
  fprintf(stdout, "WORKER [%ld]  (rank [%ld], thread [%ld]): GPU device is [%ld]. Starting slave loop...\n", getworker(getrank(), mytag), getrank(), mytag, mygpuidx );
  //REV: This will do nothing if there is no CUDA.
  set_cuda_device( mygpuidx ); 

  fprintf(stdout, "WORKER [%ld]  (rank [%ld], thread [%ld]): GPU device is [%ld]. SET GPU, now sending READY to root!\n", getworker(getrank(), mytag), getrank(), mytag, mygpuidx );
  //REV: need to send first guy to tell its ready
  std::string pcinit = "READY";
  send_cmd_to_root( pcinit, mytag );

  fprintf(stdout, "WORKER [%ld]  (rank [%ld], thread [%ld]): GPU device is [%ld]. **DONE** sending READY to root. Now waiting for CMD!\n", getworker(getrank(), mytag), getrank(), mytag, mygpuidx );
  
  while( loopslave == true )
    {
      make_directory( LOCALDIR );
      
      psweep_cmd cmd = receive_cmd_from_root( mytag );
      fprintf(stdout, "WORKER [%ld]  (rank [%ld], thread [%ld]): GPU device is [%ld]. GOT CMD FROM ROOT!!!!!\n", getworker(getrank(), mytag), getrank(), mytag, mygpuidx );
      
      if( cmd_is_exit( cmd ) == true )
	{
	  fprintf(stderr, "REV: WORKER [%ld] received EXIT\n", getworker( getrank(), mytag) );
	  loopslave = false;
	  break;
	}

      //REV: may EXIT, or contain a PITEM (to execute).
      pitem mypitem = handle_cmd( cmd, mytag );
      fprintf(stdout, "WORKER [%ld]  (rank [%ld], thread [%ld]): GPU device is [%ld]. handled CMD. Will handle PITEM!!!!!!\n", getworker(getrank(), mytag), getrank(), mytag, mygpuidx );
	
      memfsys myfsys = worker_handle_pitem( mypitem, LOCALDIR, mytag);
      fprintf(stdout, "WORKER [%ld]  (rank [%ld], thread [%ld]): GPU device is [%ld]. handled PITEM. Executing!!!!!!\n", getworker(getrank(), mytag), getrank(), mytag, mygpuidx );
		
      bool blah = execute_work( mypitem, myfsys );

      fprintf(stdout, "WORKER [%ld]  (rank [%ld], thread [%ld]): GPU device is [%ld]. DONE Executing, notifying of finished...!!!!!!\n", getworker(getrank(), mytag), getrank(), mytag, mygpuidx );

      //this includes the many pieces of "notifying, waiting for response, then sending results, then waiting, then sending files, etc..
      worker_notify_finished( mypitem, myfsys, mytag );
      
      fprintf(stdout, "WORKER [%ld]  (rank [%ld], thread [%ld]): GPU device is [%ld]. DONE notifying, cleaning up and waiting for next cmd...!!!!!!\n", getworker(getrank(), mytag), getrank(), mytag, mygpuidx );
      
      cleanup_workspace( mypitem );
    }
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
  send_cmd_to_worker( "PITEM", workeridx );

  send_pitem_to_worker( mypitem, workeridx );

  int nfiles = mypitem.required_files.size();
    
  send_int_to_worker( nfiles, workeridx );

  //Then, send the files.
  for(size_t f=0; f<mypitem.required_files.size(); ++f)
    {
      //REV: Yea...just send files from here. Problem is, some of them might not be in MEMFSYS??? We should load them all at some point (required,
      //and input)
      memfile_ptr mfp = myfsys.open( mypitem.required_files[f], true ); //will attempt to read through if it does not exist. Note may be empty?
	
      //send_file_from_disk( workeridx, mypitem.required_files[f] );
      send_file_to_worker( mfp.get_memfile(), workeridx );
      mfp.close();
    }

}



  


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
      fprintf(stdout, "Not all done of WPROG! WORKING: \n");
      for(size_t x=0; x<_workingworkers.size(); ++x)
	{
	  if( _workingworkers[x] == true )
	    {
	      fprintf(stdout, " [%ld] ", x );

	    }
	}
      fprintf(stdout, "\n");
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
	  fprintf(stdout, "CASE: Either there is a worker available, OR there is work available. I will receive a response from a worker\n");
	  if( wprog.avail_worker( _workingworkers ) == true )
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

	  psweep_cmd pcmd = receive_cmd_from_any_worker();
	  
	  if( is_finished_work( pcmd ) == true )
	    {
	      //pcmd contains DONE cmd
	      //I need to handle it based on corresponding worker.
	      size_t workernum = pcmd.SRC;

	      //farmed_status[workernum] should tell us where
	      parampoint_coord pc = wprog.farmed_status[workernum];
		
	      //fprintf(stdout, "&&& Recevied finished cmd from worker [%ld]. The received was coordinate: PPOINT [%ld], PSET [%ld], PITEM [%ld]\n", workernum,  pc.parampointn, pc.psetn, pc.pitemn );
	      
	      pitem handledpitem = wprog.get_corresponding_pitem_from_workernum( pg, workernum );
	      //fprintf(stdout, "Got handled pitem from that coordinate\n");

	      
	      //Specifically, call it on the parampoint#, from pg.parampoint_memfsystems[ pc.parampointn ].
	      //REV: THIS will write to files! Modify to use a local (temporary) memfsys
	      varlist<std::string> result = handle_finished_work( pcmd, handledpitem, pg.parampoint_memfsystems[ pc.parampointn ], usedisk );
		
	      fprintf(stdout, "MASTER: got result from worker [%ld]. Now marking done...\n", workernum);
	      
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
	      fprintf(stdout, "NO, IT WAS JUST READY (setting worker [%d] to FALSE)\n", pcmd.SRC);
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
	  fprintf(stdout, "CASE: BOTH WORK AND A WORKER ARE AVAILABLE. WILL FARM\n");

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
	  fprintf(stdout, "DONE master to slave.\n");
	}
    } //end while !all done.
  fprintf(stdout, "\n\nROOT FINISHED GENERATION? (comp pp)\n\n");
} //end comp_pp_list

