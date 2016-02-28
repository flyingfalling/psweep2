#pragma once

//#include <memfile3.h>
#include <memfsys.h>

#include <commontypes.h>

//REV: fake system() call struct thing.
//Basically, takes a function (?) pointer? User implements it
//and adds it to the thing.
//Arguments are all in the form of variable=value. And user handles it.
//It takes a mem_filesystem struct as an argument too, so it can effectively
//read from that. Much nicer than passing a bunch of void pointers and
//forcing user to  handle them?
//Another alternative is to just pass VARLISTS that can include
//arbitrarily complex things.

//User can specify in the CMD script at beginning that it should execute
//this first (in other words, it should always check through some
//artificial_system list before actually calling system() on the target).

//So, user instead of saying ADD_TO_CMD, he says ADD_TO_MEM_CMD or something.
//And, he specifies a string that specifies which of the fake_system() structs
//to call. Furthermore, he adds stuff to it, like adds a specific "option"
//to be equal to a specific "value" (i.e. string, i.e. it takes a varlist?
//They will be parsed by user side just like argc and argv. But user will
//always read from fake filesystem. It's super ghetto, but at least everything
//will be called in memory, and we won't hit the LDAP for calling a .exe file
//each time, at least for C/C++ code (or dynamic links to other guys that
//we can establish once at startup time).

//This still requires user to be able to "encapsulate/restart" his simulation
//cleanly at each function call.

//So, format will be just a list of strings. And user can parse it as he
//wishes. Totally agnostic to etc. And user can simply wrap it in main
//if he wishes to do system() that way, in other words he can call
//other programming langauges, or just have main() call the program with
//the string array equal to argc/argv.

//Basically user makes one of these, passes his wrapper function to it (must be a basic function that takes only the mem_filesystem and arglist with it)
//REV; Haha why not just do the stuff my way...


//QUESTION: where/how to have user specify write/through or not.
//In his specification. Of course if it is a normal system() call, force them to all be written out (on client side).
//On master side, it might not be necessary to write it out. We can read output directly from the mem_file. We can force some to be saved (required)
//At any rate, user thing specifies how it should be read in (as vars always?)
//Just return MEMFILE to it?

//On other hand, if it is FAKE_SYSTEM, in that case let's SOMETIMES write out required files to disk (specify which ones). OK, so let's just say,
//Main use for this is FILESENDER writing out the required/etc. files locally on client, after transferring from MASTER via MPI.
//Not much need to write back to MASTER except for 1) logging purposes (in which case output to HDF5 straight?), or 2) to get the output (always guaranteed
//to be in VARIABLE format?)

//So a few things to decide:

//0: Will the file be read from DISK on MASTER side (before computation?). For example, if we store it in MEMORY after first read only, we can send it
//that way to each one without re-reading it. Note these might require a massive directory of many audio files for example...in which case I want "read-dir"
//and recursive option...

//1: Will the file be written to DISK on MASTER side (before computation?). Reason for this is e.g. if it was a "parameters" or something file to tell
//what parameters were use, I would write it to my thing and blah. Only reason to this would be remember what happened, and in that case I could just
//log it to my HDF5 file of strings... I.e. add a LOG option to log notes/variables/files/etc.

//2: Will the file (input?) be written to DISK on SLAVE side (answer is ALWAYS yes if it is a real SYSTEM() command, unless I pass with the system arg somehow). Wait, might write output after getting directly from memory for some reason? Nah.
//3: Will the file (output?!) be read from DISK on SLAVE side? Again, for sure if it was a real SYSTEM() command, otherwise it might go either way.

//4: Will the file be written to DISK on MASTER side (after finishing computation?). Only reason for this is if user wants to do it?
//5: Will the file be read from DISK on MASTER side (after finishing computation?). No reason to really do this, I'd mostly get it directly from SLAVE
//   side in MEMORY anyway, so no need to re-read it...


//Slave side, write out/read in is main issue.


//So, by default, master side ONLY writes to a memfile. Furthermore, they are in some directory (oh...shit...that's fine though). Anyway, user can specify.
//They can stay in that dir I guess... If I use a mem file-system, then there is some thing to do with directories?

//So, remember, I will be "re-basing" anyways. So I need to account for that. Like, taking form "this side" (finding in dir first), and then sending
//to other side. OK. So it will always "search" on required side, and send them all. It will also rename them (?). For example it is using /tmp thing.
//I will do renaming etc. regardless. Question is whether I will actually write/read them.
//So, when user specifies

//REQUIRED, SUCCESS, OUTPUT, and INPUT files, for each file, he will specify whether (on my side) the file should be written to file or not. Or,
//whether it should be actually loaded from file. Easier, is if I check file in memory. If it exists, that's fine, I load it that way. Otherwise, I
//could try to load it from disk too. I.e. if file does not exist already in memory, I will automatically read-through from disk ( to see if it
//exists ). If it does exist, I read it. Otherwise, I make an empty memfile. Problem is, in some cases, I don't care if it exists in disk. I want
//to delete it. For output files, I will try to read it from file, then from disk (i.e. that is the setting).

//Main question is, if the file doesn't exist, do I want to try to read it from disk? Or do I just want to make a temp file. Only situation to make
//new is if I KNOW it doesn't exist. Or if I'm opening a disk file. I.e. making a "new" file is different from "opening" a file. Separate those.
//Thus, I can do everything with those separate commands. Meh, easiest to check it's existence, if it doesn't exist, make it anyway (who cares about
//the disk). Problem is the checks will hit the LDAP server...or NFS, which might take time etc.

//EASIEST way of all is to just separate into DISK writes and NONDISK writes. In other words, have a "marker" for each file telling whether it should
// a) TRY to read from disk (only for OUTPUT files/SUCCESS files) b) WRITE to disk (only for INPUT/REQUIRED) files. All on slave side of course.
//Have a flag in MEMFILE, of whether it actually has/should have corresponding disk thing. For mem_filesystem, easiest to rebase it...? or something?
//Easiest to re-build it each time...? REQUIRED files, copying into each, waste. Oh well for now heh.
//How to do it? Make a "new" list. of mem req files etc.?

//Ah, give two options. Just make a new filename, easier that way. Default writes to disk? Yea...and non-default doesn't. I.e. add required DISK-file
//or some shit.

//have a marker in memfile, that notes whether it should be written/read from disk. At any rate, we need a marker that prevents it from the default
//behavior? Or a "setting". That is easiest. Turn off disk-writes. That's easiest. I.e. give option that turns off disk writes. A toggle. It will
//let user access stuff directly. How about, automatically if it is SYSTEM_CMD, we assume now disk writes? But some might disk write. Specify which
//are which, way easier...ugh.

//Just give a single example, and in that case, it writes all to disk (may waste it, oh well). Much nicer that way. We can specify non-NFS local thing
//to make it better. Problem may be our specification of all MYDIR locations, i.e. we re-name them all to there...heh.
//If we keep them around on our side what do we do?
//Just make a new "system" for each? Nah it will re-read input files from disk lololol. Note, some files on MASTER side actually need to be read.
//CREATING one versus "reading" one is different...hm. Read once in beginning. Just do that setting now, much easier? But then I need to "read" or
//create on that side etc. but, question is, in parampoint, do I read out there? Doesn't matter, the setting is global (spread to all guys).

//Note, we could keep a more "permanent" mem filesystem or "data" on other side or something.

//OK that'S fine.

//We've made things nice, now the thing is we want to allow them to take a list of HIERARCHCAL VARLIST I guess? use pointers for god's sake...

//Allow "naming" of named varlists, in this way, we can pass indices of varlists that we want to pass.

//Allow NAMING of hierarchical varlists?

//For example, nsim model might have some variables that we want to have set...modify those later I guess? Oh well.. stuff like um, DEBUG level, or
//um, sstuff to forward there? That we want to read. Stuff that will be passed and then passed through to nsim. I.e. we want to specify it before
//hand hm? Model-specific variables...

//Everything needs to work with FAKE_SYSTEM cmds, user can specify. Wait idea was to turn it all off?

typedef std::vector<std::string>                    arg_list;
typedef std::function< void( const std::vector<std::string>&, memfsys& ) > fake_system_funct_t;


struct fake_sys_rep
{
  std::string name;
  fake_system_funct_t funct;

  fake_sys_rep( const std::string& s, const   fake_system_funct_t f )
  {
    name = s;
    funct = f;
  }

};

struct fake_system
{
  //Has a functor that user calls. Must take a fake FS/varlist (i.e. compile his code to overwrite typical system guys with my fake ones ). Whatever.
  //How does user read in values? Via BINARY or via DOUBLE.
  
  //mem_filesys my_filesys;
  std::vector< fake_sys_rep > sys_functs;

  fake_system( )
  {
  }
  
  //Must take some kind of "arglist"?, as well as the fake FS.
  /*fake_system( mem_filesys& mf )
  {
    my_filesys = mf;
    }*/
  
  
  
  void register_funct( const std::string& name, const fake_system_funct_t& funct )
  {
    fake_sys_rep fsr( name, funct );
    sys_functs.push_back( fsr );
  }

  bool call_funct( const std::string& name, const std::vector<std::string>& args, memfsys& my_filesys )
  {
    std::vector<size_t> locs;
    for(size_t x=0; x<sys_functs.size(); ++x)
      {
	if( name.compare( sys_functs[x].name ) == 0 )
	  {
	    locs.push_back(x);
	  }
	
      }
    //fprintf(stdout, "In CALL FUNCT: Found [%ld] matching target of [%s]\n", locs.size(), name.c_str() );
    if(locs.size() == 1 )
      {
	sys_functs[ locs[0] ].funct( args, my_filesys );
	return true;
      }
    else if( locs.size() > 1 )
      {
	fprintf(stderr, "WARNING ERROR!? In fake system, user you have registered more than one funct of same name [%s]\n", name.c_str());
	sys_functs[ locs[0] ].funct( args, my_filesys );
	return true;
	//exit(1);
      }
    else
      {
	return false; //didn't call it. Assume user will do something with me...?  Crap user will need to handle crap like 1>stdout.out etc. in arg list?
	//Couldn't find it, call it with system for real...
      }
  }


  //Before calling system, I call function (check if it exists). This requires some nastiness.

  //When I actually call, first I check the list of SYSTEM calls. If I find one that matches it, I use it first. User has to set it up appropriately to have that
  //name.
  //Furthermore, what it will do is take the current (global?) fake filesystem with it as well. When it is called, it will take the same CMD list of variables with it.
  //And it will parse those just like a CMD? But it will automatically have stuff output to STDOUT etc. Problem is user program will print to normal STDOUT, I need
  //to make sure everything is redirected to a user-function corresponding file/memfile. I.e. fprintf, but to a memory file? sprintf, basically haha.
};
