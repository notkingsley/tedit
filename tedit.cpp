#include <iostream>
#include <cstdio>
#include <vector>
#include <termios.h>

#include "tedit.hpp"

void driver()
{
	Manager me;
	me.listen();
}

int main(int argc, char* argv[])
{
	driver();
	return 0;
}