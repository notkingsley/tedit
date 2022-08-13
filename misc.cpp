#include "tedit.hpp"

int move_to(int y, int x)
{
	printf("%c[%d;%dH", 0x1B, y + 1, x + 1);
	return 0;
}


Line::Line(std::string str, size_t pos) : std::string(str), position(pos) { }

Line::~Line() { }

void Line::render()
{
	move_to(position - 1, 0);
	printf("%c[%dK", 0x1B, 2);
	std::cout << *this;
	std::cout.flush();
}

