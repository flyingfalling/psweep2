#include <fake_system.h>



fake_sys_rep::fake_sys_rep( const std::string& s, const   fake_system_funct_t f )
  {
    name = s;
    funct = f;
  }

fake_system::fake_system( )
  {
  }
  
  //Must take some kind of "arglist"?, as well as the fake FS.
  /*fake_system( mem_filesys& mf )
  {
    my_filesys = mf;
    }*/
  
  
  
  void fake_system::register_funct( const std::string& name, const fake_system_funct_t& funct )
  {
    fake_sys_rep fsr( name, funct );
    sys_functs.push_back( fsr );
  }

  bool fake_system::call_funct( const std::string& name, const std::vector<std::string>& args, memfsys& my_filesys )
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
