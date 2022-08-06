#include "tedit.hpp"

Manager::Manager() : file(nullptr), doc(Document()), cur_line(doc.cur_line)
{
	curx = 0;
	cury = 0;
	// save old terminal state, then disable buffering
	// and echo
	struct termios newt;
	tcgetattr(0, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &newt);
}

Manager::~Manager()
{
	tcsetattr(0, TCSANOW, &oldt);
	delete file;
}

std::pair<CharType, char> Manager::get_next()
{
	char c = getchar();
	if(c >= 32 && c <= 126){
		return {CharType::PRINTABLE, c};
	}else if(c == 27){								// escape key
		c = getchar();
		if(c == 91){								// arrow key or delete key
			c = getchar();
			if(c == 51){							// delete key, verify
				c = getchar();
				if(c == 126) return {CharType::DELETE_KEY, c};
			}else if(c >= 65 && c <= 68){
				return {CharType::ARROW_KEY, c};
			}
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
	doc.render();
	move_to(cury, curx);

	// present action
	std::pair<CharType, char> act;
	// iterator to the current line
	bool loop = true;

	while(loop){
		act = get_next();
		// reference to cur_line's string
		std::string& this_str = (*cur_line)->data;
		switch(act.first){
			case CharType::PRINTABLE:
			{
				this_str.insert(curx, 1, act.second);
				++curx;
				(*cur_line)->render();
				break;
			}
			case CharType::ARROW_KEY:
			{
				switch(act.second){
					case 'A':
						if(cur_line != doc.lines.begin()){
							--cur_line;
							--cury;
							curx = std::min(curx, (*cur_line)->data.length());
						}
						break;
					case 'B':
						if(*cur_line != doc.lines.back()){
							++cur_line;
							++cury;
							curx = std::min(curx, (*cur_line)->data.length());
						}
						break;
					case 'C':
						if(curx < this_str.length()){
							++curx;
						}else if(*cur_line != doc.lines.back()){
							++cur_line;
							++cury;
							curx = 0;
						}
						break;
					case 'D':
						if(curx > 0){
							--curx;
						}else if(cur_line != doc.lines.begin()){
							--cur_line;
							--cury;
							curx = (*cur_line)->data.length();
						}
						break;
				}
				break;
			}
			case CharType::BACKSPACE:
			{
				if(curx > 0){
					this_str.erase(this_str.begin() + curx - 1);
					(*cur_line)->render();
					--curx;
				}else if(cur_line != doc.lines.begin()){
					auto hold = cur_line;
					--cur_line;
					--cury;
					curx = (*cur_line)->data.length();
					(*cur_line)->data += this_str;
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
					(*cur_line)->render();
				}else if(*cur_line != doc.lines.back()){
					auto next = cur_line;
					++next;
					(*cur_line)->data += (*next)->data;
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
				++cur_line;
				curx = 0;
				break;
			}
			case CharType::TAB_KEY:
			{
				int len = this_str.length();
				this_str.insert(curx, 4, ' ');
				curx += 4;	// tab means 4 spaces
				(*cur_line)->render();
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
}