#include "tedit.hpp"

int move_to(int y, int x)
{
	printf("%c[%d;%dH", 0x1B, y + 1, x + 1);
	return 0;
}


Line::Line(std::string str, size_t pos) : data(str), position(pos) { }

Line::~Line() { }

void Line::render()
{
	move_to(position - 1, 0);
	printf("%c[%dK", 0x1B, 2);
	std::cout << data;
	std::cout.flush();
}

void Line::count()
{
	tab_count = std::count(data.begin(), data.end(), '\t');
	indent_level = data.find_first_not_of('\t');
}