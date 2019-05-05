#include "parse.h"
#include "vm.h"
#include <cstdio>
#include <vector>

using namespace std;

static int skipSpaces(char const* str) {
	int pos = 0;
	while (str[pos] == ' ' || str[pos] == '\n' || str[pos] == '\r') pos++;
	return pos;
}

static int parseLoc(char const* str, loc &res) {
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
			printf("Bad location [%c]\n", str[pos-1]);
			return -1;
	}
	return pos;
}

//Return number of characters read
static int parseInstr(char const *str, vector<instr> &parsed) {
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
