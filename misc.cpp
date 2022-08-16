#include "tedit.hpp"

void move_to(int y, int x)
{
	Renderer::move_to(y, x);
}

Line::Line(std::string str, size_t pos) : std::string(str), position(pos) { }

Line::~Line() { }

void Line::render()
{
	Renderer::render_line(this);
}