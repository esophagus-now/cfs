#include <iostream>
#include <vector>
#include <string>
#include <cstdio> //For sscanf
#include <utility> //For std::move

using namespace std;

template <typename T>
ostream& operator<< (ostream &o, vector<T> const& v) {
	o << "{";
	for (auto const& i: v) {
		o << "\n\t" << i;
	}
	return o << "\n}";
}
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
char const *op_names[] = {
#define X(x) #x
	OPS
#undef X
};

#define LOC_TYPES X(REG), X(MEM), X(REGIND), X(REGINDOFF), X(MEMIND), X(MEMINDOFF), X(IMM)
typedef enum {
#define X(x) x
	LOC_TYPES
#undef X
} loc_t;
char const *loc_type_names[] = {
#define X(x) #x
	LOC_TYPES
#undef x	
};

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

ostream& operator<< (ostream &o, loc const& l) {
	switch(l.type) {
		case REG:
			return o << "(register " << l.reg.r << ")";
		case MEM:
			return o << "(memory at " << l.mem.addr << ")";
		case IMM:
			return o << "(immediate value " << l.imm.val << ")";
		default:
			return o << "(Bad location [" << l.type << "])";
	}
}

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

ostream& operator<< (ostream &o, instr const& i) {
	switch(i.type) {
		case CPY:
			return o << "Copy: " << i.cpy.dst << " = " << i.cpy.src;
		case BOP:
			return o << "Binop: " << i.bop.dst << " = " << i.bop.src1 << i.bop.op << i.bop.src2;
		case UOP:
			return o << "Unary op: " << i.uop.dst << " = " << i.uop.op << i.uop.src;
		case JZ:
			return o << "Jump if 0: " << i.jz.off;
		case JN:
			return o << "Jump if < 0: " << i.jn.off;
		case JP:
			return o << "Jump if > 0: " << i.jp.off;
		case JU:
			return o << "Jump: " << i.ju.off;
		case SYSCALL:
			return o << "Syscall: " << i.syscall.num;
		default:
			return o << "Bad instruction: [" << i.type;
	}
}

int skipSpaces(char const* str) {
	int pos = 0;
	while (str[pos] == ' ' || str[pos] == '\n' || str[pos] == '\r') pos++;
	return pos;
}

int parseLoc(char const* str, loc &res) {
	int pos = skipSpaces(str);
	int tmp;
	switch(str[pos++]) {
		case 'r':
			res.type = REG;
			//printf("register ");
			sscanf(str + pos, "%d%n", &(res.reg.r), &tmp);
			pos += tmp;
			//printf("%d", res.reg.r);
			break;
		case 'm':
			res.type = MEM;
			//printf("memory at ");
			sscanf(str + pos, "%x%n", &(res.mem.addr), &tmp);
			pos += tmp;
			//printf("0x%X", res.mem.addr);
			break;
		case 'i':
			res.type = REGIND;
			sscanf(str + pos, "%x%n", &(res.regind.r), &tmp);
			pos += tmp;
			break;
		case 'j':
			puts("Not implemented (j)");
			break;
		case 'I':
			puts("Not implemented (I)");
			break;
		case 'J':
			puts("Not implemented (J)");
			break;
		case 'v':
			res.type = IMM;
			sscanf(str + pos, "%d%n", &(res.imm.val), &tmp);
			pos += tmp;
			break;
		default: 
			puts("Bad location");
			return -1;
	}
	return pos;
}

//Return number of characters read
int parseInstr(char const *str, vector<instr> &parsed) {
	int pos = 0;
	//Skip past spaces
	pos += skipSpaces(str);
	int tmp;
	instr res;
	switch(str[pos++]) {
		case 'c':
			res.type = CPY;
			//printf("Copy into ");
			pos += parseLoc(str + pos, res.cpy.dst);
			//printf(" from ");
			pos += parseLoc(str + pos, res.cpy.src);
			//printf("\n");
			break;
		case 'b':
			res.type = BOP;
			//printf("Binary op: ");
			pos += skipSpaces(str + pos);
			res.bop.op = str[pos++];
			pos += parseLoc(str + pos, res.bop.dst);
			//printf(" = ");
			pos += parseLoc(str + pos, res.bop.src1);
			//printf(" %c ", res.bop.op);
			pos += parseLoc(str + pos, res.bop.src2);
			//printf("\n");
			break;
		case 'u':
			res.type = UOP;
			//printf("Unary op: ");
			pos += skipSpaces(str + pos);
			res.uop.op = str[pos++];
			pos += parseLoc(str + pos, res.uop.dst);
			//printf(" = ");
			//printf(" %c", res.uop.op);
			pos += parseLoc(str + pos, res.uop.src);
			//printf("\n");
			break;
		case 's':
			res.type = SYSCALL;
			sscanf(str + pos, "%d%n", &(res.syscall.num), &tmp);
			pos += tmp;
			//printf("Syscall #%d\n", res.syscall.num);
			break;
		case 'j':
			switch (str[pos++]) {
				case 'z': case 'Z':
					res.type = JZ;
					sscanf(str + pos, "%d%n", &(res.jz.off), &tmp);
					pos += tmp;
					break;
				case 'n': case 'N':
					res.type = JN;
					sscanf(str + pos, "%d%n", &(res.jn.off), &tmp);
					pos += tmp;
					break;
				case 'p': case 'P':
					res.type = JP;
					sscanf(str + pos, "%d%n", &(res.jp.off), &tmp);
					pos += tmp;
					break;
				case 'u': case 'U':
					res.type = JU;
					sscanf(str + pos, "%d%n", &(res.ju.off), &tmp);
					pos += tmp;
					break;
				default:
					printf("Unimplemented jump type [%c]\n", str[pos - 1]);
					return -1;
			}
			break;
		case 0:
			//puts("done");
			return 0;
		default:
			printf("Unimplemented instruction [%c]\n", str[pos - 1]);
			return -1;
	}
	parsed.push_back(move(res)); //Right now, the instr type is POD, so moving doesn't do anything.
	//Actually, I only wrote it because it was easy. I don't actally care about performance here.
	return pos;
}

vector<instr> parseProg(char const* text) {
	int pos = 0;
	int tmp;
	vector<instr> ret;
	while((tmp = parseInstr(text+pos, ret)) > 0) pos += tmp;
	return ret;
}

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

ostream& operator<< (ostream &o, vm const& v) {
	for (int i = 0; i < NUM_REGS; i++) {
		o << "Reg " << i << ": " << v.regs[i] << "\n";
	}
	for (int i = 0; i <= ADDR_MASK; i++) {
		o << v.mem[i] << "\t";
	}
	return o << "\n";
}

//WARNING: no sanity checks performed!!!
//LOC_TYPES X(REG), X(MEM), X(REGIND), X(REGINDOFF), X(MEMIND), X(MEMINDOFF), X(IMM)
int readLoc(loc const& l, vm const& m) {
	switch(l.type) {
		case REG:
			return m.regs[l.reg.r];
		case MEM:
			return m.mem[l.mem.addr];
		case REGIND:
			return m.mem[m.regs[l.regind.r]];
		case REGINDOFF:
			return 0;
		case MEMIND:
			return 0;
		case MEMINDOFF:
			return 0;
		case IMM:
			return l.imm.val;
	}
	
	return 0;
}
//WARNING: no sanity checks performed!!!
//LOC_TYPES X(REG), X(MEM), X(REGIND), X(REGINDOFF), X(MEMIND), X(MEMINDOFF), X(IMM)
int writeLoc(loc const& l, vm &m, int val) {
	switch(l.type) {
		case REG:
			m.regs[l.reg.r] = val;
			break;
		case MEM:
			m.mem[l.mem.addr] = val;
			break;
		case REGIND:
			m.mem[m.regs[l.regind.r]] = val;
			break;
		case REGINDOFF:
			return -1;
		case MEMIND:
			return -1;
		case MEMINDOFF:
			return -1;
		case IMM:
			return -1; //This doesn't make sense!
	}
	return 0;
}

int setFlags(int val, vm &m) {
	if (val < 0) {
		m.flags.Z = 0;
		m.flags.N = 1;
		m.flags.P = 0;
	} else if (val == 0) {
		m.flags.Z = 1;
		m.flags.N = 0;
		m.flags.P = 0;
	} else {
		m.flags.Z = 0;
		m.flags.N = 0;
		m.flags.P = 1;
	}
	return 0;
}

int stepProg(vector<instr> const& prog, vm &state, int pos) {
	if (pos >= int(prog.size())) return pos; //Succesful stop
	else if (pos < 0) return -1; //Error
	int tmp, tmp2;
	instr const& i = prog[pos++];
	switch(i.type) {
		case CPY:
			tmp = readLoc(i.cpy.src, state);
			setFlags(tmp, state);
			writeLoc(i.cpy.dst, state, tmp);
			break;
		case BOP:
			tmp = readLoc(i.bop.src1, state);
			tmp2 = readLoc(i.bop.src2, state);
			switch(i.bop.op) {
				case '+':
					tmp += tmp2;
					break;
				case '-':
					tmp -= tmp2;
					break;
				case '*':
					tmp *= tmp2;
					break;
				case '/':
					tmp /= tmp2;
					break;
				case '&':
					tmp &= tmp2;
					break;
				case '|':
					tmp |= tmp2;
					break;
				case '%':
					tmp %= tmp2;
					break;
				default:
					puts("Bad binop!");
					return -1;
			}
			setFlags(tmp, state);
			writeLoc(i.bop.dst, state, tmp);
			break;
		case UOP:
			tmp = readLoc(i.uop.src, state);
			switch(i.uop.op) {
				case '+':
					tmp = (tmp < 0) ? -tmp : tmp;
					break;
				case '-':
					tmp = -tmp;
					break;
				case '!':
					tmp = !tmp;
					break;
				case '~':
					tmp = ~tmp;
					break;
				default:
					puts("Bad uop!");
					return -1;
			}
			setFlags(tmp, state);
			writeLoc(i.uop.dst, state, tmp);
			break;
		case JZ:
			if (state.flags.Z) pos += i.jz.off;
			break;
		case JN:
			if (state.flags.N) pos += i.jn.off;
			break;
		case JP:
			if (state.flags.P) pos += i.jp.off;
			break;
		case JU:
			pos += i.ju.off;
			break;
		case SYSCALL:
			puts("Not implemented (SYSCALL)");
			break;
		default:
			puts("A terrible error has occurred!");
			return -1;
	}
	return pos;
}

//OPS X(CPY), X(BOP), X(UOP), X(JZ), X(JN), X(JP), X(JU), X(SYSCALL)
int runProg(vector<instr> const& prog, vm &state) {
	int pos = 0;
	while (pos >= 0 && pos < int(prog.size())) pos = stepProg(prog, state, pos);
	return pos < 0 ? -1 : 0;
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
