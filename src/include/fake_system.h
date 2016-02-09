#pragma once



//REV: fake system() call struct thing.
//Basically, takes a function (?) pointer? User implements it
//and adds it to the thing.
//Arguments are all in the form of variable=value. And user handles it.
//It takes a mem_filesystem struct as an argument too, so it can effectively
//read from that. Much nicer than passing a bunch of void pointers and
//forcing user to  handle them?
//Another alternative is to just pass VARLISTS that can include
//arbitrarily complex things.

//User can specify in the CMD script at beginning that it should execute
//this first (in other words, it should always check through some
//artificial_system list before actually calling system() on the target).

//So, user instead of saying ADD_TO_CMD, he says ADD_TO_MEM_CMD or something.
//And, he specifies a string that specifies which of the fake_system() structs
//to call. Furthermore, he adds stuff to it, like adds a specific "option"
//to be equal to a specific "value" (i.e. string, i.e. it takes a varlist?
//They will be parsed by user side just like argc and argv. But user will
//always read from fake filesystem. It's super ghetto, but at least everything
//will be called in memory, and we won't hit the LDAP for calling a .exe file
//each time, at least for C/C++ code (or dynamic links to other guys that
//we can establish once at startup time).

//This still requires user to be able to "encapsulate/restart" his simulation
//cleanly at each function call.

//So, format will be just a list of strings. And user can parse it as he
//wishes. Totally agnostic to etc. And user can simply wrap it in main
//if he wishes to do system() that way, in other words he can call
//other programming langauges, or just have main() call the program with
//the string array equal to argc/argv.

struct fake_system
{
  //Has a functor that user calls. Must take a fake FS/varlist (i.e. compile his code to overwrite typical system guys with my fake ones ). Whatever.
  //How does user read in values? Via BINARY or via DOUBLE.
};
