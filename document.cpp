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
		lp->count();
		return;
	}

	auto iter = lines.begin();
	std::advance(iter, pos);
	lines.insert(iter, lp);
	lp->count();

	iter = lines.begin();
	while(*iter != lp) ++iter;
	while(++iter != lines.end())
		(*iter)->position++;
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