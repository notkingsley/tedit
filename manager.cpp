#include <termios.h>

#include "tedit.hpp"

Manager::Manager() : file(nullptr), doc(Document())
{
	curx = 0;
	cury = 0;
}

Manager::~Manager()
{
	delete file;
}

std::pair<CharType, char> Manager::get_next()
{
	char c = getchar();
	if(c >= 32 && c <= 126){
		return {CharType::PRINTABLE, c};
	}else if(c == 27){	// escape key
		c = getchar();
		if(c == 91){	// arrow key or delete key
			c = getchar();
			if(c == 51){	// delete key, verify
				c = getchar();
				if(c == 126)
					return {CharType::DELETE_KEY, c};
			}else if(c >= 65 && c <= 68)
				return {CharType::ARROW_KEY, c};
		}
	}else if(c == 127){	// backspace
		return {CharType::BACKSPACE, c};
	}else if(c == 10 || c == 13){	// enter key
		return {CharType::ENTER_KEY, c};
	}else if(c == 9){	//tab
		return {CharType::TAB_KEY, c};
	}else if(c == 2){
		return {CharType::EXIT, c};
	}
	return {CharType::INVALID, c};
}


void Manager::listen()
{
	// save old terminal state, then disable buffering
	// and echo
	struct termios oldt, newt;
	tcgetattr(0, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &newt);

	doc.add_new_line("", cury);
	doc.render();
	move_to(cury, curx);

	// present action
	std::pair<CharType, char> act;
	// iterator to the current line
	auto this_line = doc.lines.begin();
	std::advance(this_line, cury);
	bool loop = true;

	while(loop){
		act = get_next();
		std::string& this_str = (*this_line)->data;
		switch(act.first){
			// reference to this_line's string
			case CharType::PRINTABLE:
			{
				this_str.insert(curx, 1, act.second);
				++curx;
				(*this_line)->render();
				break;
			}
			case CharType::ARROW_KEY:
			{
				switch(act.second){
					case 'A':
						if(this_line != doc.lines.begin()){
							--this_line;
							--cury;
							curx = std::min(curx, (*this_line)->data.length());
						}
						break;
					case 'B':
						if(*this_line != doc.lines.back()){
							++this_line;
							++cury;
							curx = std::min(curx, (*this_line)->data.length());
						}
						break;
					case 'C':
						if(curx < this_str.length()){
							++curx;
						}else if(*this_line != doc.lines.back()){
							++this_line;
							++cury;
							curx = 0;
						}
						break;
					case 'D':
						if(curx > 0){
							--curx;
						}else if(this_line != doc.lines.begin()){
							--this_line;
							--cury;
							curx = (*this_line)->data.length();
						}
						break;
				}
				break;
			}
			case CharType::BACKSPACE:
			{
				if(curx > 0){
					this_str.erase(this_str.begin() + curx - 1);
					(*this_line)->render();
					--curx;
				}else if(this_line != doc.lines.begin()){
					auto hold = this_line;
					--this_line;
					--cury;
					curx = (*this_line)->data.length();
					(*this_line)->data += this_str;
					doc.lines.erase(hold);
					doc.render();
					delete *hold;
					// clean last line
					move_to(cury + 1, 0);
					printf("%c[%dK", 0x1B, 2);
				}
				break;
			}
			case CharType::DELETE_KEY:
			{
				if(curx < this_str.length()){
					this_str.erase(this_str.begin() + curx);
					(*this_line)->render();
				}else if(*this_line != doc.lines.back()){
					auto next = this_line;
					++next;
					(*this_line)->data += (*next)->data;
					doc.lines.erase(next);
					doc.render();
					delete *next;
					// clean last line
					move_to(cury + 1, 0);
					printf("%c[%dK", 0x1B, 2);
				}
				break;
			}
			case CharType::ENTER_KEY:
			{
				doc.add_new_line(this_str.substr(curx), ++cury);
				this_str.erase(this_str.begin() + curx, this_str.end());
				doc.render();
				++this_line;
				curx = 0;
				break;
			}
			case CharType::TAB_KEY:
			{
				int len = this_str.length();
				this_str.insert(curx, 4, ' ');
				curx += 4;	// tab means 4 spaces
				(*this_line)->render();
				break;
			}
			case CharType::EXIT:
			{
				loop = false;
				break;
			}
		}
		move_to(cury, curx);
	}

	std::cout << "\nBye\n";
	tcsetattr(0, TCSANOW, &oldt);
}