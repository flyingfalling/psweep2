
#include <filesender.h>
#include <unistd.h>

#define ROOTWORKER 0


void filesender::broadcast_to_workers( const std::string& cmd )
{
  for(size_t w=1; w<_workingworkers.size(); ++w)
    {
      send_cmd_to_worker( cmd, ROOTWORKER, w );
    }
}

void filesender::signal_exit_to_workers()
{
  if(!getrank() == 0 )
    {
      fprintf(stderr, "REV: ERROR, calling exit from non-zero rank!!!! Still might be from non-zero thread, we are not checking...\n");
    }
  std::string cmd = "EXIT";
  //REV: If threads havent been spawned yet, nothing will happen at join? Will it hang?
  broadcast_to_workers( cmd );
}

void filesender::start_worker_loop(const std::string& runtag)
{
  //REV: In here, I will mess around with threads!
  //Each filesender instance will only have a single dev#, etc.
  //but multiple threads of course. First thread is one of them.
  //std::thread mythread();
  //std::vector< std::thread > thrs;

  if(workersperrank > 1)
    {
      workerthreads.resize( workersperrank-1 );
      for(size_t tag=0; tag<workerthreads.size(); ++tag)
      {
	int myworker = getworker( getrank(), tag+1 );
	
	//REV: This is issue.
	//If I call method, how does it know which "base" class to use? Copy of whole "this" filesender?
	//Need to use "this" as second, due to the execute class funct issue.
	workerthreads[tag] = std::thread( &filesender::execute_slave_loop, this, myworker, runtag );
      }
    }

  int myworker = getworker( getrank(), 0 );
  execute_slave_loop(myworker, runtag);

  //This execute slave loop maintains control.
  
}


//Special case for ROOT...
void filesender::start_worker_loop_ROOT(const std::string& runtag)
{
  workerthreads.resize( workersperrank );
  for(size_t tag=0; tag<workerthreads.size(); ++tag)
    {
      int myworker = getworker( getrank(), tag+1 ); //Tag +1 because ROOT worker is the other one. Note,
      //all computations must add 1 now, fuck.
      
      //REV: This is issue.
      //If I call method, how does it know which "base" class to use? Copy of whole "this" filesender?
      //Need to use "this" as second, due to the execute class funct issue.
      workerthreads[tag] = std::thread( &filesender::execute_slave_loop, this, myworker, runtag );
    }

  //ROOT thread maintains control
  
}


//REV: This will get processor name (only for the root). All those with same number as ROOT remember that they need to add 1.
//This way I only compute it once, a bit better this way... I could have user-side store a GPU associate counter for speed though ;)
//REV: Called BEFORE threads spawned.
//REV: BIG PROBLEM: we can not have different RANKS accessing same GPU, but we can have same RANK accessing multi
//GPUs. If e.g. 4 GPUs and 6 threads, what do we do? Just 1 1 1 1? Or 1 2 1 2?
//REV: Ghetto, make variable number of threads per GPU? Or something? Assume user will set num threads and num
//ranks to fit #GPUs...? Each rank will occupy exactly a single GPU...
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


  return;

  //REV: I don't do this anymore b/c root includes other threads.
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

    }
  fprintf(stdout, "FINISHED init local worker idx!!!\n");
  
} //init_local_worker_idx done.


int filesender::getworker( const int& rank, const int& tag )
{
  if( rank==0 )
    {
      return tag;
    }
  else
    {
      int myworker = 1 + (rank * workersperrank) + tag;
      return myworker;
    }
}

int filesender::getworkerrank( const int& myworker )
{
  if( myworker < (workersperrank+1) )
    {
      return 0;
    }
  else
    {
      int myr = ((myworker-1) / workersperrank);
      return myr;
    }
}

int filesender::getworkertag( const int& myworker )
{
  if(myworker < (workersperrank+1))
    {
      return myworker;
    }
  else
    {
      int mytag = ((myworker-1) % workersperrank);
      return mytag;
    }
}

void filesender::initfilesender()
{
  //MPI_init_thread(argc, argv, int required, int* provited)
  MPI_Init(0, NULL);
  
  myrank = world.rank();
  
  //Assume that world/env are automatically constructed?
  init_local_worker_idx();
  
  mygpuidx=compute_gpu_idx( mylocalidx, workersperrank, getrank() );

  size_t nworkers = world.size()*workersperrank+1; //REV: Even main thread has other workers...
  
  bool currworking=true;

  //Only need this in root rank...careful to not try to access it in worker ranks ;)
  if( myrank == 0 )
    {
      _workingworkers.resize( nworkers, true );
    }

  if( sizeof(int) != 4 || sizeof(uint16_t) != 2 )
    {
      fprintf( stderr, "REV: Tag sending error messed up...for sendrecvstruct, assume that int=32, but was [%ld] bytes, uint16_t is 16 bits but was [%ld] bytes...This *may* not be a problem, if so comment this check out...\n", sizeof(int), sizeof(uint16_t));
      exit(1);
    }
#define BYTESIZE_BITS 8
  uint32_t maxsize=0xFFFFFFFF; //should be all 1s
  long holdersize = sizeof(uint32_t)*BYTESIZE_BITS;
  long bitspertag = sizeof(uint16_t)*BYTESIZE_BITS;
  if( bitspertag > holdersize )
    {
      fprintf(stderr, "Size of holder bytes > size of bitspertag\n");
      exit(1);
    }
  uint32_t shiftover=(holdersize - bitspertag); //I.e. shift over "all but 16"
  maxsize = maxsize >> shiftover; //should fill left side with zeros.
  fprintf(stdout, "Max size representable by [%ld] bits in sendrecvstruct: [%d]\n", bitspertag, maxsize);
  if( workersperrank >= maxsize )
    {
      fprintf(stderr, "REV: too many workersperrank to be represented in half-size of sendrecvstruct elements\n");
      exit(1);
    }

  if( sizeof(sendrecvstruct) != sizeof(int) )
    {
      fprintf(stderr, "REV: WHOA WHOA WTF SIZE OF SENDRECV STRUCT is NOT sizeof INT. struct is [%ld]\n", sizeof(sendrecvstruct));
      exit(1);
    }
  else
    {
      fprintf(stdout, "REV: Sanity check passed: sendrecvstruct is same size as int [%ld] vs [%ld]\n", sizeof(sendrecvstruct), sizeof(int));
    }


  sendrecvstruct tag;
  tag = sendrecvstruct( 1, 1 );
  fprintf(stdout, "REV: Sanity check of STRUCT for send/recv: setting 1-1 causes as int [%d], and sendtag is [%d], recvtag is [%d]. (Can I access as int directly... [%d]?)\n", tag.getasint(), tag.getsendtag(), tag.getrecvtag(), tag.asint );
  
  fprintf(stdout, "Rank [%d] finished initialize file sender, will now spawn threads...?\n", getrank());
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
  for(size_t w=0; w<workerthreads.size(); ++w)
    {
      fprintf(stdout, "(FILESENDER destructor:) Rank [%d], WAITING to join worker thread [%ld]\n", getrank(), w);
      workerthreads[w].join();
      fprintf(stdout, "(FILESENDER destructor:) Rank [%d], joined worker thread [%ld]\n", getrank(), w);
    }
  fprintf(stdout, "(FILESENDER destructor:) Rank [%d], all threads joined, Calling MPI finalize and exiting.\n", getrank());
  
  MPI_Finalize();
}

bool filesender::checkroot( const int& myworker )
{
  if( getrank() == 0 && getworkertag( myworker ) == 0 )
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


bool filesender::probe( const int& srcworker, const int& myworker )
{
  sendrecvstruct sr( srcworker, myworker );
  
  boost::optional< boost::mpi::status > msg = world.iprobe( getworkerrank(srcworker), sr.getasint() );
  
  if(msg)
    {
      return true; //only care if there is a message...
    }
  else
    {
      return false;
    }
}

bool filesender::probe_any( const int& myworker, sendrecvstruct& tag )
{
  //fprintf(stdout, "Attempting to PROBE ANY!!\n");
  //boost::optional< boost::mpi::status > msg = world.iprobe( getworkerrank( myworker ), boost::mpi::any_tag );
  boost::optional< boost::mpi::status > msg = world.iprobe( boost::mpi::any_source, boost::mpi::any_tag );
  
  //fprintf(stdout, "Finished probe...\n");
  if(msg) //if we actually got a message... (REV: Fuck we might have problems if we are waiting for a worker to consume something...)
    {
      //fprintf(stdout, "We got a message!!!!! From src [%d], tag is [%d]\n", msg->source(), msg->tag());
      tag = sendrecvstruct( msg->tag() );
      if( getworkerrank( tag.getsendtag() ) != msg->source() )
	{
	  fprintf(stderr, "REV: Something is wrong in how I coded worker conversion to tags, a message with sendtag [%d], i.e. src rank [%d] but source rank [%d] exists\n", tag.getsendtag(), getworkerrank(tag.getsendtag()), msg->source() );
	  exit(1);
	}
      if( tag.getrecvtag() == myworker )
	{
	  //fprintf(stdout, "YAY got the probe, i.e. target is me!!!!!!! \n");
	  return true; //only care if there is a message...
	}
      else
	{
	  return false;
	}
    }
  else
    {
      //fprintf(stdout, "Nope, still nothing from the probe...\n");
      return false;
    }
}


int filesender::getrank()
{
  return myrank;
}

void filesender::broadcast_cmd( const std::string& cmd ) 
{
  //world.broadcast( MPI_COMM_WORLD, cmd, world.rank() );
  std::string c=cmd;
  boost::mpi::broadcast( world, c, world.rank() );
  return;
}

template <typename T>
void filesender::send( const T& v, const int& myworker, const int& targworker )
{
  sendrecvstruct tag( myworker, targworker );
  lmux();
  world.send( getworkerrank(targworker), tag.getasint(), v );
  ulmux();
}

void filesender::send_varlist_to_worker(  const varlist<std::string>& v, const int& myworker, const int& targworker )
{
  if( checkroot( myworker ) == false )
    {
      fprintf(stderr, "ERROR: REV: send varlist to worker, should only be called from ROOT rank 0/thread, but calling from worker [%d]\n", myworker );
      exit(1);
    }
  
  sendrecvstruct tag( myworker, targworker );
  lmux();
  world.send( getworkerrank(targworker), tag.getasint(), v );
  ulmux();
}

void filesender::send_varlist_to_root( const varlist<std::string>& v, const int& myworker )
{
  if( checkroot( myworker ) == true )
    {
      fprintf(stderr, "ERROR: REV: send varlist to root, should only be called from non-ROOT rank >0, but calling from worker [%d]\n", myworker);
      exit(1);
    }

  sendrecvstruct tag( myworker, ROOTWORKER );
  lmux();
  world.send( getworkerrank(ROOTWORKER), tag.getasint(), v ); //Rank will be implicitly known by root based on SRC
  ulmux();
}



void filesender::send_cmd_to_worker( const std::string& cmd, const int& myworker, const int& targworker )
{
  if( checkroot( myworker ) == false )
    {
      fprintf(stderr, "ERROR: REV: send cmd to worker, should only be called from ROOT rank 0, but calling from worker [%d]\n", myworker);
      exit(1);
    }
  sendrecvstruct tag( myworker, targworker );
  lmux();
  world.send( getworkerrank(targworker), tag.getasint(), cmd );
  ulmux();
}


void filesender::send_cmd_to_root( const std::string& cmd, const int& myworker )
{
  if( checkroot( myworker ) == true )
    {
      fprintf(stderr, "ERROR: REV: send varlist to root, should only be called from non-ROOT rank >0, but calling from worker [%d]\n", myworker);
      exit(1);
    }
  
  sendrecvstruct tag( myworker, ROOTWORKER);

  //REV: sanity check... if my worker is in upper 16 bits, it should be quite large...
  //sendrecvstruct tag2( 1, 0 );
  //fprintf(stdout, "REV: SANITY CHECKING: I have created a sendrecvstruct with (1, 0), which means that bit 16 should be a 1, and all others are zero. This implies value should be 2^16, i.e. 65536. Value is [%d]\n", tag2.getasint() );
  //exit(1);
  
    
  //fprintf(stdout, "WORKER [%d]: ATTEMPTING TO LOCK MUX for send CMD to ROOT\n", myworker );

  lmux();

  //fprintf(stdout, "WORKER [%d]: SUCCEEDED TO LOCK MUX for send CMD to ROOT\n", myworker );

  //fprintf(stdout, "Worker [%d], sending cmd [%s] to ROOT, ostensibly with tag [%d], which contains src [%d] and targ [%d]. Will be send to rank [%d]\n", myworker, cmd.c_str(), tag.getasint(), tag.getsendtag(), tag.getrecvtag(), getworkerrank( ROOTWORKER ) );
  
  world.send( getworkerrank( ROOTWORKER ), tag.getasint(), cmd );

  //fprintf(stdout, "WORKER [%d]: FINISHED send CMD to ROOT, will unlock mux\n", myworker );

  ulmux();
}


void filesender::send_pitem_to_worker( const pitem& mypitem, const int& myworker, const int& targworker )
{
  if( checkroot( myworker ) == false )
    {
      fprintf(stderr, "ERROR: REV: send pitem to worker, should only be called from ROOT rank 0, but calling from worker [%d]\n", myworker);
      exit(1);
    }

  sendrecvstruct tag( myworker,  targworker);
  
  lmux();
  world.send( getworkerrank(targworker), tag.getasint(), mypitem ); //will serialize it for me.
  ulmux();
}

void filesender::send_pitem_to_root( const pitem& mypitem, const int& myworker )
{
  if( checkroot( myworker ) == true )
    {
      fprintf(stderr, "ERROR: REV: send pitem to root, should only be called from ROOT rank 0, but calling from worker [%d]\n", myworker);
      exit(1);
    }
  sendrecvstruct tag( myworker, ROOTWORKER );
  lmux();
  world.send( getworkerrank(ROOTWORKER), tag.getasint(), mypitem );
  ulmux();
}

template <typename T>
void filesender::recv( const int& sendworker, const int& myworker, T& val )
{
  sendrecvstruct tag( sendworker, myworker );
  int sendrank = getworkerrank( sendworker );

  bool gotmesg=false;
  while(gotmesg==false)
    {
      lmux();
      gotmesg = probe( sendworker, myworker );
      if( gotmesg )
	{
	  world.recv( sendrank, tag.getasint(), val );
	  ulmux();
	}
      else
	{
	  ulmux();
	  usleep( 100 ); //microseconds?
	}
    }
}

template <typename T>
sendrecvstruct filesender::recv_any( const int& myworker, T& val )
{
  sendrecvstruct tag;
  bool gotmesg=false;
  while(gotmesg==false)
    {
      lmux();
      gotmesg = probe_any( myworker, tag );
      if( gotmesg )
	{

	  int sendworker= tag.getsendtag();
	  int sendrank = getworkerrank( sendworker );
	  //fprintf(stdout, "REV: recvany: OK, got message of sender [%d] (to worker [%d], should be 0). Will try to recv from that rank [%d]\n", tag.getsendtag(), tag.getrecvtag(), sendrank );
	  world.recv( sendrank, tag.getasint(), val );
	  //fprintf(stdout, "REV: recvany: Finished recv!\n");
	  ulmux();
	}
      else
	{
	  //fprintf(stdout, "REV: recvany: Nope, still sleeping...\n");
	  ulmux();
	  usleep( 1000 ); //microseconds?
	}
    }
  return tag;
}

//If this is blocking, we have an issue. Ignore for now...
varlist<std::string> filesender::receive_varlist_from_worker( const int& srcworker, const int& myworker )
{
  if( checkroot( myworker ) == false )
    {
      fprintf(stderr, "ERROR: REV: recv varlist worker, should only be called from ROOT rank 0 (tag 0), but calling from worker [%d]\n", myworker );
      exit(1);
	  
    }
  varlist<std::string> tmpv;
  
  recv< varlist<std::string> >( srcworker, myworker, tmpv );
  
  return tmpv;
}

varlist<std::string> filesender::receive_varlist_from_root( const int& myworker )
{
  if( checkroot( myworker ) == true )
    {
      fprintf(stderr, "ERROR: REV: recv varlist root, should only be called from non ROOT rank != 0 or tag >0, but calling from worker [%d]\n", myworker);
      exit(1);
    }
  
  varlist<std::string> tmpv;

  recv<varlist<std::string>>( ROOTWORKER, myworker, tmpv );
  return tmpv;
}

psweep_cmd filesender::receive_cmd_from_any_worker( const int& myworker )
{
  if( checkroot( myworker ) == false )
    {
      fprintf(stderr, "ERROR: REV: recv cmd from any worker, should only be called from ROOT rank 0, but calling from worker [%d]\n", myworker );
      exit(1);
    }
  
  std::string data;

  //fprintf(stdout, "Will recieve from any worker...\n");
  sendrecvstruct sr = recv_any<std::string>( ROOTWORKER, data );
  //fprintf(stdout, "Finished receiving from any worker\n");
  /*lmux();
  boost::mpi::status msg = world.probe();
  //world.recv(msg.source(), boost::mpi::any_tag, data);
  world.recv(msg.source(), msg.tag(), data);
  ulmux();
  */
  psweep_cmd pc( sr.getsendtag(), data );
  return pc;
}

//FOR INIT ONLY!!!!
psweep_cmd filesender::receive_cmd_from_root( )
{
  std::string cmd;
  world.recv( 0, boost::mpi::any_tag, cmd);
  return psweep_cmd( 0, cmd );
}

psweep_cmd filesender::receive_cmd_from_root( const int& myworker )
{
  if( checkroot(myworker) == true )
    {
      fprintf(stderr, "ERROR: REV: recv cmd from ROOT (noarg), should only be called from non-ROOT rank >0, but calling from worker [%d]\n", myworker);
      exit(1);
    }
  
  std::string data;

  recv<std::string>( ROOTWORKER, myworker, data );
  
  psweep_cmd pc( ROOTWORKER, data );

  return pc;
}


pitem filesender::receive_pitem_from_root( const int& myworker ) 
{
  if( checkroot( myworker) == true )
    {
      fprintf(stderr, "ERROR: REV: recv pitem from ROOT, should only be called from non-ROOT rank >0, but calling from worker [%d]\n", myworker);
      exit(1);
    }
  pitem newpitem;

  recv<pitem>( ROOTWORKER, myworker, newpitem );
  
  return newpitem;
}

pitem filesender::receive_pitem_from_worker( const int& targworker, const int& myworker ) 
{
  if( checkroot( myworker ) == false )
    {
      fprintf(stderr, "ERROR: REV: recv pitem from worker, should only be called from ROOT rank 0, but calling from [%d]\n", myworker);
      exit(1);
    }
  
  pitem newpitem;

  recv<pitem>( targworker, ROOTWORKER, newpitem );
  
  return newpitem;
}

int filesender::receive_int_from_root( const int& myworker )
{
  if( checkroot( myworker ) == true )
    {
      fprintf(stderr, "ERROR: REV: recv int from ROOT, should only be called from non-ROOT rank >0, but calling from worker [%d]\n", myworker);
      exit(1);
    }
  int newint;

  recv<int>( ROOTWORKER, myworker, newint );
  
  return newint;
}

int filesender::receive_int_from_worker( const int& targworker, const int& myworker ) 
{
  if( checkroot( myworker ) == false )
    {
      fprintf(stderr, "ERROR: REV: recv int from worker, should only be called from ROOT rank 0, but calling from worker [%d]\n", myworker);
      exit(1);
    }
  int newint;
  recv<int>( targworker, ROOTWORKER, newint);
  return newint;
}


void filesender::send_int_to_root( const int& tosend, const int& myworker ) 
{
  if( checkroot( myworker ) == true )
    {
      fprintf(stderr, "ERROR: REV: send int to ROOT, should only be called from non-ROOT rank >0, but calling from worker [%d]\n", myworker );
      exit(1);
    }

  sendrecvstruct tag( myworker, ROOTWORKER );
  lmux();
  world.send( ROOTWORKER, tag.getasint(), tosend );
  ulmux();
}

void filesender::send_int_to_worker( const int& tosend, const int& myworker, const int& targworker )
{
  if( checkroot( myworker ) == false )
    {
      fprintf(stderr, "ERROR: REV: send int to worker, should only be called from ROOT rank 0, but calling from worker [%d]\n", myworker);
      exit(1);
    }
  sendrecvstruct tag( ROOTWORKER, targworker );
  lmux();
  world.send( getworkerrank(targworker), tag.getasint(), tosend );
  ulmux();
}


void filesender::send_file_to_worker( const memfile& memf, const int& myworker, const int& targworker)
{
  if( checkroot( myworker ) == false )
    {
      fprintf(stderr, "ERROR: REV: send file to worker, should only be called from ROOT rank 0, but calling from worker [%d]\n", myworker);
      exit(1);
    }
  sendrecvstruct tag( ROOTWORKER, targworker );
  lmux();
  world.send( getworkerrank(targworker), tag.getasint(), memf );
  ulmux();
}

void filesender::send_file_to_root( const memfile& memf, const int& myworker )
{
  if( checkroot( myworker ) == true )
    {
      fprintf(stderr, "ERROR: REV: send file to ROOT, should only be called from non-ROOT rank >0, but calling from worker [%d]\n", myworker);
      exit(1);
    }
  sendrecvstruct tag( myworker, ROOTWORKER );
  lmux();
  world.send( ROOTWORKER, tag.getasint(), memf );
  ulmux();
}
  
void filesender::send_file_to_root_from_disk( const std::string& fname, const int& myworker )
{
  bool readfromdisk=true;
  memfile mf( fname, readfromdisk );
  send_file_to_root( mf, myworker );
}

void filesender::send_file_to_worker_from_disk( const std::string& fname, const int& myworker, const int& targworker )
{
  bool readfromdisk=true;
  memfile mf( fname, readfromdisk );
  send_file_to_worker( mf, myworker, targworker );
}

memfile filesender::receive_file_from_worker( const int& targworker, const int& myworker )
{
  if( checkroot( myworker ) == false )
    {
      fprintf(stderr, "ERROR: REV: recv file from worker, should only be called from ROOT rank 0, but calling from worker [%d]\n", myworker);
      exit(1);
    }
  memfile mf;
  recv<memfile>(targworker, ROOTWORKER, mf);
  return mf;
}

memfile filesender::receive_file_from_root( const int& myworker )
{
  if( checkroot( myworker ) == true )
    {
      fprintf(stderr, "ERROR: REV: recv file from ROOT, should only be called from non-ROOT rank >0, but calling from [%d]\n", myworker);
      exit(1);
    }
  memfile mf;
  recv<memfile>( ROOTWORKER, myworker, mf );
  return mf;
}


//Handle pitem is called by WORKER
memfsys filesender::worker_handle_pitem( pitem& mypitem, const std::string& dir, const int& myworker )
{
  
  //First, get an INT, number of files.
  int numfiles = receive_int_from_root( myworker );
  std::vector< memfile > mfs;
  std::vector< std::string > newfnames;
  std::vector< std::string > oldfnames;
    
  memfsys myfsys;
    
  std::string fnamebase= "reqfile";
  for(size_t f=0; f<numfiles; ++f)
    {
      memfile mf = receive_file_from_root( myworker );
      
      //fprintf(stdout, "RANK [%d] TAG [%d] (worker [%d]): Received file with fname [%s]\n", getrank(), mytag, (size_t)getworker(getrank, mytag), mf.filename.c_str() );
      	
      std::string myfname = fnamebase + std::to_string( f );
      newfnames.push_back( myfname );
      oldfnames.push_back( mf.filename ); //this is old fname, I guess I could get it elsewhere... myfname is the NEW one...
      
      
      //fprintf(stdout, "IN RANK [%d] TAG [%d] (worker [%ld]): GOT LOCAL MEM FILE: [%s]\n", getrank(), mytag, getworker(getrank(), mytag), std::string(dir+"/"+myfname).c_str());
      
      mf.filename = dir + "/" + myfname;
	
      if( todisk == true)
	{
	  //fprintf(stdout, "IN RANK [%d] TAG [%d] (worker [%ld]): PRINTING TO DISK LOCAL MEM FILE: [%s]\n", world.rank(), , mytag, getworker(world.rank(), mytag), std::string(dir+"/"+myfname).c_str());
	  mf.tofile( dir + "/" + myfname );
	}

      myfsys.add_file( mf );
    }
  
  mypitem.re_base_directory( mypitem.mydir, dir, oldfnames, newfnames);
  //REV: I will now create memfsys using that array as the thing...I will write them out if necessary.
        
  mypitem.mydir = dir;
  //mypitem will now be rebased appropriately, although hierarchical varlists etc. are not carried with it of course.
    
  
  return myfsys;
}

  

bool filesender::cmd_is_exit(  const psweep_cmd& pcmd )
{
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
pitem filesender::handle_cmd( const psweep_cmd& pcmd, const int& myworker )
{
  //get the CMD from it, return...
  if( pcmd.SRC != 0 )
    {
      fprintf(stderr, "ERROR rank [%d] tag [%d] (worker: [%d]) got cmd from non-root rank...[%d]\n", getrank(), getworkertag(myworker), myworker,  pcmd.SRC);
      exit(1); //EXIT more gracefully?
    }

  //if( strcmp( pcmd.CMD, "EXIT") == 0 )
  if( pcmd.CMD.compare("EXIT") == 0 )
    {
      //exit
      fprintf(stderr, "::: rank [%d] tag [%d] (worker: [%d]) received EXIT command from root rank\n", getrank(), getworkertag(myworker), myworker);
      exit(1);
    }
  //else if( strcmp( pcmd.CMD, "PITEM") == 0 )
  else if( pcmd.CMD.compare( "PITEM") == 0 )
    {
      //will process this pitem.
      pitem mypitem = receive_pitem_from_root( myworker ); //get_pitem_from_targ_rank( 0 );

      return mypitem;
    }
  else
    {
      fprintf(stderr, "ERROR::: rank [%d] tag [%d] (worker: [%d]) received unknown command [%s]\n", getrank(), getworkertag(myworker), myworker, pcmd.CMD.c_str() );
      exit(1);
    }
}


void filesender::worker_notify_finished( pitem& mypitem, memfsys& myfsys, const int& myworker )
{

  //fprintf(stdout, "WORKER [%d] sending DONE to root\n", myworker );
  //send_cmd_to_root( "DONE", myworker );
  send<std::string>( "DONE", myworker, ROOTWORKER );

  //fprintf(stdout, "WORKER [%d] done sending DONE to root, will send INT\n", myworker );
  
  //Note, all OUTPUT are automatically appended to SUCCESS, so just
  //return all SUCCESS files.
  size_t nsuccess = mypitem.success_files.size();
  
  
  //send_int_to_root( nsuccess, myworker);
  send<int>( (int)nsuccess, myworker, ROOTWORKER );
  
  //fprintf(stdout, "WORKER [%d] done sending INT to root. Will now send [%ld] files\n", myworker, nsuccess );  
  for(size_t x=0; x<nsuccess; ++x)
    {
      std::string mfname = mypitem.success_files[x];
      
      memfile_ptr mfp = myfsys.open( mfname, todisk );
      //REV: This is where a problem could happen...
      //Send from disk if todisk is true...
      //send_file_to_root( mfp.get_memfile(), myworker );
      send<memfile>( mfp.get_memfile(), myworker, ROOTWORKER );
      mfp.close();
    }


  //fprintf(stdout, "WORKER [%d] done sending FILES to root. Will send VARLIST\n", myworker );
  varlist<std::string> resvar = mypitem.get_output( myfsys, todisk );
    
  //send_varlist_to_root( resvar, myworker );
  send<varlist<std::string>>( resvar, myworker, ROOTWORKER );
  
  //fprintf(stdout, "WORKER [%d] done sending VARLIST to root\n", myworker );
    
  return;
}


//REV: this needs to "find" which PITEM was allocated to that worker/ thread.
//REV: Note we could use todisk, but easier to do it as todisk?
varlist<std::string> filesender::handle_finished_work( const psweep_cmd& pc, pitem& corresp_pitem, memfsys& myfsys, const int& myworker, const bool& usedisk )
{
  int targworker = pc.SRC;
  std::string cmd = pc.CMD;
  if( cmd.compare("DONE") != 0 )
    {
      fprintf(stderr, "WHOA, got a non-DONE cmd?!! From worker [%d]. CMD: [%s]\n", targworker, cmd.c_str() );
      exit(1);
    }
  
  //fprintf(stdout, "ROOT: handling finished work from worker [%d]. Getting INT\n", targworker);
  size_t nfiles = receive_int_from_worker( targworker, myworker );
  //fprintf(stdout, "ROOT: handling finished work from worker [%d]. Got INT (nfiles=[%ld])\n", targworker, nfiles);

  for(size_t x=0; x<nfiles; ++x)
    {
      memfile mf = receive_file_from_worker( targworker, myworker );
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
  //fprintf(stdout, "ROOT: handling finished work from worker [%d]. Finished receiving all files (nfiles=[%ld]). Now getting varlist\n", targworker, nfiles);

  varlist<std::string> outputvlist = receive_varlist_from_worker( targworker, myworker );
  //fprintf(stdout, "ROOT: handling finished work from worker [%d]. GOT varlist\n", targworker);
  
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
void filesender::execute_slave_loop( const int myworker, const std::string runtag )
{
  bool loopslave=true;
  std::string LOCALDIR = "/tmp/" + runtag + "_" + std::to_string( myworker );
  
  fprintf(stdout, "WORKER [%d]  (rank [%d], thread [%d]): GPU device is [%d] (local idx is %ld). Starting slave loop...\n", myworker, getworkerrank(myworker), getworkertag( myworker ), mygpuidx, mylocalidx );
  //REV: This will do nothing if there is no CUDA.

  set_cuda_device( mygpuidx ); 

  fprintf(stdout, "WORKER [%d]  (rank [%d], thread [%d]): GPU device is [%d]. SET GPU, now sending READY to root!\n", myworker, getrank(), getworkertag(myworker), mygpuidx );
  //REV: need to send first guy to tell its ready
  std::string pcinit = "READY";
  //send_cmd_to_root( pcinit, myworker );
  send<std::string>( pcinit, myworker, ROOTWORKER );

  fprintf(stdout, "WORKER [%d]  (rank [%d], thread [%d]): GPU device is [%d]. **DONE** sending READY to root. Now LOOPING!\n", myworker, getrank(), getworkertag(myworker), mygpuidx );
  
  while( loopslave == true )
    {
      make_directory( LOCALDIR );
      
      psweep_cmd cmd = receive_cmd_from_root( myworker );
      //fprintf(stdout, "WORKER [%d]  (rank [%d], thread [%d]): GPU device is [%d]. Got CMD from root!\n", myworker, getrank(), getworkertag(myworker), mygpuidx );
      
      
      if( cmd_is_exit( cmd ) == true )
	{
	  fprintf(stderr, "REV: WORKER [%d] received EXIT\n", myworker );
	  loopslave = false;
	  return;
	  //break;
	}

      //REV: may EXIT, or contain a PITEM (to execute).
      pitem mypitem = handle_cmd( cmd, myworker );
      //fprintf(stdout, "WORKER [%d]  (rank [%d], thread [%d]): GPU device is [%d]. Will handle pitem!\n", myworker, getrank(), getworkertag(myworker), mygpuidx );
	
      memfsys myfsys = worker_handle_pitem( mypitem, LOCALDIR, myworker);
      //fprintf(stdout, "WORKER [%d]  (rank [%d], thread [%d]): GPU device is [%d]. Finished handle PITEM, executing!\n", myworker, getrank(), getworkertag(myworker), mygpuidx );
      		
      bool blah = execute_work( mypitem, myfsys );

      //fprintf(stdout, "WORKER [%d]  (rank [%d], thread [%d]): GPU device is [%d]. Done executing, notifying of done...!\n", myworker, getrank(), getworkertag(myworker), mygpuidx );
      
      //this includes the many pieces of "notifying, waiting for response, then sending results, then waiting, then sending files, etc..
      worker_notify_finished( mypitem, myfsys, myworker );
      
      //fprintf(stdout, "WORKER [%d]  (rank [%d], thread [%d]): GPU device is [%d]. DONE notify, cleaning up.....!\n", myworker, getrank(), getworkertag(myworker), mygpuidx );
      
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
void filesender::master_to_slave( const pitem& mypitem, const int& workeridx, const int& myworker, memfsys& myfsys )
{
  //send_cmd_to_worker( "PITEM", workeridx, myworker );
  send<std::string>("PITEM", ROOTWORKER, workeridx );

  //send_pitem_to_worker( mypitem, workeridx, myworker );
  send<pitem>( mypitem, ROOTWORKER, workeridx );
  
  int nfiles = mypitem.required_files.size();
    
  //send_int_to_worker( nfiles, workeridx, myworker );
  send<int>( nfiles, ROOTWORKER, workeridx );

  //Then, send the files.
  for(size_t f=0; f<mypitem.required_files.size(); ++f)
    {
      //REV: Yea...just send files from here. Problem is, some of them might not be in MEMFSYS??? We should load them all at some point (required,
      //and input)
      memfile_ptr mfp = myfsys.open( mypitem.required_files[f], true ); //will attempt to read through if it does not exist. Note may be empty?
	
      //send_file_from_disk( workeridx, mypitem.required_files[f] );
      //send_file_to_worker( mfp.get_memfile(), workeridx, myworker );
      send<memfile>( mfp.get_memfile(), ROOTWORKER, workeridx );
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
      //fprintf(stdout, "Not all done of WPROG! WORKING: \n");
      /*for(size_t x=0; x<_workingworkers.size(); ++x)
	{
	  if( _workingworkers[x] == true )
	    {
	      fprintf(stdout, " [%ld] ", x );
	    }
	}
	fprintf(stdout, "\n");*/
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
	  /*if( wprog.avail_worker( _workingworkers ) == true )
	    {
	      fprintf(stdout, " NOTE: there ARE workers available\n");
	    }

	  if(  wprog.check_work_avail() == true  )
	    {
	      fprintf(stdout, " NOTE: there IS work available\n");
	      }*/
	  //ACCEPT MESSAGES FROM WORKERS AND HANDLE.
	  //Note, we want to keep it so that this array will carry over
	  //to the next loop??? Fuck. They sent DONE or READY or
	  //whatever, we said OK to that, and marked our guy NOT WORKING.
	  //Because of that, there is no more message from the
	  //WORKER. So, I need to remember which are which, i.e.
	  //pass around a WORKING thing separate from the list of PP.
	  //OK, do it.

	  
	  psweep_cmd pcmd = receive_cmd_from_any_worker( ROOTWORKER );
	  
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
	      varlist<std::string> result = handle_finished_work( pcmd, handledpitem, pg.parampoint_memfsystems[ pc.parampointn ], ROOTWORKER, usedisk );
		
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
			   ROOTWORKER,
			   pg.parampoint_memfsystems[pc.parampointn] );
	  //fprintf(stdout, "DONE master to slave.\n");
	}
    } //end while !all done.
  //fprintf(stdout, "\n\nROOT FINISHED GENERATION? (comp pp)\n\n");
} //end comp_pp_list

