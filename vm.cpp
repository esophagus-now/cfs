#include "vm.h"
#include <iostream>

using namespace std;

char const *op_names[] = {
#define X(x) #x
	OPS
#undef X
};

char const *loc_type_names[] = {
#define X(x) #x
	LOC_TYPES
#undef x	
};

ostream& operator<< (ostream &o, loc const& l) {
	switch(l.type) {
		case REG:
			return o << "(register " << l.reg.r << ")";
		case MEM:
			return o << "(memory at " << l.mem.addr << ")";
		case REGIND:
			return o << "(memory at indexed by r" << l.regind.r << ")";
		case IMM:
			return o << "(immediate value " << l.imm.val << ")";
		default:
			return o << "(Bad location [" << l.type << "])";
	}
}

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
static int readLoc(loc const& l, vm const& m) {
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
static int writeLoc(loc const& l, vm &m, int val) {
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

static int setFlags(int val, vm &m) {
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
