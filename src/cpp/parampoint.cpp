
#pragma once

#include <parampoint.h>



void seedgen::seed( const long& seedval )
  {
    //seq = std::seed_seq( {seedval} );
    reng.seed(seedval);
  }

  //Hopefully it keeps state...?
  //REV: Generate random int in here from our chain and use it as seed...not very beautiful, but same as doing seeds(1)
std::uint32_t seedgen::nextseed()
  {
    std::uniform_int_distribution<std::uint32_t> mydistr(0, 666666666);
    std::vector<std::uint32_t> seeds(1);
    seq = std::seed_seq( { mydistr( reng ) } );
    seq.generate(seeds.begin(), seeds.end());
    
    //fprintf( stdout, "GENERATED SEED: [%d]\n", seeds[0] );

    return seeds[0];
  }


parampoint_coord::parampoint_coord()
  {
  }
  
parampoint__coord::parampoint_coord( const size_t& pp, const size_t& ps, const size_t& pi )
: parampointn(pp), psetn(ps), pitemn(pi)
  {
    //REV: nothing to do.
  }

  


pset_functional_representation::pset_functional_representation( client::PSET& p )
  {
    myname = p.NAME;
    pset_width = std::stoul( p.NREP );
    for( size_t s=0; s<p.CONTENT.size(); ++s )
      {
	functrep tmpfr(p.CONTENT[s], myfl);
	frlist.push_back( tmpfr );
      }
    
    //frlist now has the recursive functional representations that we can call each of frlist[f].execute( vectofHIERCH, myHIEARCHindex)
  }


   template<class Archive>
   void pitem::serialize(Archive & ar, const unsigned int version)
  {
    ar & mycmd;
    ar & required_files;
    ar & success_files;
    ar & output_files;
    ar & input_file;
    ar & mydir;
  }
  
  //In new version, this may call from memory to speed things up.
bool pitem::execute_cmd( fake_system& fakesys, memfsys& myfsys )
  {
    //fprintf(stdout, "Attempting to check ready in execute cmd\n");
    std::vector< std::string > notready = check_ready( myfsys );
    //fprintf(stdout, "FINISHED to check ready in execute cmd\n");

    //FIRST, TRY TO CALL IT WITH USER THING.
    //REV: user might be trying to call a script with python or something...? E.g. pythong SCRIPT go...? In which case we better not find it lol.
    std::string ostensible_cmd = mycmd[0]; //Hopefully it doesn't start with a ./ etc.? We can correct that to check heh.
    //std::vector<std::string> args = std::vector<std::string>( mycmd.begin()+1, mycmd.end() );
    //bool calledfake = fakesys.call_funct( ostensible_cmd, args );
    //REV: 10 Feb 2016 -- make it same as ARGV so user doesn't have to change much...
    //fprintf(stdout, "Attempting to call FAKESYS mycmd\n");
    bool calledfake = fakesys.call_funct( ostensible_cmd, mycmd, myfsys );
    //fprintf(stdout, "SUCCESSFULLY to call FAKESYS mycmd\n");
    
    if( calledfake == false )
      {
	//fprintf(stdout, "THINK I **DIDNT** call a fakesystem, i.e. will attempt real system()\n");
	std::string stderrfile = mydir+"/stderr";
	std::string stdoutfile = mydir+"/stdout";
	mycmd.push_back( "1>"+stdoutfile );
	mycmd.push_back( "2>"+stderrfile );
	
	
	std::string sep = " ";
	//std::string execute_string = mycmd.CONCATENATE_STR_ARRAY( cmdarray, sep );
	std::string execute_string = CONCATENATE_STR_ARRAY( mycmd, sep );
	
	if( notready.size() > 0 )
	  {
	    fprintf(stderr, "REV: Error in executing of command:\n[%s]\n", execute_string.c_str());
	    fprintf(stderr, "found [%ld] NON-EXISTENT REQUIRED FILES: ", notready.size() );
	    for(size_t f=0; f<notready.size(); ++f)
	      {
		fprintf(stderr, "FILE: [%s]\n", notready[f].c_str());
	      }
	    fprintf(stderr, "\n");
	    exit(1);
	  }

	//fprintf(stdout, "----EXECUTING [%s]\n", execute_string.c_str() );
	
	int sysret = system( execute_string.c_str() );
	
	std::vector<std::string> notdone = checkdone( myfsys );

	size_t tries=0;
	size_t NTRIES=10;
	while( notdone.size() > 0 && tries < NTRIES)
	  {
	    sysret = system( execute_string.c_str() ); //Delete/reset this pitem?
	    notdone = checkdone( myfsys );
	    ++NTRIES;
	  }

	if(notdone.size() > 0)
	  {
	    fprintf(stderr, "REV: Error in executing of command:\n[%s]\n", execute_string.c_str());
	    fprintf(stderr, "found [%ld] NON-EXISTENT SUCCESS FILES: ", notdone.size() );
	    for(size_t f=0; f<notdone.size(); ++f)
	      {
		fprintf(stderr, "FILE: [%s]\n", notdone[f].c_str());
	      }
	    exit(1); //Just exiting now...but meh.
	    return false;
	  }

	//More elegantly exit. Note all guys should output to a certain specific output file!!! In their dir.
      }
    return true;
    //this only tells if it was SUCCESSFUL or not (?). For example we had problems before due to something failing for one reason or another. In this way
    //we can automatically restart it...?
  }
  

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

void pitem::rename_to_targ( std::vector<std::string>& targvect, std::vector<bool>& marked, const std::string& olddir, const std::string& newdir,
		       const std::vector<std::string>& oldfnames, const std::vector<std::string>& newfnames )
  {
    if( newfnames.size() != targvect.size() || newfnames.size() != oldfnames.size() )
      {
	fprintf(stderr, "ERROR newfnames or oldfnames != targvect.size\n");
	exit(1);
      }

    //Note, NEWFNAMES will be the list we pushed back to of new fnames. They will be not set in NEWDIR, we need to add the NEWDIR to them? NO, THEY ARE
    //SET IN THE NEWDIR!!!

    std::vector<bool> reqmarked( targvect.size(), false );
    
    for(size_t x=0; x<oldfnames.size(); ++x)
      {
	//For all guys, I already know the ORIGINAL and NEW fnames. Specifically ORIGINAL should still be in TARGVECT (?) We'll replace them one by one.
	std::string fname = canonicalize_fname(oldfnames[x]);

	//locate it in cmd...if it exists...
	std::vector<size_t> reqmatched=find_matching_files(fname, targvect, reqmarked); //will canonicalize fnames in MYCMD as they go.
	std::vector<size_t> cmdmatched=find_matching_files(fname, mycmd, marked); //will canonicalize fnames in MYCMD as they go.

	std::string newfname = canonicalize_fname( newdir + "/" + newfnames[x] );

	//fprintf( stdout, "NEW FNAME IN RENAME-to-TARG: new fname [%s] (dir=[%s])\n", newfname.c_str(), newdir.c_str());
	replace_old_fnames_with_new( targvect, newfname, reqmatched ); //replaces mycmd inline.
	replace_old_fnames_with_new( mycmd, newfname, cmdmatched ); //replaces mycmd inline.
	
      }

    bool alldone=true;
    for(size_t x=0; x<reqmarked.size(); ++x)
      {
	if( reqmarked[x] == false )
	  {
	    alldone = false;
	  }
      }
    if( alldone == false )
      {
	fprintf(stderr, "ERROR, not ALLDONE after doing RENAMING OF FILES, not all (REQUIRED FILES) were renamed, some are missing from newfnames/oldfnames?\n");
	exit(1);
      }

  }

void pitem::rename_str_to_targ( std::string& targstr, std::vector<bool>& marked, const std::string& olddir, const std::string& newdir,
		       const std::vector<std::string>& oldfnames, const std::vector<std::string>& newfnames )
  {
    if( newfnames.size() != oldfnames.size() )
      {
	fprintf(stderr, "ERROR newfnames !=  oldfnames \n");
	exit(1);
      }

    //Note, NEWFNAMES will be the list we pushed back to of new fnames. They will be not set in NEWDIR, we need to add the NEWDIR to them? NO, THEY ARE
    //SET IN THE NEWDIR!!!
    
    std::vector<bool> reqmarked = {false};

    std::vector<std::string> dummyvect = { targstr };
    
    for(size_t x=0; x<oldfnames.size(); ++x)
      {
	//For all guys, I already know the ORIGINAL and NEW fnames. Specifically ORIGINAL should still be in TARGVECT (?) We'll replace them one by one.
	std::string fname = canonicalize_fname(oldfnames[x]);

	//fprintf(stdout, "Checking match: targ [%s] vs OLD fname [%s]\n", targstr.c_str(), oldfnames[x].c_str() );
	
	//locate it in cmd...if it exists...
	std::vector<size_t> reqmatched=find_matching_files(fname, dummyvect, reqmarked); //will canonicalize fnames in MYCMD as they go.
	std::vector<size_t> cmdmatched=find_matching_files(fname, mycmd, marked); //will canonicalize fnames in MYCMD as they go.

	
	
		
	
	if(reqmatched.size() > 0)
	  {
	    std::string newfname = canonicalize_fname( newdir + "/" + newfnames[x] );
	    //fprintf(stdout, "FOUND MATCH!!!! New fname: [%s]  (newdir: [%s]\n", newfname.c_str() , newdir.c_str());
		    
	    //found it, rename it.
	    targstr = newfname;
	    //replace_old_fnames_with_new( targvect, newfname, reqmatched ); //replaces mycmd inline.
	    replace_old_fnames_with_new( mycmd, newfname, cmdmatched ); //replaces mycmd inline. Was already replaced in REQ so no need.
	  }
	
      }

    bool alldone=true;
    for(size_t x=0; x<reqmarked.size(); ++x)
      {
	if( reqmarked[x] == false )
	  {
	    alldone = false;
	  }
      }
    if( alldone == false )
      {
	fprintf(stderr, "ERROR, not ALLDONE after doing RENAMING OF FILES, not all (REQUIRED FILES) were renamed, some are missing from newfnames/oldfnames?\n");
	exit(1);
      }

  }

void pitem::rename_str_to_targ_dir( std::string& targ, std::vector<bool>& marked, const std::string& olddir, const std::string& newdir )
  {
    
    std::string fnametail;

    //REV: oops I fucked up, I need to only change the DIR for these!
    std::string dirname = get_canonical_dir_of_fname( targ, fnametail );

    if( dirname.compare( canonicalize_fname( olddir ) ) != 0 )
      {
	fprintf(stderr, "in REBASE DIRECTORY: ERROR: olddir and found dir in SUCCESS file are not the same!!! WTF target: [%s], old: [%s] (targ: [%s], newdir: [%s], fnametail: [%s]\n", dirname.c_str(), olddir.c_str(), targ.c_str(), newdir.c_str(), fnametail.c_str());
	
	exit(1);
      }
	
    std::string fname = canonicalize_fname(targ);
    //locate it in cmd...if it exists...
    
    std::vector<size_t> matched=find_matching_files(fname, mycmd, marked);

    std::string newfname = newdir + "/" + fnametail;
    //fprintf(stdout, "In renamed_str_to_targ_dir, about to replace with [%s]\n", newfname.c_str());
    replace_old_fnames_with_new( mycmd, newfname, matched ); //replaces mycmd inline.

    //Now replace SUCCESS itself.
    targ = newfname;
      

  }
  
void pitem::rename_to_targ_dir( std::vector<std::string>& targvect, std::vector<bool>& marked, const std::string& olddir, const std::string& newdir )
  {
    for(size_t x=0; x<targvect.size(); ++x)
      {
	rename_str_to_targ_dir( targvect[x], marked, olddir, newdir );
      }
    
  }
  
  //REV: For success etc. check existence AND THAT IT IS A FILE!!!! I.e. we don't do DIRS (for now)
void pitem::re_base_directory( const std::string& olddir, const std::string& newdir, const std::vector<std::string>& oldfnames, const std::vector<std::string>& newfnames )
  {
    std::vector<bool> marked(mycmd.size(), false);

    rename_to_targ_dir(success_files, marked, olddir, newdir);//, oldfnames);
    rename_to_targ_dir(output_files, marked, olddir, newdir);//, oldfnames);
    
    //REV: Input file is just a string, not vector<string>, so need to have a similar function to just do single file...And modify underlying string.
    //REV: BY DEFAULT, input will be named as "REQUIRED". I should just rename it that way...
    //rename_str_to_targ_dir(input_file, marked, olddir, newdir);//, oldfnames);
    //REV: HERE. Note clear what to do. There is simply ONE old/new, which is input_file and how it was renamed specifically. So, I need to go through
    //it as if it was a CMD or required_files, and replace all renaemd references there. We determined old/new fnames from REBASE. So I need to make
    //sure to change INPUTFILE too? I.e. I need to be aware of changing INPUT file name. OK.
    //fprintf(stdout, "ORIG INPUT: [%s]\n", input_file.c_str());
    
    
    rename_to_targ(required_files, marked, olddir, newdir, oldfnames, newfnames);
    //bool marked1=false;
    
    rename_str_to_targ(input_file, marked, olddir, newdir, oldfnames, newfnames);//, oldfnames);

    //fprintf(stdout, "REBASED INPUT: [%s]\n", input_file.c_str());
    
    //All MARKED not necessarily, because it might contain non-files in the string vector representing the command.
    
  }

pitem::pitem( )
  {
  }
  
pitem::pitem( pset_functional_representation& pfr, const size_t idx,  hierarchical_varlist<std::string>& hv, memfsys& myfsys, const std::uint32_t& myseed, const bool& usedisk=false )
  {
    //Need to add to the most recent pset a child...
    std::vector<size_t> rootchildren = hv.get_children( 0 );
    size_t npsets = rootchildren.size();
    varlist<std::string> emptyvl;
    size_t myidx = hv.add_child( rootchildren[npsets-1],  emptyvl); //add child to the last PSET (!!)
    size_t mychildnum = hv.get_children( rootchildren[npsets-1] ).size()-1;
    
    my_hierarchical_idx = myidx;

    //Set up the required variables. Automatically name the things here (like choose dir based on sub of parent), much easier ;)

    std::string parentdir = hv.vl[ rootchildren[npsets-1] ].getvar( "__MY_DIR" ).get_s();
    mydir = parentdir + "/" + std::to_string( mychildnum ); //REV: whatever, to_string shouldn't cause a problem here, other than not padding zeroes.
    
    
    variable<std::string> var1( "__MY_DIR", mydir );

    std::vector<std::string> emptyvect;
    variable<std::string> var2( "__MY_REQUIRED_FILES", emptyvect );
    variable<std::string> var3( "__MY_SUCCESS_FILES", emptyvect );
    variable<std::string> var4( "__MY_OUTPUT_FILES", emptyvect );
    variable<std::string> var5( "__MY_CMD", emptyvect );
    variable<std::string> var6( "__MY_RANDSEED", std::to_string(myseed) );
    hv.vl[myidx].addvar( var1 );
    hv.vl[myidx].addvar( var2 );
    hv.vl[myidx].addvar( var3 );
    hv.vl[myidx].addvar( var4 );
    hv.vl[myidx].addvar( var5 );
    hv.vl[myidx].addvar( var6 );

    //Reserve __ vars internally or something? I'm not doing unrolling so whatever I guess.
    
    
        
    
    //Each must have for each worker the "required" files etc. Then it creates stuff and 
    bool succ = make_directory( mydir );
    
    
    std::vector<hierarchical_varlist<std::string>> hvl;
    hvl.push_back( hv ); //will it modify it? I'm not sure! Crap.
    //This is a copy operator, so it will not bubble up the copy operator
    //unless I re-copy
    //Add other HVarlist to it such as the global NAMES one.

    std::vector<size_t> idcs = {myidx};

    //REV: THIS is where it executes. Get INPUT and SUCCESS first, easier.
    
    
    //And now execute all arguments in pset_functional_rep
    for(size_t s=0; s<pfr.frlist.size(); ++s)
      {
	myvar_t retval = pfr.frlist[s].execute( hvl, idcs );
	//What to do with retval?!?!?! Ignore it?
      }

    
    //ghetto hack. GLOBAL namespace doesn't exist.
    hv = hvl[0];
    

    //Getting input file NAME to here.
    input_file = hv.get_val_var( "__MY_INPUT_FILE", my_hierarchical_idx );
    
    hv.add_to_var( "__MY_REQUIRED_FILES" , input_file, my_hierarchical_idx );


    output_files = hv.get_array_var( "__MY_OUTPUT_FILES", my_hierarchical_idx );

    for(size_t x=0; x<output_files.size(); ++x)
      {
	hv.add_to_var( "__MY_SUCCESS_FILES" , output_files[x], my_hierarchical_idx );
      }
    
    
    required_files = hv.get_array_var( "__MY_REQUIRED_FILES", my_hierarchical_idx );
    success_files = hv.get_array_var( "__MY_SUCCESS_FILES", my_hierarchical_idx );
    
    //std::vector<std::string> cmdarray = hv.get_array_var( "__MY_CMD", my_hierarchical_idx );
    mycmd = hv.get_array_var( "__MY_CMD", my_hierarchical_idx );
    
    //Add output to correct file.
    


    //REV: this was error, this should have been cmdarray before changing mycmd to be vector of strings.

    
    
        
    //std::string sep = " "; //spaces. Make user specify "cmd sep" if it exists or something?

    //mycmd = CONCATENATE_STR_ARRAY( cmdarray, sep ); //hv.get_val_var( "__MY_CMD", my_hierarchical_idx );
    
    
    
    
    //REV: I need to set INPUT files too, which I will write to from what? From HVL I guess? OK...
    //Will it do all of them, or only the first one?
    //We need a "setup" way or something. Note this is for every PITEM. I.e. it will re-write it each time, which is not good
    //Maybe only the FIRST one needs it. OK.
    
    //REV: ANOTHER OPTION, automatically copy it to desired filename in a dir?
    

    //fprintf(stdout, "XXXXXXX In PITEM const -- Set input_file to [%s]\n", input_file.c_str());
    
    //REV: here is the problem, it was outputting ALL local variables to the INPUT file of the user...crap.
    //hv.tofile( input_file, my_hierarchical_idx ); //HAVE TO DO THIS HERE BECAUSE I NEED ACCESS TO THE HV. I could do it at top level though...

    //REV: AH, I am outputting 0th HV index to input_file! This is why it is only the bottommost.
    //Ideally I should do each, leaving "most leaf-most" values of same guys as the same?

    
    //REV: HERE is final place to modify TODISK filesystem from filesystem. I need to set at creation time whether it should actually write
    //to something? I should store a memfsys internal to each, problem is that each PITEM will need to communicate with previous ones, which
    //will cause problems?

    //Zeroth is the base, i.e. root...the varlist that was passed that I built off of haha.
    //hv.tofile( input_file, 0, myfsys, usedisk ); //, my_hierarchical_idx ); //HAVE TO DO THIS HERE BECAUSE I NEED ACCESS TO THE HV. I could do it at top level though...
    //This will output ALL of my varlist...is this necessary? Ugh...
    //Note some of them may be ARRAY types, which will lead to problems reading the varlist from file (?)
    hv.tofile( input_file, myidx, myfsys, usedisk ); //, my_hierarchical_idx ); //HAVE TO DO THIS HERE BECAUSE I NEED ACCESS TO THE HV. I could do it at top level though...

    //fprintf(stdout, "YYYYYY Succeeded in setting (wrote to file?)\n");
    //Input files are REQUIRED files (by default, might be checked twice oh well).
    //Furthermore, output files are SUCCESS files (fails and tries to re-run without their creation of course).
    //required_files.push_back( input_file );

    /*for(size_t o=0; o<output_files.size(); ++o)
      {
	success_files.push_back( output_files[o] );
      }
    */
    //but then I need to do it for each ugh.
    //REV: note this will write EVERYTHING in here out, which may not be needed/might mess things up.
    //Can we specify only things that are tagged for this model or something?
    //Just do all for now, hope it doesn't break lol.
    
    //mydir = hv.vl[my_hierarchical_idx].getvar("MY_DIR").get_s();
    
        
    //Finished constructing pitem, i.e. setting all variables.
    //Now we go through and use those to set REQUIRED and SUCCESS files
    //and MYCMD, and MYDIR, etc.
    //From our specific SPECIAL VARIABLES. May be more than 1? Or will
    //be an array?
    //MM yea, set zero length array SUCCESS and REQUIRED from beginning
    //and also (empty) CMD string variable? OK, those are "set". Also, like
    //MYDIR. Functions will return those variables.
    //Need a way like "get_success_variable_name" or something? User
    //may want to SET them. OK.

    //I.e. at the end of this these REAL variables will be set.

    //We need ways of finding inside GLOBAL vars named var etc.
  }
  
  //REV: HERE, need to make them appropriately check!
  std::vector<std::string> check_ready( memfsys& myfsys )
  {
    std::vector<std::string> notreadylist;
    //bool ready=true;
    //Strings must be FULL filename? Or are they relative? Assume full...

    //REV: Check existence in memfsys!!
    for(size_t f=0; f<required_files.size(); ++f)
      {
	//fprintf(stdout, " # # # CHECKING FOR EXISTENCE OF REQUIRED FILE: [%s]\n", required_files[f].c_str());
	bool checkdisk=true;
	bool rdy= myfsys.check_existence( required_files[f], checkdisk );
	//fprintf(stdout, " X X X checked done\n");
	  
	if (rdy == false )
	  /*if( check_file_existence( required_files[f] ) == false ) */
	  {
	    notreadylist.push_back( required_files[f] );
	  }
	
      }
    return notreadylist;
    
    //checks if all required files are present?? etc.
  }
  
std::vector<std::string> pitem::checkdone( memfsys& myfsys )
  {
    std::vector<std::string> notdonelist;
    //Strings must be FULL filename? Or are they relative? Assume full...
    for(size_t f=0; f<success_files.size(); ++f)
      {
	//REV: Check existence in memfsys!!
	bool checkdisk=true;
	if( myfsys.check_existence( success_files[f], checkdisk ) == false)
	  //if( check_file_existence( success_files[f] ) == false )
	  {
	    //ready = false;
	    notdonelist.push_back( success_files[f] );
	  }
      }
    //REV: Ah, VARLIST does not reflect modifications to REQUIRED FILES/SUCCESS FILES etc....
    
    return notdonelist;
    
    //checks if I'm done. Specifically by seeing if all required guys are
    //finished. I need multiple "variable lists" of guys, some that
    //set required guys for starting, some that check required files for
    //ending. Don't check file contents though.
  }

  
varlist<std::string> pitem::get_output( memfsys& myfsys, const bool& usedisk=false )
  {
    varlist<std::string> retvarlist;
    
    //REV: I could read this some other way? Some number of (named) varlists? A list of them? OK.
    for(size_t f=0; f<output_files.size(); ++f)
      {
	retvarlist.inputfromfile( output_files[f], myfsys, usedisk );
      }
    return retvarlist;
  }

std::vector<std::string> pitem::get_cmd()
  {
    return mycmd;
  }
  


  //Constructs the list of each parampoint. Great.
functional_representation::functional_representation( client::PARAMPOINT& pp )
  {
    for( size_t ps=0; ps<pp.psets.size(); ++ps)
      {
	pset_functional_representation pfr( pp.psets[ ps ] );
	pset_list.push_back( pfr );
      }
  }

  functional_representation::functional_representation()
  {
  }
 

pset::pset(hierarchical_varlist<std::string>& hv)
  {
    //std::vector<size_t> rootchildren = hv.get_children(0);
    varlist<std::string> emptyvl;
    size_t myidx = hv.add_child( 0, emptyvl ); //add child to root;
    my_hierarchical_idx = myidx;

    size_t mychildnum = hv.get_children( 0 ).size()-1;

    //REV: get root's dir... i.e. my parent...
    std::string parentdir = hv.vl[ 0 ].getvar( "__MY_DIR" ).get_s();


    //Which PSET am I???
    mydir = parentdir + "/" + std::to_string( mychildnum ); //REV: whatever, to_string shouldn't cause a problem here, other than not padding zeroes.


    bool succ = make_directory( mydir );
    
    variable<std::string> var1( "__MY_DIR", mydir );
    hv.vl[myidx].addvar( var1 );
        
  }
  
  
  
void pset::add_pitem( const pitem& p )
  {
    pitems.push_back ( p );
    return;
  }
  
//REV: need a way to set that it is "done" too, i.e. go through and re-write back the files (?) to master, and check they're all there, and BAM.
//Furthermore, need to only give next pitem (next pset) when that is done.
//Anyway, need to rewrite this, or make a 3-rd party struct that keeps track of "done"-ness. Problem is if we check done-neess of previous but unneeded
//guys at local worker location, will fail because we just didnt copy files locally to TMP so be careful. Mm.
//Waste of time to look through ALL parampoints after gen 300k or something, so best to have a "start" point or something? That we periodically check.
//That makes the  most sense, not elegant though.


void parampoint::add_pset( const pset& myps)
  {
    psets.push_back( myps );
  }
  
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
parampoint::parampoint( hierarchical_varlist<std::string>& hv, std::string dirname )
  {
    mydir = dirname;
    variable<std::string> var1( "__MY_DIR", mydir );
    hv.vl[ my_hierarchical_idx ].addvar( var1 );
    //REV: OK, __MY_DIR is set after all!

    bool succ = make_directory( mydir );
  }


  

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




  
executable_representation::executable_representation()
  {
    //Do nothing, will reconstruct anyway.
  }
  
  //build the exec rep using config file(s) passed by user.
  //For now, force it to be a single file...
executable_representation::executable_representation( const std::string& script_filename )
  {
    //constructor. Constructs all the stuff, based on input script FILENAME?

    fprintf(stdout, "EXEC REPRESENT: trying to get contents from file [%s]\n", script_filename.c_str());
    std::string input = get_file_contents( script_filename  );

#if DEBUG>5
    fprintf(stdout, "GOT FILE CONTENTS FROM file [%s]; [%s]\n", script_filename.c_str(), input.c_str());
#endif
    
    if( input.compare("") == 0 )
      {
	fprintf(stderr, "Huh whacko error in reading of script in creating executable_representation: script is empty/script file didn't exist? [%s]\n", script_filename.c_str() );
	exit(1);
      }


    //construct the actual PARAMPOINT (config file representation?)
    ppscript = parse_psweep_script( input );

#if DEBUG > 1
    fprintf(stdout, "PARSED PARAMPOINT, got [%ld] PSETS\n", ppscript.psets.size());
    for(size_t a=0; a<ppscript.psets.size(); ++a)
      {
	fprintf(stdout, "PSET # [%ld]\n", a);
	enum_pset( ppscript.psets[a] );
      }
#endif
    
    //construct the executable representations based on that. I.e. construct for all PSETs inside it, the actual PSET things.
    fscript = functional_representation( ppscript );
    
  } //end constructor


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
parampoint executable_representation::build_parampoint( hierarchical_varlist<std::string>& hv, const std::string& dir, memfsys& myfsys, seedgen& sg, const bool& usedisk=false )
  {
    //REV: this will not work lol, I need one of my own?!??!!
    parampoint mypp( hv, dir );
    
    //make each pset...
    for(size_t p=0; p<fscript.pset_list.size(); ++p)
      {

	//This does the actual execution? Nah, but we might set DIR? Ah
	//Remember I control the DIR format, I need to decide how to
	//organize the DIRS of working etc.
	//This will add the child, and then set the my_hierarch_idx to
	//the correct newly added one.
	pset mypset(hv);
		    
	//I will now have a PARALLEL list of the pitems. But those are each one generated by a running of the whole script, which is what the
	//pset_list is (stmnt list).
	//So, I need to somehow locally generate/modify the local hierarch varlist to set e.g. INDEX NUMBER etc. Do it manually here? Based on where
	//I iterate through each of NUM_PITEMS or something?
	//I need to set

	//pset_list[p] holds the pset_funct_rep, which has members like X.pset_width, which is # of parallel guys to do for this. Use that to set
	//a variable (i.e. which "index" number am I?)? Need a way to set (in the config) to e.g. set to FILE_X.dat etc., X is my #. And need a way
	//to specify REQUIRED files as e.g. PSET_N/PITEM_X/FILE.dat, and it will resolve those variables to their actual directories. I can handle
	//that within my hierarchical thing, I go up to the PSET, go up, move over one, etc.. Do I know which "child" number each child is? Ah, that's
	//the problem. If we only care about "global" numbering that's fine.
	//Either way, need a "get my index" type thing.


	//Add child to HV here, set myidx to that added guy.
	for( size_t w=0; w<fscript.pset_list[p].pset_width; ++w )
	  {
	    //PITEM CONSTRUCTOR WILL EXECUTE THE GUYS IN ORDER TO
	    //GENERATE CMD?
	    //Constructor does the modification of HV?!?!!! OK.
	    //REV: This is where memfsys comes in.
	    pitem mypitem( fscript.pset_list[p], w, hv, myfsys, sg.nextseed(), usedisk);
	    mypset.add_pitem(mypitem);
	  }

	mypp.add_pset( mypset );
      }

    return mypp;

    //This will be the list of PSETS, and list of PITEM. And certain "special" variables will be set telling of required files, success files, directories
    //to run in, etc. I.e. at runtime, only CMD will run, all the other stuff is static.
    //Furthermore, "READ OUT" variables are specified by setting filenames that way. So, it will read those out and pass back to either main program or
    //whatever. Each PSET has possibility of returning? Nah, each PITEM even. But for fitness, it will return only from PPOINT!
    
    
    //What does this actually do? It doesn't return anything? It needs to actually execute everything and finally set the CMD variable. I.e. it modifies
    //all variables, etc. Can other things have side effects such as creating/removing directories, changing directories, etc.? Will the thing create
    //them automatically when it tries to go? What about stuff like setting /tmp?
    
    //Runs the function for the stored script and functional representation...
    //This will execute everything down to the CMD. It will also set all appropriate required variables etc.?
    //Ah, best idea is to have this generate lists of worker_functional_rep, which include e.g. SUCCESS and FAILURE. And then finally it will read
    //the CMD variable. Um, CALL it, with SET_CMD(blah). That is what will be executed later, using like "mydir" etc.

    //We need to know "which" thing to set it to. I.e. we need to "create" the individual guys, OK that's fine. We create them one at a time...and then
    //go execute them, checking success files of previous (no they are checked after), and required files of this time. And we make the guys to execute.
    //And we return the list of those. We need to make sure they're executed in order though, crap. *THAT* is what an executable representation is.
    //It will literally be farmed straight out to workers.

    //Ah, so we literally build our own set of worker representations in that order.
    
    //How about getting stuff out? We are setting some "specific" variables for this "sweep type", we need to tell it where those are. Or tell it to
    //read those into a special varlist? As results. I.e. this expects some specific variables to be set by this config file. To tell where the results
    //will come from/what those files will be named. Like "add results file". etc. They will always be in format of variables? And be read in that way?
    //name the files and read them in accordingly. Meh.
  }
};




pset_result::pset_result( const size_t& nitems )
  {
    pitem_results = std::vector< varlist<std::string> >( nitems );
  }


parampoint_result::parampoint_result( const parampoint& myp  )
  {
    for(size_t x=0; x<myp.psets.size(); ++x)
      {
	pset_results.push_back( pset_result( myp.psets[x].pitems.size() ) );
      }
  }






void parampoint_generator::set_result( const parampoint_coord& pc, const varlist<std::string>& result )
  {
    parampoint_results[ pc.parampointn ].pset_results[ pc.psetn ].pitem_results[ pc.pitemn ] = result;
    return;
  }

  varlist<std::string> parampoint_generator::get_result( const parampoint_coord& pc )
  {
    return  parampoint_results[ pc.parampointn ].pset_results[ pc.psetn ].pitem_results[ pc.pitemn ];
    
  }

  std::vector<varlist<std::string>> parampoint_generator::get_last_N_results( const size_t& N )
  {
    if( parampoints.size() < N )
      {
	fprintf(stderr, "ERROR in get last N: parampoints size too small [%ld] for N [%ld]\n", parampoints.size(), N);
	exit(1);
      }
    
    std::vector<varlist<std::string> > ret( N );
    for(size_t x=parampoint_results.size()-N; x<parampoint_results.size(); ++x)
      {
	if( parampoint_results.size() <= x )
	  {
	    fprintf(stderr, "Error parampoint results size too small in get last N (results size: [%ld], trying to get Xth [%ld]\n", parampoint_results.size(), x);
	    exit(1);
	  }
	if( parampoint_results[x].pset_results.size() == 0 )
	  {
	    fprintf(stderr, "ERROR parampoint results, PSET results size is 0\n" );
	    exit(1);
	  }
	if(parampoint_results[x].pset_results[ parampoint_results[x].pset_results.size()-1 ].pitem_results.size() != 1)
	  {
	    fprintf(stderr, "REV: In get last N results (parampoint results): Final PSET is of width != 1\n");
	    exit(1);
	  }
	ret[x] = parampoint_results[x].pset_results[ parampoint_results[x].pset_results.size()-1 ].pitem_results[0];
      }
    
    
    return ret;
    
  }

  varlist<std::string> parampoint_generator::get_result( const size_t& ppnum, const size_t& psetnum, const size_t& pitemn )
  {
    return  get_result( parampoint_coord( ppnum, psetnum, pitemn ) );
  }


  //REV: How to do this? User should specify how often to cleanup...
  //I'll leave it up to user to know when to clean it up.
  //Best to simply "delete" them, i.e. remove from beginning of vector..
  //Meh it's a vector but whatever.
  
  //REV: Might want to also clean up (write to HDF5 log) the stuff
  //from file system if it is saved there? Otherwise it will grow.
  //In other words, memfsystem? I.e. iteratively do parampoint.
  void parampoint_generator::cleanup_parampoints_upto( const size_t& ppidx )
  {
    //Note all arrays should be of same size...if not there's a problem..
    if( ppidx > parampoint_vars.size() )
      {
	fprintf(stderr, "ERROR, cleanup_parmpoints_upto, requested ppidx [%ld] > size of array [%ld]\n", ppidx, parampoint_vars.size() );
	exit(1);
      }


    //REV: .end() pointer should point PAST end of guys we are erasing. So this will including the +ppidx-1 guy I assume...?
    parampoint_vars.erase(parampoint_vars.begin(), parampoint_vars.begin()+ppidx );
    parampoints.erase(parampoints.begin(), parampoints.begin()+ppidx );
    parampoint_results.erase(parampoint_results.begin(), parampoint_results.begin()+ppidx );
    parampoint_memfsystems.erase(parampoint_memfsystems.begin(), parampoint_memfsystems.begin()+ppidx );

    if( parampoint_vars.size() != 0 )
      {
	fprintf(stderr, "REV: WARNING in cleanup parampoints upto: parampoint length is not 0! It is [%ld]\n", parampoint_vars.size() );
      }

    return;
  }

  
  void parampoint_generator::log_parampoints_upto( const size_t& ppidx)
  {
    //REV: Some regular way to handle these? force user to do something about it?
  }
  
  
  
  //Construct it by specifying global vars or something?

  parampoint_generator::parampoint_generator()
  {
    //do nothing, dummy constructor
  }
  
  parampoint_generator::parampoint_generator( const std::string& scriptfname, const std::string& bdir )
  {
    fprintf(stdout, "Making exec rep\n");
    exec_rep = executable_representation( scriptfname );

    fprintf(stdout, "FINISHED Making exec rep\n");
    
    basedir = bdir;
    make_directory( basedir ); //better already exist...
  }

  
  //takes an argument which is just a (new?) variable list of the parameters to test, and does stuff. Note some (other fixed parameters) might be global.
  //This ONLY generates, it is totally independent to sending stuff to be executed.

  //I would like to totally dissociate the farming out (of work) from the search function.
  //To make new param points, we do that by calling for new points. No, something is on top of that I guess. I.e. something calls the VL thing.
  //And then farms out the work (inside this structure?) until it is done
  //I.e. control passes to that to farm out that "set" of paramthings at a time? May be a generation, may not be?
  //We pass a set of varlists to execute, and it does everything for me.
  
  size_t parampoint_generator::generate( const varlist<std::string>& vl, seedgen& sg, const bool& usedisk=false )
  {
    //Takes the varlist...
    hierarchical_varlist<std::string> hv( vl );
    
    parampoint_vars.push_back( hv );
    
    std::string dirname = basedir + "/" + std::to_string( parampoints.size() );
    
    memfsys myfsys;
    
    parampoint retp = exec_rep.build_parampoint( hv, dirname, myfsys, sg, usedisk );
    
    parampoints.push_back ( retp );

    parampoint_results.push_back( parampoint_result( retp ) );
    
    //fprintf(stdout, "Finished, now GENERATE pg, will enumerate.\n");
    //hv.enumerate();
    
    parampoint_memfsystems.push_back( myfsys );
    
    return (parampoints.size() - 1);
  }



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

