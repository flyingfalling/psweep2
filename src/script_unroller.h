#pragma once

#include "psweep_cmd_script_parser.h"


//FUNCTS TO CONVERT TO UNROLLED
template <typename T >
void concat_deque( std::deque< T >& topushto, const std::deque< T >& src )
{
  if(src.size() == 0 ) { return; }

  //Wait, how does this work? It chcks first, then decrements?
  //fprintf(stdout, "Should push back %ld\n", src.size() );
  for(size_t d=src.size(); d>0; --d)
    {

      //REV: wow that is fucked up? Why are indices all off?
      //      fprintf(stdout, "Trying to push %ld\n", d-1);
      topushto.push_front( src[ d-1 ] );
    }
}

template <typename T >
void insert_deque( std::deque< T >& topushto, const std::deque< T >& src, size_t loc )
{
  if( loc > topushto.size() )
    {
      fprintf(stderr, "ERROR in insert_deque, trying to insert outside of size of target queue (queue size=%ld, inserting at %ld)\n", topushto.size(), loc);
      exit(1);
    }

  topushto.insert( topushto.begin()+loc, src.begin(), src.end() );

  return;
}


std::string make_tmpvar ( size_t& varidx )
{
  ++varidx;
  return ( "__TMPLOCAL_" + std::to_string( varidx ) );
}


//This is for any other function. I.e. all guys must be atomic.
std::deque< client::STMNT > check_atomic( const client::STMNT& ss, size_t& varidx )
{

  //REV: check what the tag is. If it is a SETVAR, eventually, we need to allow it to have one nested level.
  //But only the second variable should have one nested level.
  
  std::deque < client::STMNT > newlist;
  client::STMNT oldguy = ss;
  
  for( size_t s=0; s<oldguy.ARGS.size(); ++s )
    {
      client::LEAF_STMNT ts = oldguy.ARGS[s];
      if( ts.ARGS.size() != 0 )
	{
	  //it's not atomic. Thus, we must make a new tmpvar, set it to this guy as an argument, push it to beginning...
	  client::LEAF_STMNT nguy( ts.TAG, ts.ARGS ); //make a new (leaf) statement that is the guy, because it will be an argument to a SETVAR
	  std::string tmpvarname = make_tmpvar( varidx );
	  client::LEAF_STMNT varn( tmpvarname ); //empty one, i.e. read var
	  std::vector< client::LEAF_STMNT > topush;
	  topush.push_back( varn );
	  topush.push_back( nguy );
	  client::STMNT setguy ( "SETVAR",  topush );
	  newlist.push_front ( setguy );
	  oldguy.ARGS[s] = varn; //this is the "read var" leaf stmnt.
	}
    }
  newlist.push_back( oldguy ); //finally add the "sanitized (one level)" guy at the end
  return newlist;
}



//REV: WHat about variables that do things like create directories, set required files etc.? Add new required file etc.?
//This is for setvar.
std::deque< client::STMNT > check_atomic_setvar( const client::STMNT& ss, size_t& varidx )
{

  //REV: check what the tag is. If it is a SETVAR, eventually, we need to allow it to have one nested level.
  //But only the second variable should have one nested level.
  
  std::deque < client::STMNT > newlist;
  client::STMNT oldguy = ss;
  
  if( oldguy.TAG != "SETVAR" )
    {
      fprintf(stderr, "ERROR calling check_atomic_setvar on non SETVAR STMNT (is [%s])\n", oldguy.TAG.c_str());
      exit(1);
    }
  if( oldguy.ARGS.size() != 2 )
    {
      fprintf(stderr, "ERROR in special case SETVAR STMNT check atomic: arglength is not == 2 (==[%ld])\n", oldguy.ARGS.size());
      exit(1);
    }

  

  //Check that first argument is ATOMIC (readvar)
  client::LEAF_STMNT ts = oldguy.ARGS[0];
  if( ts.ARGS.size() != 0 )
    {
      //fprintf(stdout, "Pushing back new SETVAR arg1\n");
      //it's not atomic. Thus, we must make a new tmpvar, set it to this guy as an argument, push it to beginning...
      client::LEAF_STMNT nguy( ts.TAG, ts.ARGS ); //make a new (leaf) statement that is the guy, because it will be an argument to a SETVAR
      std::string tmpvarname = make_tmpvar( varidx );
      client::LEAF_STMNT varn( tmpvarname ); //empty one, i.e. read var
      std::vector< client::LEAF_STMNT > topush;
      topush.push_back( varn );
      topush.push_back( nguy );
      client::STMNT setguy ( "SETVAR",  topush );
      newlist.push_front ( setguy );
      oldguy.ARGS[0] = varn; //this is the "read var" leaf stmnt.
      //fprintf(stdout, "--DONE Pushing back new SETVAR arg1\n");
    }

  //Check that second argument is ONE-LAYER-NESTED at most.
  client::LEAF_STMNT ts2 = oldguy.ARGS[1];
  //Do a check_atomic on THAT statement. Note if it is already a readvar, just leave it. Otherwise, we want to compress it to a first-layer STMNT,
  //but for type stuff we unfortunately need to leave it as LEAF_STMNT. So make one that returns the "new" leaf statement to replace it with, and
  //also statement list to put up above
  //if ARGS.size() == 0, it's just a read, so we're done.
  
  //If it's > 0, then we must check it, add to above statements, and modify ts2 of oldguy.
  if( ts2.ARGS.size() != 0 )
    {
      //fprintf(stdout, "Pushing back new SETVAR arg2\n");
      client::STMNT tmpstmnt( ts2.TAG, ts2.ARGS ); //construct a STMNT from our LEAF_STMNT (ghetto )
      
      std::deque< client::STMNT > toadd = check_atomic( tmpstmnt, varidx );
      
      if( toadd.size() < 1 )
	{
	  fprintf(stderr, "ERROR, check_atomic returned something of size <1, error! [%ld]\n", toadd.size() );
	  exit(1);
	}

      if(toadd.size() > 1)
	{
	  fprintf( stdout, "CHECKED SETVAR  arg2 for atomicity, found that it is NOT ATOMIC! [%ld]\n", toadd.size() );
	}
      
      client::STMNT orig = toadd [ toadd.size() - 1 ]; //this is the last MODIFIED guy!
      toadd.pop_back( ); //delete the last guy? Last ADDED, or last in order?
      
      //reconstruct a leaf stmnt from the updated guy. Actually just set the ts2 index guy to that.
      oldguy.ARGS[1] = client::LEAF_STMNT( orig.TAG, orig.ARGS );

      //fprintf(stdout, "Concating NEWLIST with toadd (newlist: %ld   toadd: %ld)\n", newlist.size(), toadd.size());
      //Add the returned dequeued guys to beginning of our dequeu. Go backwards
      concat_deque( newlist, toadd );
      
      //fprintf(stdout, "--DONE Pushing back new SETVAR arg2\n");
    }
  else
    {
      //fprintf(stdout, "SETVAR arg2 is ARG size 0, i.e. direct variable set X1=X2\n");
    }
  
  
  newlist.push_back( oldguy ); //finally add the "sanitized (one level)" guy at the end
  return newlist;
}



//RECURSIVE:
//We have a list of statements. Each one can/could be expanded to (replaced by) a larger set of statements. Ideally in place. But it will go deep.
//Just do as a single. For any given vector of statements, it will iterate through, and replace each with the next (depth) guy.
//Just make sure to not modify for loops while I go through them.
std::deque< client::STMNT > recursive_unroll_nested_functs( const std::deque< client::STMNT >& stmnts, size_t& varidx )
{
  
  std::deque <client::STMNT> locals = stmnts;
  bool finished = false;
  while( !finished )
    {
      finished=true;

      //fprintf(stdout, "STARTING unroll again (!finished) (size of locals is [%ld])!\n", locals.size() );
      for(size_t s=0; s<locals.size(); ++s)
	{
	  client::STMNT st = locals[s];

	  //fprintf(stdout, "Checking if atomic...\n");
	  std::deque< client::STMNT > rv;
	  if ( st.TAG == "SETVAR" )
	    {
	      //fprintf(stdout, "   Checking if SETVAR atomic...\n");
	      rv = check_atomic_setvar ( st, varidx );
	    }
	  else
	    {
	      //fprintf(stdout, "   Checking if NORMAL atomic...\n");
	      rv = check_atomic ( st, varidx );
	    }

	  //fprintf(stdout, "FINISHED checking atomicity\n");
	  	  
	  //std::deque< client::STMNT > tmpvec;
	  //tmpvec.push_back ( st );
	  
	  //fprintf(stdout, "RECURSIVELY UNROLLING...s=[%ld]\n", s);
	  
	  //REV: OOPS, this needs to check if it's atomic, if so, return.
	  //std::deque< client::STMNT > retval = recursive_unroll_nested_functs( tmpvec, varidx );

	  //If it returned only a single value, it means that it was recursively unrolled already
	  if(rv.size () < 1 )
	    {
	      fprintf(stderr, "ERror returned val from recurisve unroll was 0, \n");
	      exit(1);
	    }
	  
	  if( rv.size() == 1 )
	    {
	      //fprintf(stdout, "REV: found stmnt that is atomic\n");
	      //do nothing. It was perfect
	    }
	  else
	    {
	      //fprintf(stdout, "STMNT is not atomic, inserting etc.\n");
	      //Insert them all to the beginning of S location, and restart s loop. Note we must delete S...?
	      locals.erase( locals.begin() + s );
	      
	      insert_deque( locals, rv, s );
	      //concat_deque( locals, retval ); //pushing to beginning. I *could* do an insert in place...which might actually be better heh.
	      
	      finished = false;
	      break; //this should break the for loop, but not the outer while loop
	    }
	}
      //fprintf(stdout, "WHOA, not finished with while loop yet, redoing (size of locals is [%ld])!\n", locals.size() );
      
    }
  //fprintf(stdout, "Finished SOME level of recursive unrolling...returning locals of size [%ld]\n", locals.size() );
  return locals;
}



//client::PARAMPOINT unrollall(client::PARAMPOINT& src, size_t& varidx)
void unrollall(client::PARAMPOINT& src, size_t& varidx)
{
  //client::PARAMPOINT pp;
  
  //fprintf(stdout, "=======PSET # [%ld]\n\n\n", a);
  //enum_pset( r.psets[a] );
  //fprintf(stdout, "\n\n++++ UNROLLED:\n");
  //std::deque< client::STMNT > tounroll(r.psets[a].CONTENT); //vector to deque?
  for(size_t a=0; a<src.psets.size(); ++a)
    {
      std::deque< client::STMNT > tounroll(src.psets[a].CONTENT.begin(), src.psets[a].CONTENT.end()); //vector to deque?
      std::deque< client::STMNT > unrolled = recursive_unroll_nested_functs(tounroll, varidx);
      //client::PSET p( r.psets[a]
      src.psets[a].CONTENT = std::vector<client::STMNT>(unrolled.begin(), unrolled.end()); //deque to vect?
      //enum_pset( r.psets[a] );
    }
  
}
