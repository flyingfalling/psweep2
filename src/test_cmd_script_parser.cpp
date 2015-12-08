#include <psweep_cmd_script_parser.h>
#include <script_unroller.h>



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
  
  for(size_t a=0; a<r.psets.size(); ++a)
    {
      fprintf(stdout, "=======PSET # [%ld]\n\n\n", a);
      enum_pset( r.psets[a] );
      fprintf(stdout, "\n\n++++ UNROLLED:\n");
      //std::deque< client::STMNT > tounroll(r.psets[a].CONTENT); //vector to deque?
      std::deque< client::STMNT > tounroll(r.psets[a].CONTENT.begin(), r.psets[a].CONTENT.end()); //vector to deque?
      std::deque< client::STMNT > unrolled = recursive_unroll_nested_functs(tounroll, varidx);
      //client::PSET p( r.psets[a]
      r.psets[a].CONTENT = std::vector<client::STMNT>(unrolled.begin(), unrolled.end()); //deque to vect?
      enum_pset( r.psets[a] );
    }
  
}
