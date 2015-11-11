CC=g++
INCLUDEDIR="-I./"
CFLAGS=-std=c++11


#default: scriptparser2.exe
default: test_cmd_script_parser.exe

scriptparser2.exe: scriptparser2.cpp
	g++ -std=c++11 scriptparser2.cpp -o scriptparser2.exe

test_cmd_script_parser.exe: test_cmd_script_parser.cpp psweep_cmd_script_parser.h
	$(CC) $(CFLAGS) $(INCLUDEDIR) test_cmd_script_parser.cpp -o test_cmd_script_parser.exe

clean:
	rm *.exe *.o
