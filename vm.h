#ifndef VM_H
#define VM_H 1

#include <iostream>
#include <vector>

/*
Simple easy-to-lex-and-parse instruction set

Addressing syntax:
rNN - Select register NN
mAAAA - Select memory at address AAAA
iNN   - Select memory at address held in rNN ("small i addressing")
jNN MM - Select memory at address held in rNN offst by MM ("small j addressing")
IAAAA - Select memory at address held in memory location AAAA ("big I addressing")
JAAAA MM -Like big I addressing, but at an offset of MM ("big J addressing")
vNN	- Use the value NN as an immediate

Operations:
c A B 		- Copy from location B to location A (a "mov" instruction)
			- Example: cm5555r3 means copy the value in r3 to memory address 5555

b N A B C	- Perform binary operation N, for A = B N C
			- Example: b-r4r4m55 means r4 = r4 - m[55]
			- Operations are +*-/&|^% (with their usual meaning)

u N A B		- Perform unary operation N, for A = N B
			- Example: u-r4r4 means r4 = -r4
			- Operations are +-!~ (with their usual meaning, with the exception of + which is the absolute value)

Note: the operations set the typical flags based on the result

j[znpu] NN	- if the (selected) flag is set, jump by NN instructions (can be negative). Use "u" for "uncoditional"

s N			- Perform "system call" number N
			- 0: "Set an alarm": process becomes runnable after r1 ticks elapse
			- 1: "Read file": Let me think about this. Might be interesting to have a simple model of a hard disk
			- 2: "Write file": ditto
			- 3: "Test-and-set": process tests if mutex number r1 is zero, and atomically sets it if that is the case.
					     otherwise, the process will block until the mutex "frees up"
*/

#define OPS X(CPY), X(BOP), X(UOP), X(JZ), X(JN), X(JP), X(JU), X(SYSCALL)
typedef enum {
#define X(x) x
	OPS
#undef X
} op_t;
extern char const *op_names[];

#define LOC_TYPES X(REG), X(MEM), X(REGIND), X(REGINDOFF), X(MEMIND), X(MEMINDOFF), X(IMM)
typedef enum {
#define X(x) x
	LOC_TYPES
#undef X
} loc_t;
extern char const *loc_type_names[];

struct loc {
	loc_t type;
	union {
		struct {int r;} reg;
		struct {int addr;} mem;
		struct {int r;} regind;
		struct {int r; int off;} regindoff;
		struct {int addr;} memind;
		struct {int addr; int off;} memindoff;
		struct {int val;} imm;
	};
};

std::ostream& operator<< (std::ostream &o, loc const& l);

struct instr {
	op_t type;
	union {
		struct {loc dst; loc src;} cpy;
		struct {char op; loc dst; loc src1; loc src2;} bop;
		struct {char op; loc dst; loc src;} uop;
		struct {int off;} jz, jn, jp, ju;
		struct {int num;} syscall;
	};
};

std::ostream& operator<< (std::ostream &o, instr const& i);

#define ADDR_WIDTH 8
#define ADDR_MASK ((1 << ADDR_WIDTH) -1)
#define NUM_REGS   16
struct vm {
	int regs[NUM_REGS];
	int mem[1 << ADDR_WIDTH];
	struct {
		unsigned Z	:1;
		unsigned N	:1;
		unsigned P	:1;
		
	} flags;
	
	vm() {
		for (int i = 0; i < NUM_REGS; i++) regs[i] = 0;
		for (int i = 0; i <= ADDR_MASK; i++) mem[i] = 0;
	}
};

std::ostream& operator<< (std::ostream &o, vm const& v);

int stepProg(std::vector<instr> const& prog, vm &state, int pos);

int runProg(std::vector<instr> const& prog, vm &state);


#endif
