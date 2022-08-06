#include "tedit.hpp"

int move_to(int y, int x)
{
	printf("%c[%d;%dH", 0x1B, y + 1, x + 1);
	return 0;
}



Document::Document() {
	lines.push_back(new Line("", 1));
	cur_line = lines.begin();
 }

Document::~Document()
{
	for(Line* l: lines)
		delete l;
}

void Document::render()
{
	for(Line* l: lines)
		l->render();
}

void Document::add_new_line(std::string str, int pos)
{
	Line* lp = new Line(str, pos + 1);
	if(pos < 0){
		lines.push_back(lp);
		return;
	}
	auto iter = lines.begin();
	std::advance(iter, pos);
	lines.insert(iter, lp);
	iter = lines.begin();
	while(*iter != lp) ++iter;
	while(++iter != lines.end())
		(*iter)->position++;
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