

int MYRANK;


//So, depending on the RANK, this will SEND or RECEIVE.
//All SLAVES enter a LOOP of waiting for a MESG.

//void rename_file( pitem& mypitem, const std::vector<mem_file>& mf, const std::vector<std::string> newfnames, const std::string& dir )
//This will rename all files in MYPITEM (in SUCCESS only?), given a list of files? An array of files? Yea, it will tell which ones should be renamed.
//I.e. gives list of indices to it.

/* keep empty, i.e. do I want to know when it is e.g. :: or //? */
std::vector<std::string> tokenize_string(const std::string& source, const char* delim, bool include_empty_repeats=false)
{
  std::vector<std::string> res;
  
  /* REV: size_t is uint? */
  size_t prev = 0;
  size_t next = 0;

  /* npos is -1 */
  while ((next = source.find_first_of(delim, prev)) != std::string::npos)
    {
      if (include_empty_repeats || ((next-prev) != 0) )
        {
	  res.push_back(source.substr(prev, (next-prev)) );
        }
      prev = next + 1;
    }

  /* REV: push back remainder if there is anything left (i.e. after the last token?) */
  if (prev < source.size())
    {
      res.push_back(source.substr(prev));
    }

  return res;
}

//compare two filenames in canonical thing? E.g. /.././../. etc. /. will remove current, i.e. can be safely removed. /.. will remove the previous
//thing (if it starts with that /.. then error out). Can't handle things like symlinks anyway so whatever...

//REV: BIG PROBLEM, I need to check whether it starts with a / or not.

std::string get_canonical_dir_of_fname( const std::string& s, std::string& fnametail )
{
  std::stack<std::string> fnstack;

  //Only works on LINUX/UNIX? That begin with root assuming /
  //Otherwise, windows would be like C:// etc.
  bool isglobal=false;
  
  //First, remove all whitespace? No, don't.
  if( s[0] == '/' )
    {
      isglobal = true;
    }
  
  //first, tokenize it by /  (how to handle first one?)
  std::vector<std::string> vect = tokenize_string( s, '/');
  
  //Then, iterate through from beginning, push back thing to a stack. If .., pop the stack. If stack size < 0, error.
  for(size_t x=0; x<vect.size(); ++x)
    {
      if( vect[x].compare("..") == 0 )
	{
	  fnstack.pop();
	}
      else if( vect[x].compare(".") == 0)
	{
	  //do nothing (just remove it, no need for something that specifies "same" directory
	}
      else
	{
	  fnstack.push( vect[x] );
	}
      
    }
  std::vector<std::string> ret( stack.size() );
  if(stack.size() < 1)
    {
      fprintf(stderr, "REV: ERROR, trying to get dir of a file(name) that is HERE\n");
    }
  for(size_t x=0; x<ret.size()-1; ++x)
    {
      ret[ x ] = fnstack.top();
      fnstack.pop();
    }

  
  std::string finalstring =  CONCATENATE_STR_ARRAY( ret, "/" );

  if(isglobal)
    {
      finalstring = "/" + finalstring;
    }

  return finalstring;
    
  //Ending is the filename or dir name. Note remove all double or multiple slashes // etc.
    
}

std::string canonicalize_fname( const std::string& s )
{
  std::stack<std::string> fnstack;

  bool isglobal=false;
  
  //First, remove all whitespace? No, don't.
  if( s[0] == '/' )
    {
      isglobal = true;
    }
  
  //first, tokenize it by /  (how to handle first one?)
  std::vector<std::string> vect = tokenize_string( s, '/');

  
  
  //Then, iterate through from beginning, push back thing to a stack. If .., pop the stack. If stack size < 0, error.
  for(size_t x=0; x<vect.size(); ++x)
    {
      if( vect[x].compare("..") == 0 )
	{
	  fnstack.pop();
	}
      else if( vect[x].compare(".") == 0)
	{
	  //do nothing (just remove it, no need for something that specifies "same" directory
	}
      else
	{
	  fnstack.push( vect[x] );
	}
      
    }
  std::vector<std::string> ret( stack.size() );
  for(size_t x=0; x<ret.size(); ++x)
    {
      ret[ x ] = fnstack.top();
      fnstack.pop();
    }

  //Will this work, cast to vector?
  std::string finalstring =  CONCATENATE_STR_ARRAY( ret, "/" );

  if(isglobal)
    {
      finalstring = "/" + finalstring;
    }

  return finalstring;
  
    
}
std::string canonicalize_fname( const std::string& s )
{
  std::stack<std::string> fnstack;

  bool isglobal=false;
  
  //First, remove all whitespace? No, don't.
  if( s[0] == '/' )
    {
      isglobal = true;
    }
  
  //first, tokenize it by /  (how to handle first one?)
  std::vector<std::string> vect = tokenize_string( s, '/');

  
  
  //Then, iterate through from beginning, push back thing to a stack. If .., pop the stack. If stack size < 0, error.
  for(size_t x=0; x<vect.size(); ++x)
    {
      if( vect[x].compare("..") == 0 )
	{
	  fnstack.pop();
	}
      else if( vect[x].compare(".") == 0)
	{
	  //do nothing (just remove it, no need for something that specifies "same" directory
	}
      else
	{
	  fnstack.push( vect[x] );
	}
      
    }
  std::vector<std::string> ret( stack.size() );
  for(size_t x=0; x<ret.size(); ++x)
    {
      ret[ x ] = fnstack.top();
      fnstack.pop();
    }

  //Will this work, cast to vector?
  std::string finalstring =  CONCATENATE_STR_ARRAY( ret, "/" );

  if(isglobal)
    {
      finalstring = "/" + finalstring;
    }

  return finalstring;
  
    
}

bool same_fnames(const std::string& fname1, const std::string& fname2)
{
  std::string s1 = canonicalize_fname( fname1 );
  std::string s2 = canonicalize_fname( fname2 );
  if( fname1.compare( fname2 ) == 0 )
    {
      return true;
    }
  else
    {
      return false;
    }
}


void replace_old_fnames_with_new( std::vector<std::string>& fvect, const std::string& newfname, const std::vector<size_t> replace_locs )
{
  for(size_t x=0; x<replace_locs.size(); ++x)
    {
      fvect[ replace_locs[ x ] ] = newfname;
    }
  
  return;
}

//This only matches EXACT filenames
std::vector<size_t> find_matching_files(const std::string& fname, const std::vector<std::string>& fnamevect, std::vector<bool>& marked )
{
  std::vector<size_t> foundvect;
  for(size_t x=0; x<fnamevect.size(); ++x)
    {
      std::string canonical = canonicalize_fname( fnamevect[x] );
      //if( strcmp( fnamevect[x], fname ) == 0 )
      if( same_fnames( canonical, fname ) == true )
	{
	  if(marked[x] == false)
	    {
	      foundvect.push_back( x );
	      marked[x] = true;
	    }
	  else
	    {
	      fprintf( stderr, "REV: WARNING in find_matching_filenames: RE-CHANGING already MARKED file (i.e. circular renaming?) File: [%s]\n",
		      fname.c_str() );
	    }
	}
      
      
    }
  return foundvect;
}

void rename_file( pitem& mypitem, std::string fname, std::string origdir, std::string newdir )
{
  
}

pitem modify_pitem( const pitem& mypitem, const std::vector<mem_file>& mf,
		    const std::vector<std::string> newfnames,
		    const std::string& dir )
{
  //1) REQUIRED file list (these will all be copied from source location to MYDIR/required_files and renamed as well). We keep that "renamer" around.
  //2) SUCCESS file list (these will all be transferred to MYCMD, no renaming as some might be hardcoded in user program? In which case if DIR doesn't exist
  // and we can't specify it, it will error out -- so we will check to make sure it is forced to be in user dir). We copy these back.
  //3) OUTPUT file list (these must be inside MYDIR or else error, so I will rename them). We copy these back (and read out?)
  //4) INPUT file (name) just a single one (rename this and specify, inside MYDIR).
  //5) MYDIR string (just a single one) -- will modify to user.
  //6) MYCMD, a vector of strings, will be concat with some "SEP" at the end. Find all of the above changed guys, stringmatch, and change to new updated
  //      guys. Note, change to some canonical form first, to handle stuff like ../blah versus /local/blah, etc. I.e. use PWD, etc. Do that later.

  
  
}

void handle_pitem( pitem& mypitem, const std::string& dir )
{
  //First, get an INT, number of files.
  int numfiles = get_int( ROOT_RANK );
  std::vector< mem_file > mfs;
  std::vector< std::string > newfnames;
  std::vector< std::string > oldfnames;
  
  std::string fnamebase= "reqfile";
  for(size_t f=0; f<numfiles; ++f)
    {
      mem_file mf = receive_file( ROOT_RANK );
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

struct mem_file
{
  std::string fname;
  std::vector<char> contents;

  mem_file( const std::string& fn, const std::vector<char> cont )
  : fname( fn ), contents (cont )
  {
    //REV: NOTHING
  }

  void tofile(const std::string& dir, const std::string& fname )
  {
    std::string fullfname = dir + "/" + fname;

    ofstream ofs;

    open_ofstream( fullfname, ofs, );
    
    ofs.write( contents.data(), contents.size() );

    //REV: does it work? Need to check sanity before I go?
  }
};


mem_file receive_file( const int& targrank )
{
  //REV: crap I can't just cast to a string, I need to CONSTRUCT it.
  std::vector<char> fname = receive_memory( targrank );
  std::string fn = std::string( fname.data() );

  std::vector<char> fcontents = receive_memory( targrank );

  mem_file mf(  fn, fcontents );

  return mf;
}


pitem get_pitem_from_targ_rank( const int& targrank )
{
  std::vector< char > tmpmem = receive_memory( targrank );
  
  pitem tmppitem = cast_to_type< pitem >( tmpmem );

  return tmpitem;
}

//arbitrary byte array as VECTOR, and we can cast it to a target TYPE in some way? Like...static_cast? whoa...blowin my mind haha.
std::vector< char > receive_memory( const int& targrank )
{

  std::vector< char > mem;
  
  MPI_Status s;
  MPI_probe(targrank, MPI_ANY_TAG, MPI_COMM_WORLD, &s);
  
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

template <typename T>
T cast_to_type( std::vector< char > membuff )
{
  //Note, I am COPYING it in this case? meh.
  T tmpitem = (T)membuff.data();
  return tmpitem;
}

int get_int( const int& targrank )
{
  std::vector<char> mem = receive_memory( targrank );

  int myint = cast_to_type< int >( mem ); //int POINTER?? wtf? Ugh.

  return myint;

  
}


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



pitem handle_cmd( psweep_cmd pcmd )
{
  //get the CMD from it, return...
  if( pcmd.SRC != ROOT_RANK )
    {
      fprintf(stderr, "ERROR worker [%d] got cmd from non-root rank...[%d]\n", MYRANK,  pcmd.SRC);
      exit(1); //EXIT more gracefully?
    }

  if( strcmp( pcmd.CMD, "EXIT") == 0 )
    {
      //exit
      fprintf(stderr, "Worker [%d] received EXIT command from root rank\n", MYRANK);
      exit(1);
    }
  else if( strcmd( pcmd.CMD, "PITEM") == 0 )
    {
      //will process this pitem.
      pitem mypitem = get_pitem_from_targ_rank( ROOT_RANK );

      return mypitem;
    }
  else
    {
      fprintf(stderr, "ERROR, rank [%d] received unknown command [%s]\n", MYRANK, pcmd.CMD );
      exit(1);
    }
}

void execute_slave_loop()
{
  //WAIT for mesg from MASTER
  psweep_cmd cmd = wait_for_cmd();
  
  pitem myitem = handle_cmd( cmd ); //REV: may EXIT, or contain a PITEM (to execute).

  std::string LOCALDIR = "/TMP/scratch"; //will execute in local scratch. Note, might need to check we have enough memory etc.
  
  handle_pitem( mypitem, LOCALDIR ); // This will do the receiving of all files and writing to LOCALDIR, it will also rename them and keep track
  //of the correspondence. Note the SENDING side will do the error out if it is no in the __MY_DIR. Do that on PARENT side I guess. OK.

  //We now have PITEM updated, we will now EXECUTE it (until it fails, keep checking success).

  execute_work( mypitem );
  
  notify_finished(); //this includes the many pieces of "notifying, waiting for response, then sending results, then waiting, then sending files, etc..
  
  
}

void execute_master_loop()
{
  //
}



//REV: 17 Jan 2016. Header/functions to handle
//sending of file via MPI. We can separate MPI vs Hadoop etc.
//


//Goal is to go from PARAMPOINT to PARAMPOINT on other side. Transformation is
//LOCATION (e.g. locally it has /TMP as its guy, we want to pass EVERYTHING?)
//Or only output? We don't care about e.g. local outputs? Copy everything?


//It's only a single PITEM though! So we only need to copy that PITEM stuff, i.e. the base dir of the PITEM that I just computed.
//I iterate through the directory and copy over all the files, or only the output files? Much easier to only copy the output files.
//But, in that case I will lose STDIN/STDOUT etc. However, I can artificially add "STDIN" and "STDOUT" to "success" files even if nothing will
//be printed to them? Whatever, fine. What will the guy output? It is part of the blah? I don't know it might output some other files I don't know
//about or don't care about? Like network structure etc. Whatever force user to specify it all for now, for ease.

//Only grab those files. We need to know what their corresponding location at target is. We will copy them all to "this guys" directory on the
//master host .

//So, user specifies "required" files and "success" files. Success files will ALWAYS be relative to source? Or can it force it to produce elsewhere?
//Assume they are always relative to source of "this" active pitem? Need way of getting "necessary" guys from previous psets. That's fine but whatever.
//We need to modify those guys too (remember!). I.e. their location is not local, but based in /TMP instead of elsewhere in filesystem...
//Anyway, somehow we have those values set? Not generated, but set?

//What are actual expected use scenarios?

//In MASTER
//1) I have "required" files in "data" folder somewhere on the system
//2) I have "required" files in of a PREVIOUS pset in multiple folders
//3) I have "required" files in the CURRENT directory (?)

//These need to all be copied appropriately, and references to them changed to cope with SLAVEs expected locations.

//Literally they are just "success" files. The best way is to probably give them individual names so we know their correspondences. Easiest.
//However, I do not expect to be overwriting any files. Thus, it is easiest to just copy back. Of course DATA guys are um, annoying. Need to change
//only their directories, not filenames? How to deal with DATA etc? Yea, give it a way to copy reused data files at beginning. And refer to them
//that way. I.e. data files won't be recopied. At any rate, we go through REQUIRED files, taking their current location (always referenced from
//parampoint base?!?!?! or else we have problems. Assume user doesn't overwrite them?) Need to re-build the whole parampoint guy ON THE OTHER SIDE,
//using a different base. That makes it much easier. Assume I didn't move the files. So, then the only thing I need to do is copy to corresponding
//location? But in old pset, we don7t know their old or new locations? So, only option is correspondingly copy it? If we know it will be old pset guy,
//copy it over somehow? We need to give it "new" location, always offset from base? Just take relative to base if it is in our dir, and put it in
//corresponding dir over here? Pain in the butt to re-build a dir hierarchy each time..?? Waste of resources... better to change filenames?
//And put them all in one place? I guess... Any reason to have full guy? have "data" target, and "required" files all in one place? But if they
//are all named "output"? Easy, just check of name collision. We need to know where that happens in the current "CMD" though. So we change the
//corresponding filename in the generated guy? All required files will be where? Easier to rerun build of paramset. Even if we send it, we need
//to tell it what name it was? Might have filenames hardcoded into variables or something in the PARAMPOINT generator, or as an array, in which
//case we're fucked? No, they can't be, they must be single ones in REQUIRED/SUCCESS, so I can move them over. However, we might refer to them differently?
//In which case we have to change all their other references, unless we always refer to them "through" success file array. Which is not what happens.
//So, yappa, I have to rebuild it...with a different base dir. And when I copy it over, I need to tell it relative thing to base for non-data files,
//and just leave data files as are I guess. Can't require anything outside of that I guess... OK go.

//REV: JUST IMPL IT

//In SLAVE
//After completion I have "success" files (output files?) in the TMP dir. Mostly (probably) in the current PITEM guy.

//These need to be copied appropriately back to MASTER




//So, I literally send the CONSTRUCTION method? And re-construct the required guys with a different file base I guess...



//So, some guys will have references there, but most guys won't. We only make the dir. User might specify success files manually
//in global filesystems etc. Best case is to simply ignore all of that, rename them etc., and put them in a normal way. Problem is that
//all references to them must also be changed. I.e. user added file /BLAH/BLEEP/BLOO to REQUIRED_FILES, but he also has in CMD,
//something like --INPUT /BLAH/BLEEP/BLOOP. Since he's not using SUCCESS_FILES array to refer to it, it won't be picked up, and it's
//just a string anyways. So, best to copy to same place in FS.

//Problem is same place might not exist here. Problem is replacing references to the files in other places than REQUIRED_FILES etc..
//E.g. if they are coded as tokens that is a massive problem...
//I could literally string search for it, or for user to specify that it is a FILE string via some wrapper function (that will mark it some how?)?

//That could end up being nasty, but seems like the best way to do things...
//If it's memory, of course we'll just send it straight I guess...ugh. Worry about that later...
//Make all "input" files references? Nice...i.e. whenever we reference a file, we have to tell that is an expected file...
//but what if we build it/construct it? Add a vartype that is FILE?
//I.e. we CAN add a file to SUCCESS files inside it. I like that. I.e. we have to tell it when we are constructing a file. Great ;)
//I.e. instead of ADD_SUCCESS_FILE, we always just refer to it? We can use it and it returns the string name (as a variable) but it also pushes it back.
//Note success files might then have duplicates if it is done many times?

//How do we remove duplicates? Allow user to "name" the files as variables? File variables?


//This ensures that if we hit all the guys, we can move them all appropriately. But it also means that we need to rename all guys from that
//file location to the new file location when we re-build the parameter point. Pain in the butt... I.e. we just replace the user's stuff inside that
//function with a literal? That is a problem... because we have to totally resolve the string before we replace it. Not tenable...let's forget this.

//literally replace all "file like" references? I.e. if we find "success_file" string anywhere, we replace it. Problem is if e.g. user program has
//an option that happens to be named same as a required file haha. Like --recfile ./recfile (worst case). Forget that?? Only do it if separated by
//a sep? Seems excessive. Ah, search inside individual variables as we go, seems the most realistic? But what if a var is the subset? So, wait until
//each STMNT is finished, then replace inside it I guess? Yea, seems best way to do it. As each statement is re-realized. We give it correspondence
//list of original file and target file. But as we reconstruct the PITEM, we have a major problem as it will try to create the directory? I.e. we
//have to have it build the original guy in real time? Giving it a false basedir. That seems not-good haha.

//Oh shit, I go up to the pset above me and get the dir...? This is a major problem. So, I can't just re-produce. I need to literally just swap out
//individual references in the completed pset without re-executing. In other words, in the final CMD? Remember, CMD is a list of the guys, so I just need
//to check individual variables?
