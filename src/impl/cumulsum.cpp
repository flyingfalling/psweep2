//#pragma once 
#include <cumulsum.h>


//REV: SHIT, if they're zero probability EVERYWHERE, it should be the LAST before the change!
template <typename T>
void test_binary_search_cumsum()
{
  size_t correct=3;
  double targ=0.49;
  std::vector<T> t1;
  t1.push_back(0.0);
  t1.push_back(0.0);
  t1.push_back(0.0); //THIS (idx 2) is the correct solution... REV: NO NO NO 3 is the correct solution!!!!
  t1.push_back(0.5); //0.5 prob.
  t1.push_back(0.0);
  t1.push_back(0.0);
  t1.push_back(0.0);
  t1.push_back(0.0);
  t1.push_back(0.0);
  t1.push_back(0.0);
  
  std::vector<T> t2 = compute_cumsum(t1);
  size_t start =0;
  size_t res = binary_search_cumsum(t2, start, t2.size()-1, targ);
  fprintf(stdout, "BINARY SEARCH TEST RESULT: (should be index %ld): %ld\n", correct, res);
  if(res != correct)
    {
      fprintf(stderr, "BINARY SEARCH TEST FAILED, EXITING\n");
      exit(1);
    }
  
  recompute_cumsum_based_on_choice(t1, t2, res);
  
  //should be all zeros
  for(size_t x=0; x<t2.size(); ++x)
    {
      if(t2[x] != 0)
	{
	  fprintf(stderr, "TEST BIN SEARCH: recompute of cumsum failed, we have a non-zero member after choosing all probability density locations\n");
	  exit(1);
	}
    }
  
}

template <typename T>
void printvect_cerr(std::vector<T> v)
{
  for(size_t x=0; x<v.size(); ++x)
    {
      std::cerr << v[x] << " ";
    }
  std::cerr << std::endl << std::endl;
}

//REV: I Think this is OK. It puts: [0, 1) at 0, [1, 2) at 1, etc. And then 
//in the end, it puts 21.0 at 20 (because it's greater), really position 20
//should only contain [20, 21).

//REV* FIX: need to go back to the earliest zero!

//REV: might have some problem with large patches of zero prob...
//If <, then it gets it! because [0, 1), i.e. >=. So, 0.5, 0.5, would make 0.5, 1.0, and if we generate exactly 0.5, it would get 2nd guy (idx 1)

//REV: no fun, if there is too small cumsum it will all go to zero. Do I really want to normalize them?

//REV: I must be stupid, if the array is sparse I should treat it as a sparse array...the zeros in between are meaningless pretty much...
template <typename T>
size_t binary_search_cumsum(const std::vector<T>& array, size_t start, size_t end, T targ)
{
#if DEBUG > 20
  fprintf(stdout, "Binary searching  (from %ld to %ld)\n", start, end);
#endif
  if(end-start == 0)
    {
      if( end != array.size()-1) 
	{
	  if( !(array[end+1] >= targ) )
	    {
	      fprintf(stderr, "(end-start=0/* ) ERROR IN BINARY SEARCHING CUMUL SUM, NOT EXPECTED RESULT. We should either be the last in the array, OR the next member in the array should be greater than or equal to me. We are not last!!! Instead for targ %lf (of %ld length array) I chose %ld, which is of val %f, whereas next member (idx %ld) is value %lf EXITING\n", targ, array.size(), end, array[end], end+1, array[end+1]);
	      printvect_cerr(array);
	      //exit(1);
  	    }
  	}
      if( end != 0 )
  	{
  	  if( !(array[end-1] < targ) )
  	    {
  	      fprintf(stderr, "(end-start=0) ERROR IN BINARY SEARCHING CUMUL SUM, NOT EXPECTED RESULT. We should either be the first in the array, OR the previous member in the array should be strictly less than me. We are not first!!! Instead for targ %lf (of %ld length array) I chose %ld, which is of val %lf, whereas previous member (idx %ld) is value %lf EXITING\n", targ, array.size(), end, array[end], end-1, array[end-1]);
	      printvect_cerr(array);
  	      //exit(1);
  	    }
  	}
      return end; // == start
    }
  if( end-start == 1)
    {
      if(targ < array[start]) //must be strictly less than to be included. E.g. 0.5 would go to second guy in 0.5 0.5.
  	//HOWEVER, this is a problem, because if we e.g. generate between 0 and 0.5, and there are lots of empty 0.5 near end,
  	//only a 0.49 would win...
  	{
  	  if( start != 0 )
		{
		  if( !(array[start-1] < targ) )
		    {
		      fprintf(stderr, "(end-start=1, targ<array[start]) ERROR IN BINARY SEARCHING CUMUL SUM, NOT EXPECTED RESULT. We should either be the first in the array, OR the previous member in the array should be strictly less than me. We are not first!!! Instead for targ %lf (of %ld length array) I chose %ld, which is of val %lf, whereas previous member (idx %ld) is value %lf EXITING\n", targ, array.size(), start, array[start], start-1, array[start-1]);
		      printvect_cerr(array);
		      //exit(1);
		    }
	    }
	  
	  if( !(array[start+1] >= targ) )
	    {
	      fprintf(stderr, "(end-start=1, targ<array[start]) ERROR IN BINARY SEARCHING CUMUL SUM, NOT EXPECTED RESULT. We should either be the last in the array, OR the next member in the array should be greater than or equal to me. We are not last!!! Instead for targ %lf (of %ld length array) I chose %ld, which is of val %lf, whereas next member (idx %ld) is value %lf EXITING\n", targ, array.size(), start, array[start], start+1, array[start+1]);
	      printvect_cerr(array);
	      //exit(1);
	    }
	  return start;
	}
      else //ELSE, if targ >= array[start]
	{
	  if(targ >= array[end])
	    {
	      fprintf(stderr, "SUPER BIG ERROR IN BINARY SEARCH GOT TO *end* but target is still > than me: (targ %f, val of last (%ld): %f)\n", targ, end, array[end]);
	      printvect_cerr(array);
	      //exit(1);
	    }
	  if( end != 0 )  
	    {
	      if( !(array[end-1] < targ) ) //== start. Note, could start be equal to end if I chose end? No...
		{
		  fprintf(stderr, "(end-start=1, targ>=array[start]) ERROR IN BINARY SEARCHING CUMUL SUM, NOT EXPECTED RESULT. We should either be the first in the array, OR the previous member in the array should be strictly less than me. We are not first!!! Instead for targ %lf (of %ld length array) I chose %ld, which is of val %lf, whereas previous member (idx %ld) is value %lf. EXITING\n", targ, array.size(), end, array[end], end-1, array[end-1]);
		  printvect_cerr(array);
		  //exit(1);
		}
	    }
	  if( end != array.size()-1) 
	    {
	      if( !(array[end+1] >= targ) )
		{
		  fprintf(stderr, "(end-start=1, targ>=array[start]) ERROR IN BINARY SEARCHING CUMUL SUM, NOT EXPECTED RESULT. We should either be the last in the array, OR the next member in the array should be greater than or equal to me. We are not last!!! Instead for targ %lf (of %ld length array) I chose %ld, which is of val %lf, whereas next member (idx %ld) is value %lf. EXITING\n", targ, array.size(), end, array[end], end+1, array[end+1]);
		  printvect_cerr(array);
		  //exit(1);
		}
	    }
	  return end;
	}
    }
  //else if( targ <= array[start+(end-start)/2] ) //if >=, it is in the second half (i.e. not include that one). If <, it could be anywhere below.
  
  
  //We want to get the LAST one (i.e. greatest) that is still STRICTLY LESS than the targ.
  //So, want to find 
  size_t halfpt=start+(end-start)/2; //meh this won't work...
  if( targ < array[halfpt]) //if it's in second half, it is most definitely NOT contained in half one (might be in last of first tho)
    { 
      return binary_search_cumsum(array, start, halfpt, targ); //*does* "include" the last value.... E.g. if its 0...10, we check 5.
      
    }
  else //targ < array[start+(end-start)/2], i.e. bottom half //if <=, we search the first half. OK...
    {
      return binary_search_cumsum(array, halfpt+1, end, targ);
      //if targ < array[5], we search 0...5. So, then half of that is 2, if greater than array[2], we do 3...5. Then, half is 4, so array[4]. 
      //if >= array[4], we do a search for 5...5 (because it must be strictly less than 4 to be 4).
      //if < array[4], we do search in 3..4. Now we just check, is it less than 3? If so, then 3 wins. Else 4 wins. Problem is if e.g. um, 
      //it was like (targ=0.5)
      //0.0  0.0  0.0  0.0  0.0  0.5  0.5  0.5  0.5  0.5  0.5
      // 0                        |                        10
      //                      targ >= array[5], thus, search in: 5..10
      //                         0.5  0.5  0.5  0.5  0.5  0.5
      //                                    |
      //                      0.5 >= 0.5? Yes, so search 7..10
      //                         0.5  0.5  0.5  0.5  0.5  0.5
      //                                         |
      //   We will end up getting the latermost one. Not correct, we want the "earliest"
      //   So, in the search, only search above if it is strictly greater than.
    }
  fprintf(stderr, "WHOA, reached end of binary search when we shouldn't have...exiting\n");
  exit(1);
}


//void recompute_cumsum_based_on_choice(std::vector<float>& array, std::vector<double>& cumsum, size_t choice);

//array is individual probs
template <typename T>
void recompute_cumsum_based_on_choice(const std::vector<T>& array, std::vector<T>& cumsum, size_t choice)
{
  //#pragma omp parallel for 
  if(choice >= array.size() || choice >= cumsum.size() || cumsum.size() != array.size()) {fprintf(stderr, "ERROR IN RECOMPUTE CUMSUM, array/cumsum size is not equal or choice is outside array bounds!\n"); exit(1); }
  T val=array[choice];
  for(size_t a=choice; a<cumsum.size(); ++a)
    {
      cumsum[a] -= val;
      if(cumsum[a] < 0)
	{
	  fprintf(stderr, "WARNING/ERROR: in recompute cumsum, we have a massive problem: index %ld tried to go below zero (to %f by subtracting %f)\n", a, cumsum[a], val);
	  cumsum[a] = 0;
	}
    }
}


template <typename T>
std::vector<T> compute_cumsum( const std::vector<T>& array )
{
  
  if(array.size() < 1)
    {
      fprintf(stderr, "SUPER ERROR compute_cumsum, array size < 1\n"); exit(1);
    }
  
  std::vector<T> cumsum( array.size() );
  
  
  cumsum[0] = array[0];
  for(size_t a=1; a<array.size(); ++a)
    {
      if(array[a] < 0)
	{
	  fprintf(stderr, "WHOA ERROR IN CUMSUM, we have a negative probability?!?!?!\n");
	  exit(1);
	}
      cumsum[a] = cumsum[a-1] + array[a]; //REV: ah, this is the problem, we are not changing the cumsums!
    }
  
  /*if(cumsum[ cumsum.size()-1 ] == 0)
    {
      fprintf(stderr, "WE HAVE A PROBLEM: your cumulative sum computation is filled with only zeros, there will not be enough probability density to do anything\n"); 
      exit(1);
      }*/

  return cumsum;
}



/* std::vector<size_t> choose_k_multinomial_no_replace(std::vector<float>& array, std::vector<float>& cumsum, size_t k) */
/* { */
/*   std::vector<size_t> ret( k ); */
/*   //std::vector<double> cumsum( array.size() ); */
/*   //std::vector<double> cumsum = prevcumsum; //copy constructor */
  
/*   //REV: use the gsl RNG? -- must be inited, we know it's global in this case lol. Um,  */
/*   //gsl gives [0, 1), so we're OK there. */
  
  
  
  
/*   for(size_t n=0; n<k; ++n) */
/*     { */
/*       double randnum = gsl_rng_uniform( nsim3_rand_source ) * cumsum[ cumsum.size() - 1 ];  */
/*       ret[n] = binary_search_cumsum(cumsum, 0, cumsum.size()-1, randnum); //REV: I added this to be -1, is that OK...? */
/*       //size_t find=0; */
/*       //while( cumsum[find] > randnum ) { ++find; } */
/*       //ret[n] = find; */
/*       //fprintf(stderr, "Choosing [%ld]th\n", n); */
/*       recompute_cumsum_based_on_choice( array, cumsum, ret[n] ); */
      
/*       if(cumsum[ cumsum.size()-1 ] == 0) */
/* 	{ */
/* 	  #if DEBUG > 3 */
/* 	  fprintf(stderr, "REV: MAJOR WARNING, there is not enough probability density in the cumulative sum to make a selection (density is zero everywhere)\nWe will return with what we have\n"); */
/* 	  #endif */
/* 	  break; */
/* 	} */
/*     } */
  
/*   return ret; */
/* } */

template <typename T>
std::vector<size_t> choose_k_multinomial_no_replace_wrand(std::vector<T>& array, const std::vector<T>& cumsum, const std::vector<T>& randns)
{
  //fprintf(stdout, "Choosing: %d rand: %f\n", randns.size(), randns[0]);
  std::vector<size_t> ret( randns.size() );
  //std::vector<double> cumsum( array.size() );
  //std::vector<double> cumsum = prevcumsum; //copy constructor
  
  //REV: use the gsl RNG? -- must be inited, we know it's global in this case lol. Um, 
  //gsl gives [0, 1), so we're OK there.
  
  if(cumsum.size() != array.size())
    {
      fprintf(stderr, "SUPER ERROR IN CHOOSE K MULTINOM, CUMSUM SIZE != ARRAY SIZE\n");
    }
  
  //fprintf(stdout, "\nChoosing: %ld from %ld\n", randns.size(), cumsum.size());
  
  std::vector<T> array2 = array;
  std::vector<T> cumsum2 = cumsum; //compute_cumsum( array );
  for(size_t n=0; n<randns.size(); ++n)
    {
#if DEBUG > 10
      fprintf(stdout, "Probabilities: ");
      for(size_t z=0; z<array2.size(); ++z)
	{
	  fprintf(stdout, "%3ld[%4.3f] ", z, array2[z]);
	}
      fprintf(stdout, "\n");
      fprintf(stdout, "Cumulative sum: ");
      for(size_t z=0; z<array2.size(); ++z)
	{
	  fprintf(stdout, "%3ld[%4.3f] ", z, cumsum2[z]);
	}
      fprintf(stdout, "\n\n");
#endif
      
      T randnum = randns[n] * cumsum2[ cumsum2.size() - 1 ]; 
      size_t start=0;
      ret[n] = binary_search_cumsum<T>(cumsum2, start, cumsum2.size()-1, randnum);
      if(array2[ret[n]] == 0)
	{
	  fprintf(stderr, "WHOA, big problem in cum sum -- we selected an index with zero probability density...\n");
	  exit(1);
	}
      
      array2[ret[n]] = 0;
      //recompute_cumsum_based_on_choice( array2, cumsum2, ret[n] );
      cumsum2 = compute_cumsum( array2 );
      
      
      //#if DEBUG > 10
      
      //REV: could use gsl_ran_flat (const gsl_rng * r, double a, double b) instead of uniform, which is guaranteed [0, 1)
      
#if DEBUG > 10
      fprintf(stdout, "SELECTED: %ld\n\n", ret[n]);
      
#endif
      
      if(ret[n] >= cumsum2.size())
	{
	  fprintf(stderr, "WHOA CHOOSE K MULTINOM RETURNED AN ILLEGAL INDEX!!!\n"); exit(1);
	}
     
      //size_t find=0;
      //while( cumsum[find] > randnum ) { ++find; }
      //ret[n] = find;
      //recompute_cumsum_based_on_choice( array, cumsum, ret[n] );
      if(cumsum2[ cumsum2.size()-1 ] <= 0)
	{
	  //#if DEBUG > 3
	  fprintf(stderr, "REV: MAJOR WARNING, there is not enough probability density in the cumulative sum to make a selection (density is zero everywhere)\nWe will return with what we have (we did %ld out of %ld)\n", n, randns.size());
	  //#endif
	  ret.resize(n);
	  return ret;
	  //break;
	}
      //fprintf(stdout, "%d ", ret[n]);
    }
  //fprintf(stdout, "\n");
  
  return ret;
}
