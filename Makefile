cfs:	main.cpp vm.h vm.cpp parse.h parse.cpp
	g++ -o cfs -g -Wall -fno-diagnostics-show-caret main.cpp vm.cpp parse.cpp
