#include <iostream>
#include <cstdio>
#include <vector>
#include <termios.h>

#include "tedit.hpp"

int main(int argc, char* argv[])
{
	if(argc == 1){
		Manager me;
		me.listen();
	}
	else{
		Manager me(argv[1]);
		me.listen();
	}
	return 0;
}