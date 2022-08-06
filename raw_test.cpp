// to find the keyboard mapping of special keys
#include <iostream>
#include <termios.h>

int main(int argc, char* argv[])
{
	struct termios oldt, newt;
	tcgetattr(0, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &newt);

	std::cout << "Welcome\n";
	char c;
	do{
		c = getchar();
		printf("%d\n", c);
	}while(c != 'x');

	std::cout << "Bye!!\n";
	tcsetattr(0, TCSANOW, &oldt);
}