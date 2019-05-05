#include <iostream>
#include <vector>
#include <string>
#include <cstdio> //For sscanf
#include <utility> //For std::move
#include "vm.h"
#include "parse.h"

using namespace std;

template <typename T>
ostream& operator<< (ostream &o, vector<T> const& v) {
	o << "{";
	for (auto const& i: v) {
		o << "\n\t" << i;
	}
	return o << "\n}";
}

int main() {
	char const* prog1 = 
		"cr3v20" //Loop count
		"cr2v2"  //r2 is the address we are currently processing
		"cr1v1"  //r1 and r0 are the initial two Fibonacci numbers
		"cr0v0"
		"cm0r0" //Actually write the first two numbers into memory
		"cm1r1"
		//Main loop:
		"b-r3r3v1" //Decrement r3
		"jn 20" //Quit if loop is finished
		"b+r5r0r1" //Compute next Fibonacci number
		"ci2r5" //Store it in memory
		"b+r2r2v1" //Increment r2
		"cr0r1" //Shift down
		"cr1r5"
		"ju-8" //Restart main loop
		;
	char const* prog2 = 
		"cr3v20" //Loop count
		"cr2v52"  //r2 is the address we are currently processing
		"cr1v1"  //r1 and r0 are the initial two Lucas numbers
		"cr0v2"
		"cm32r0" //Actually write the first two numbers into memory
		"cm33r1"
		//Main loop:
		"b-r3r3v1" //Decrement r3
		"jn 20" //Quit if loop is finished
		"b+r5r0r1" //Compute next Fibonacci number
		"ci2r5" //Store it in memory
		"b+r2r2v1" //Increment r2
		"cr0r1" //Shift down
		"cr1r5"
		"ju-8" //Restart main loop
		;
	
	vector<instr> parsed1 = parseProg(prog1);
	vector<instr> parsed2 = parseProg(prog2);
	cout << "Fibonacci program: " << parsed1 << endl;
	cout << "Kucas program: " << parsed2 << endl;
	vm init;
	runProg(parsed1, init);
	cout << init;
	runProg(parsed2, init);
	cout << init;
}
