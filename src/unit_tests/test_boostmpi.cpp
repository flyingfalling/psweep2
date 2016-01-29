
//REV: This a test program that accomplishes quite a bit.
// Test MPI file send etc. first?
// Test rebasing?

#include <iostream>
#include <string>

#include <boost/mpi.hpp>


void master_loop(const boost::mpi::communicator& world)
{
  fprintf(stderr, "Master %d REPORTING\n", world.rank());
}

void slave_loop(const boost::mpi::communicator& world)
{
  fprintf(stderr, "Slave %d REPORTING\n", world.rank());
}


int main()
{
  boost::mpi::environment env;
  boost::mpi::communicator world;

  if( world.rank() == 0 )
    {
      master_loop(world);
    }
  else
    {
      slave_loop(world);
    }
  
}

