#include "tedit.hpp"

Document::Document() {
	lines.push_back(new Line("", 1));
	cur_line = lines.begin();
}

Document::Document(std::fstream& fs)
{
	if(fs.bad()) throw 1;
	if(not fs or not fs.is_open()){
		lines.push_back(new Line("", 1));
		cur_line = lines.begin();
		return;
	}

	std::string str;
	while(not fs.eof()){
		std::getline(fs, str);
		add_new_line(str, -1);
	}

	cur_line = lines.end();
	--cur_line;
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
		lp->position = lines.size();
		return;
	}

	auto iter = lines.begin();
	std::advance(iter, pos);
	lines.insert(iter, lp);

	std::for_each(iter, lines.end(), [](auto x){x->position++;});
}

void Document::read_file(std::fstream& fs)
{
	// delete old contents
	for(Line* l: lines)
		delete l;
	lines.clear();

	std::string str;
	while(not fs.eof()){
		std::getline(fs, str);
		add_new_line(str, -1);
	}

	cur_line = lines.end();
	--cur_line;
}

void Document::save(std::fstream& fs)
{
	for(Line* l: lines){
		fs << l;
		if(l != lines.back())
			fs << '\n';
	}
}

void Document::insert(char c, size_t idx)
{
	(*cur_line)->insert(idx, 1, c);
	(*cur_line)->render();
}

void Document::backspace(size_t idx)
{
	if(idx <= 0)
		return;
	(*cur_line)->erase(idx - 1, 1);
	(*cur_line)->render();
}

void Document::del_char(size_t idx)
{
	if(idx >= (*cur_line)->length())
		return;
	(*cur_line)->erase(idx, 1);
	(*cur_line)->render();
}

void Document::remove_line(std::__cxx11::list<Line *>::iterator pos)
{
	if(pos == cur_line)
		--cur_line;

	std::for_each(pos, lines.end(), [](auto x){x->position--;});
	delete *pos;
	lines.erase(pos);
	render();
}
