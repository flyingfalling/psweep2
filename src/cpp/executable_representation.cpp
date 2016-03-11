
#include <executable_representation.h>



//REV: OK, so now what I have is the functions etc. that does executable representation, and they will "create" the appropriate varlist etc., based
//on "parampoint" etc.?
//Note, there are some "functional" variables, such as file directory, etc.? Worker number?

//Let user add to these, etc. How do we do "named" vars? How do we access "worker X in previous paramset etc.?"





//User writes his own functions, which are void functions that just take a functsig as argument, and runs a specified function.


//REV: what var?
std::string setvar(const functsig& fs)
{
  //What is this function? A global function for setting a global variable list? I really should have a more local one...? Some way of specifying "which"
  //variable to set? Which may be latent?
  SETVAR( fs.args[0], fs.args[1] );
}


//Whoa, sometimes it needs to return something? I.e. a string? Oh shit, it always does...?
std::string getvar(const functsig& fs)
{
  return GETVAR( fs.args[0] );
}

struct functsig
{
  std::string tag;
  std::vector<std::string> args; //args.size() is number of args.
};
  
  //Constructor
functlist_item::functlist_item( const std::string _tag, const std::function<void(functsig)> _funct, const size_t _nargs )
  : tag(_tag),
    funct(_funct),
    nargs(_nargs)
  {
    //REV: do nothing
  }
  
  std::string functlist_item::execute( const functsig& fs )
  {
    if( fs.args.size() != nargs )
      {
	fprintf(stderr, "ERROR: in execute of functlist_item [%s]: nargs (%ld) != args of passed functsig (%ld)\n", tag.c_str(), nargs, fs.args.size());
	exit(1);
      }
    else
      {
	return funct( fs );
      }
  }
