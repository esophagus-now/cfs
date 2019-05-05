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
jNN:MM - Select memory at address held in rNN offst by MM ("small j addressing")
IAAAA - Select memory at address held in memory location AAAA ("big I addressing")
JAAAA:MM -Like big I addressing, but at an offset of MM ("big J addressing")

Operations:
c A B 		- Copy from location B to location A (a "mov" instruction)
			- Example: cm5555r3 means copy the value in r3 to memory address 5555

b N A B C	- Perform binary operation N, for A = B N C
			- Example: b-r4r4m55 means r4 = r4 - m[55]
			- Operations are +-/*&|^ (with their usual meaning)

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

#define LOC_TYPES X(REG), X(MEM), X(REGIND), X(REGINDOFF), X(MEMIND), X(MEMINDOFF)
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
	};
};

ostream& operator<< (ostream &o, loc const& l) {
	switch(l.type) {
		case REG:
			return o << "(register " << l.reg.r << ")";
		case MEM:
			return o << "(memory at " << l.mem.addr << ")";
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
			puts("Not implemented (i)");
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
		default: 
			puts("Bad location");
			return -1;
	}
	return pos;
}

//Return number of characters read
int parse(char const *str, vector<instr> &parsed) {
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
		case 0:
			puts("done");
			return 0;
		default:
			puts("Not implemented");
			return -1;
	}
	parsed.push_back(move(res)); //Right now, the instr type is POD, so moving doesn't do anything.
	//Actually, I only wrote it because it was easy. I don't actally care about performance here.
	return pos;
}

int main() {
	char const* test = " cr4mBEEFs75b-r3r4r7cr4r5u-r1r1";
	int pos = 0;
	int tmp;
	vector<instr> parsed;
	while((tmp = parse(test+pos, parsed)) > 0) pos += tmp;
	cout << parsed << endl;
}
