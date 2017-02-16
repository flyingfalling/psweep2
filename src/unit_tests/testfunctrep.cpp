#include <psweep_cmd_script_parser.h>
#include <script_unroller.h>
#include <functrep.h>
#include <variable.h>
//#include <hierarchical_varlist.h>


int main(int argc, char **argv)
{

    
  size_t varidx=0;

  functlist myfl; //automatically registers legal functions hardcoded in struct..
  
  
      
  std::vector<functrep> frlist;

  std::vector<client::LEAF_STMNT> emptyls;

  // /*
  // std::vector< client::LEAF_STMNT > lslist1;
  // client::STMNT c("SETVAR");
  // client::LEAF_STMNT a("VAR1NAME", emptyls, true);
  // client::LEAF_STMNT b("VAR1VAL", emptyls, true);
  // lslist1.push_back(a);
  // lslist1.push_back(b);
  // c.ARGS=lslist1;
  
  // functrep tmpfr1(c, myfl);
  // frlist.push_back( tmpfr1 );
  // */
  
  // //client::STMNT xx("IDENTITY");
  // //std::vector< client::LEAF_STMNT > lslistxx;
  // //client::LEAF_STMNT xz("TESTVAL", emptyls, true);
  // client::LEAF_STMNT xx("MYVAL", emptyls, true);
  // //lslistxx.push_back(xz);
  // xx.ARGS=emptyls;
  // functrep frtest( xx, myfl );

  // //This results in an IDENTITY MYVAL
  
  // /*functtype ftx = IDENTITY;
  // variable<std::string> v( "TMPNAME", "TESTVAL" );
  // std::vector<variable<std::string>> vv;
  // vv.push_back(v);*/
  
  // std::vector< hierarchical_varlist<std::string> > hvvv;
  // std::vector< size_t > hvvs;
  // variable<std::string> res = frtest.execute( hvvv, hvvs );
  // fprintf(stdout, "PRINTING RESULT (should be == [TESTVAL]): [%s]\n", res.get_s().c_str());

  // return 0;
  
  
  

  // fprintf(stdout, "Now doing search...\n");
  // functrep fff( xx  , myfl );

  // variable<std::string> res2 = fff.execute( hvvv, hvvs );
  
  
  
  // return 0;
  
  
  std::vector< client::LEAF_STMNT > lslist2;
  client::STMNT c2("SETVAR");
  client::LEAF_STMNT a2("VAR2NAME", std::vector<client::LEAF_STMNT>(), true);
  client::LEAF_STMNT b2("VAR2VAL", std::vector<client::LEAF_STMNT>(), true);
  lslist2.push_back(a2);
  lslist2.push_back(b2);
  c2.ARGS=lslist3;

  functrep tmpfr2(c2, myfl);
  frlist.push_back( tmpfr2 );
      
  std::vector< client::LEAF_STMNT > lslist3;
  client::STMNT c3("SETVAR");
  client::LEAF_STMNT a3("VAR3NAME", std::vector<client::LEAF_STMNT>(), true);
  client::LEAF_STMNT b3("VAR3VAL", std::vector<client::LEAF_STMNT>(), true);
  client::LEAF_STMNT e3("$VAR2NAME", std::vector<client::LEAF_STMNT>(), true);
  client::LEAF_STMNT f3("YOLO", std::vector<client::LEAF_STMNT>(), true);
  std::vector< client::LEAF_STMNT > nestedlist3;
  nestedlist3.push_back("-");
  nestedlist3.push_back(e3);
  nestedlist3.push_back(f3);
  client::LEAF_STMNT d3("CAT", nestedlist3, false);
  lslist3.push_back(d3);
  lslist3.push_back(b3);
  c3.ARGS=lslist3;
            
  functrep tmpfr3(c3, myfl);
  frlist.push_back( tmpfr3 );
      
  
      
      
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

  hvl.add_child( 0, vl2 );
  hvl.add_child( 0, vl4 );
  hvl.add_child( hvl.get_children(0)[0], vl3 );



      
  fprintf(stdout, "\n\nVARLIST BEFORE MODIFICATION BY EXECUTION\n");
  hvl.enumerate();
  std::vector<hierarchical_varlist<std::string>>  hvlvect;
  hvlvect.push_back( hvl );
            
  std::vector<size_t> whichvarlist;
  std::vector<size_t> cs = hvl.get_children(0);
  fprintf(stdout, "Size should not be zero: %ld\n", cs.size() );
  whichvarlist.push_back( hvl.get_children(0)[0] );

  fprintf(stdout, "Now attempting to execute!\n");
      
  for(size_t f=0; f<frlist.size(); ++f)
    {
      fprintf(stdout, "Executing statement [%ld] (stmnt: [%s])\n", f, "XXX");
      variable<std::string> t = frlist[f].execute( hvlvect, whichvarlist  ); //could do a "find varlist by name" type thing to make it easier...?
    }
      
  fprintf(stdout, "\n\nVARLIST **AFTER** MODIFICATION BY EXECUTION\n");
  hvl.enumerate();


  //do it again:

  
  

}
