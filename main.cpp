#include <iostream>
#include <vector>
#include <string>
#include <cstdio>

using namespace std;

// Simple easy-to-lex-and-parse instruction set
//
// Addressing syntax:
// rNN - Select register NN
// mAAAA - Select memory at address AAAA
// iNN   - Select memory at address held in rNN ("small i addressing")
// jNN:MM - Select memory at address held in rNN offst by MM ("small j addressing")
// IAAAA - Select memory at address held in memory location AAAA ("big I addressing")
// JAAAA:MM -Like big I addressing, but at an offset of MM ("big J addressing")

// Operations:
// c A B 		- Copy from location B to location A (a "mov" instruction)
//				- Example: Cm5555r3 means copy the value in r3 to memory address 5555

// b N A B C	- Perform binary operation N, for A = B N C
//				- Example: b-r4r4m55 means r4 = r4 - m[55]
//				- Operations are +-*/&|^ (with their usual meaning)

// u N A B		- Perform unary operation N, for A = N B
//				- Example: u-r4r4 means r4 = -r4
//				- Operations are +-!~ (with their usual meaning, with the exception of + which is the absolute value)
	
// Note: the operations set the typical flags based on the result

// j[znp] NN	- if the (selected) flag is set, jump by NN instructions (cane be negative).

// s N			- Perform "system call" number N
//				- 0: "Set an alarm": process becomes runnable after r1 ticks elapse
//				- 1: "Read file": Let me think about this. Might be interesting to have a simple model of a hard disk
//				- 2: "Write file": ditto
//				- 3: "Test-and-set": process tests if mutex number r1 is zero, and atomically sets it if that is the case.
										//otherwise, the process will block until the mutex "frees up"

int skipSpaces(char const* str) {
	int pos = 0;
	while (str[pos] == ' ' || str[pos] == '\n' || str[pos] == '\r') pos++;
	return pos;
}

int printLocation(char const* str) {
	int pos = skipSpaces(str);
	int tmp;
	int num;
	switch(str[pos++]) {
		case 'r':
			printf("register ");
			sscanf(str + pos, "%d%n", &tmp, &num);
			pos += num;
			printf("%d", tmp);
			break;
		case 'm':
			printf("memory at ");
			sscanf(str + pos, "%x%n", &tmp, &num);
			pos += num;
			printf("0x%X", tmp);
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
int parsenprint(char const *str) {
	int pos = 0;
	//Skip past spaces
	pos += skipSpaces(str);
	int tmp;
	switch(str[pos++]) {
		case 'c':
			printf("Copy into ");
			pos += printLocation(str + pos);
			printf(" from ");
			pos += printLocation(str + pos);
			printf("\n");
			break;
		case 'b':
			printf("Binary op: ");
			pos += skipSpaces(str + pos);
			tmp = str[pos++];
			pos += printLocation(str + pos);
			printf(" = ");
			pos += printLocation(str + pos);
			printf(" %c ", tmp);
			pos += printLocation(str + pos);
			printf("\n");
			break;
		case 'u':
			printf("Unary op: ");
			pos += skipSpaces(str + pos);
			tmp = str[pos++];
			pos += printLocation(str + pos);
			printf(" = ");
			printf(" %c", tmp);
			pos += printLocation(str + pos);
			printf("\n");
			break;
		case 's':{
			int num;
			sscanf(str + pos, "%d%n", &num, &tmp);
			pos += tmp;
			printf("Syscall #%d\n", num);
			break;
		}
		case 0:
			puts("done");
			return 0;
		default:
			puts("Not implemented");
			return -1;
	}
	
	return pos;
}

int main() {
	char const* test = " cr4mBEEFs75b-r3r4r7cr4r5u-r1r1";
	int pos = 0;
	int tmp;
	while((tmp = parsenprint(test+pos)) > 0) {
		//printf("tmp = %d\n", tmp);
		pos += tmp;
		printf("\n");
	}
	cout << "Hello world" << endl;
}
