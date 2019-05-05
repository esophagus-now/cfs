#include <iostream>
#include <vector>
#include <string>
#include <cstdio> //For sscanf
#include <utility> //For std::move
#include <queue> //For priority_queue
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

#define TASK_STATES X(RUNNING), X(READY), X(WAITING)
typedef enum {
#define X(x) x
	TASK_STATES
#undef X
} task_state_t;
char const *task_state_names[] = {
#define X(x) #x
	TASK_STATES
#undef X
};

struct task {
	string name;
	vector<instr> text;
	int pos;
	int regSave[NUM_REGS];
	
	task_state_t state;
	int weight;
	int vruntime; 
};

ostream& operator<< (ostream &o, task const& t) {
	return o << t.name << ": " << task_state_names[t.state] << ", w=" << t.weight << ", vr=" << t.vruntime;
}

bool operator< (task const& t1, task const& t2) {
	return t1.vruntime < t2.vruntime;
}

int main() {
	char const* fib = 
		"cr3v20" //Loop count
		"cr2v2"  //r2 is the address we are currently processing
		"cr1v1"  //r1 and r0 are the initial two Fibonacci numbers
		"cr0v0"
		"cm0r0" //Actually write the first two numbers into memory
		"cm1r1"
		//Main loop:
		"b-r3r3v1" //Decrement r3
		"jn20" //Quit if loop is finished
		"b+r5r0r1" //Compute next Fibonacci number
		"ci2r5" //Store it in memory
		"b+r2r2v1" //Increment r2
		"cr0r1" //Shift down
		"cr1r5"
		"ju-8" //Restart main loop
		;
	char const* lucas = 
		"cr3v20" //Loop count
		"cr2v52"  //r2 is the address we are currently processing
		"cr1v1"  //r1 and r0 are the initial two Lucas numbers
		"cr0v2"
		"cm32r0" //Actually write the first two numbers into memory
		"cm33r1"
		//Main loop:
		"b-r3r3v1" //Decrement r3
		"jn9" //Quit if loop is finished
		"b+r5r0r1" //Compute next Fibonacci number
		"ci2r5" //Store it in memory
		"b+r2r2v1" //Increment r2
		"cr0r1" //Shift down
		"cr1r5"
		"ju-8" //Restart main loop
		;
	char const* cnt = 
		"cr3v20" //Loop count
		"cr2v100" //Start address
		"cr1v1" //First number
		//Main loop:
		"b-r3r3v1" //Decrement r3
		"jn20" //Quit if loop is finished
		"ci2r1" //mem[r2] = r1
		"b+r1r1v1" //Increment r1
		"b+r2r2v1" //Increment r2
		"ju-6" //Restart main loop
		;
		
	
	vector<instr> parsed1 = parseProg(fib);
	vector<instr> parsed2 = parseProg(lucas);
	vector<instr> parsed3 = parseProg(cnt);
	cout << "Fibonacci program: " << parsed1 << endl;
	cout << "Lucas program: " << parsed2 << endl;
	cout << "Iota program: " << parsed3 << endl;
	vm init;
	
	task t1 = {
		.name = "Fibonacci",
		.text = parsed1,
		.pos = 0,
		.state = READY,
		.weight = 5,
		.vruntime = 0
	};
	task t2 = {
		.name = "Lucas",
		.text = parsed2,
		.pos = 0,
		.state = READY,
		.weight = 4,
		.vruntime = 0
	};
	task t3 = {
		.name = "Iota",
		.text = parsed3,
		.pos = 0,
		.state = READY,
		.weight = 1,
		.vruntime = 0
	};
	
	vector<task> masterlist = {t1, t2, t3};
	priority_queue<task> pq;
	//pq.push(t1); pq.push(t2); pq.push(t3);
	for (auto const& i : masterlist) {
		cout << i << endl;
		pq.push(i);
	}
	
	runProg(parsed1, init);
	cout << init;
	runProg(parsed2, init);
	cout << init;
	runProg(parsed3, init);
	cout << init;
	
	
	return 0;
}
