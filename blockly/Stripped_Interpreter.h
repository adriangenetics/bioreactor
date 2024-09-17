#pragma once
#include <vector>
#include <string>

/*
* Summary:
* I looked into it, and it's really hard to do any kind of dynamic linking or similar stuff on an ESP32 where we can call compiled C++ from the main 
* interface. Instead, I made this solution. It's a very simple interpreted bytecode that we can convert the blocks to and then run on the ESP32. 
* 
* One potential advantage to this approach is that we could save multiple programs on the ESP32 and change the UI so the user can pick which program to
* run. 
*/


//Interprets the program (given as bytes so we can send it over a wireless connection easier)
void RunProgram(std::vector<unsigned char> Data);

#define Testing
#ifdef Testing
//Assembles a program from the source string (used for testing)
std::vector<unsigned char> Assemble(std::string Text);
#endif