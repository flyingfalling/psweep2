CC=g++
INCLUDEDIR="-I./"
CFLAGS=-std=c++11 -O3


#default: scriptparser2.exe
default: test_cmd_script_parser.exe

testvariable.exe: variable.h testvariable.cpp 
	$(CC) $(CFLAGS) $(INCLUDEDIR) testvariable.cpp -o testvariable.exe

testhvar.exe: variable.h hierarchical_varlist.h testhvar.cpp
	$(CC) $(CFLAGS) $(INCLUDEDIR) testhvar.cpp -o testhvar.exe

scriptparser2.exe: scriptparser2.cpp script_unroller.h
	g++ -std=c++11 scriptparser2.cpp -o scriptparser2.exe

test_cmd_script_parser.exe: test_cmd_script_parser.cpp psweep_cmd_script_parser.h script_unroller.h
	$(CC) $(CFLAGS) $(INCLUDEDIR) test_cmd_script_parser.cpp -o test_cmd_script_parser.exe

clean:
	rm *.exe *.o
