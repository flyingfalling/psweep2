#include <psweep_cmd_script_parser.h>
#include <script_unroller.h>
#include <functrep.h>
#include <variable.h>
//#include <hierarchical_varlist.h>


int main(int argc, char **argv)
{

  bool usefile=false;
  std::string ftoparse;
  if(argc > 1)
    {
      if(argc > 2)
	{
	  fprintf(stderr, "ERROR: got more than the expected [1] cmd line arguments (should only take filename of file to parse)\n");
	}
      fprintf(stdout, "Read in cmd line arguments, will parse file [%s]\n", argv[1]);
      ftoparse = std::string(argv[1]);
      usefile = true;
    }
  
 
  //std::string input= "PSET P1 10; STMNT1; STMNT2; /PSET P1;"; // PSET P2 1; STMNT1; /PSET P2";

  std::string input= "PSET P1, 10; STMNT1(YO(), YOA(THIS2(), BUB()), YOB()); STMNT2(YO2(), YO2A()); !PSET P1;"; // PSET P2 1; STMNT1; /PSET P2";
  //std::string input= "PSET P1, 10; STMNT1(YO(), YOA(THIS2(), BUB()), YOB()); STMNT2(YO2(), YO2A()); !PSET P1; PSET P2, 5; STMNT1(YO(), YOA(THIS2(), BUB()), YOB()); STMNT2(YO2(), YO2A()); !PSET P2;"; // PSET P2 1; STMNT1; /PSET P2";
  if(usefile)
    {
      input = get_file_contents(ftoparse);
    }
  
  client::PARAMPOINT r = parse_psweep_script( input );

  
  fprintf(stdout, "PARSED PARAMPOINT, got [%ld] PSETS\n", r.psets.size());

  /*for(size_t a=0; a<r.psets.size(); ++a)
    {
      fprintf(stdout, "PSET # [%ld]\n", a);
      enum_pset( r.psets[a] );
    }
  */

  
  
  size_t varidx=0;

  functlist myfl; //automatically registers legal functions hardcoded in struct..
  
  for(size_t a=0; a<r.psets.size(); ++a)
    {
      fprintf(stdout, "=======PSET # [%ld]\n\n\n", a);
      enum_pset( r.psets[a] );
      fprintf(stdout, "\n\n++++ UNROLLED:\n");
      //std::deque< client::STMNT > tounroll(r.psets[a].CONTENT); //vector to deque?

      std::deque< client::STMNT > tounroll(r.psets[a].CONTENT.begin(), r.psets[a].CONTENT.end()); //vector to deque?
      std::deque< client::STMNT > unrolled = recursive_unroll_nested_functs(tounroll, varidx);
      
      r.psets[a].CONTENT = std::vector<client::STMNT>(unrolled.begin(), unrolled.end()); //deque to vect?
      enum_pset( r.psets[a] );

      fprintf(stdout, "   \nNow, will construct executable representation and execute!\n\n");

      std::vector<functrep> frlist;
      for(size_t s=0; s<r.psets[a].CONTENT.size(); ++s)
	{
	  functrep tmpfr(r.psets[a].CONTENT[s], myfl);
	  frlist.push_back( tmpfr );
	}

      //When I actually execute it, it will have the desired effect. For example, changing a varlist etc.
      varlist<std::string> vl1("ROOTVLIST");
      vl1.addTvar( "V1", "32");
      vl1.addTvar( "V2", "42");
      std::vector<std::string> arr;
      arr.push_back( "AV1" );
      arr.push_back( "AV2" );
      vl1.addArrayvar( "V2", arr );

  

      varlist<std::string> vl2("LEFT1LEVEL");
  
      vl2.addTvar( "V1-1", "X-CMD");
      vl2.addTvar( "V1-2", "493");
      std::vector<std::string> arr2;
      arr2.push_back( "222.1" );
      arr2.push_back( "830.94" );
      vl2.addArrayvar( "V21", arr2 );

  
      varlist<std::string> vl3("LEFT2LEVEL");
  
      vl3.addTvar( "V3-1", "YOLOV1");
      vl3.addTvar( "V3-2", "YOLOV2");
      std::vector<std::string> arr3;
      arr3.push_back( "AV3-1" );
      arr3.push_back( "AV3-2" );
      vl3.addArrayvar( "V32", arr3 );

      varlist<std::string> vl4("RIGHT1LEVEL");
  
      vl4.addTvar( "V4-1", "YOLOV1");
      vl4.addTvar( "V4-2", "YOLOV2");
      std::vector<std::string> arr4;
      arr4.push_back( "AV4-1" );
      arr4.push_back( "AV4-2" );
      vl4.addArrayvar( "V42", arr4 );

      fprintf(stdout, "TESTING hierarchical varlist:\n");
      hierarchical_varlist<std::string> hvl (vl1 );

      hvl.add_child( 0, vl2 ); //this holds V2
      hvl.add_child( 0, vl4 );
      hvl.add_child( hvl.get_children(0)[0], vl3 );



      
      fprintf(stdout, "\n\nVARLIST BEFORE MODIFICATION BY EXECUTION\n");
      hvl.enumerate();
      std::vector<hierarchical_varlist<std::string>>  hvlvect;
      hvlvect.push_back( hvl );
            
      std::vector<size_t> whichvarlist;
      std::vector<size_t> cs = hvl.get_children(0);
      fprintf(stdout, "Size should not be zero: %ld\n", cs.size() );
      whichvarlist.push_back( hvlvect[0].get_children(0)[0] );

      fprintf(stdout, "Now attempting to execute!\n");
      
      for(size_t f=0; f<frlist.size(); ++f)
	{
	  fprintf(stdout, "Executing statement [%ld] (stmnt: [%s])\n", f, frlist[f].fs.mytag.c_str());
	  variable<std::string> t = frlist[f].execute( hvlvect, whichvarlist  ); //could do a "find varlist by name" type thing to make it easier...?
	}
      
      fprintf(stdout, "\n\nVARLIST **AFTER** MODIFICATION BY EXECUTION\n");
      hvlvect[0].enumerate();
    }

  //do it again:

  
  

}
