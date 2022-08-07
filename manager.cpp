#include "tedit.hpp"

Manager::Manager() : 
	name("untitled.txt"),
	doc(Document()), cur_line(doc.cur_line)
{
	system("touch untitled.txt");
	scurx = curx = 0;
	scury = cury = 0;

	// save old terminal state, then disable buffering
	// and echo
	struct termios newt;
	tcgetattr(0, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &newt);
}

Manager::Manager(const char* filename) :
	name(filename), file(filename), 
	doc(file), cur_line(doc.cur_line)
{
	file.close();

	char command[strlen(filename) + 10] = "touch ";
	strcat(command, filename);
	system(command);

	doc.render();
	scury = cury = (*cur_line)->position - 1;
	curx = (*cur_line)->data.length();
	update_scur();
	move_to(scury, scurx);

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
		}else if(c == 115){							// alt-s or esc + s
			return {CharType::SAVE, c};
		}else if(c == 99){							// alt-s or esc + c
			return {CharType::DISCARD_SESSION, c};
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
	// clear the screen
	printf("%c[%dJ", 0x1B, 2);
	doc.render();
	move_to(scury, scurx);

	// present action
	std::pair<CharType, char> act;
	bool loop = true;

	while(loop){
		act = get_next();
		switch(act.first){
			case CharType::PRINTABLE:
			{
				key_printable(act.second);
				break;
			}
			case CharType::ARROW_KEY:
			{
				key_arrow(act.second);
				break;
			}
			case CharType::BACKSPACE:
			{
				key_backspace();
				break;
			}
			case CharType::DELETE_KEY:
			{
				key_delete();
				break;
			}
			case CharType::ENTER_KEY:
			{
				key_enter();
				break;
			}
			case CharType::TAB_KEY:
			{
				key_tab();
				break;
			}
			case CharType::SAVE:
			{
				save();
				break;
			}
			case CharType::DISCARD_SESSION:
			{
				loop = false;
				break;
			}
			case CharType::EXIT:
			{
				save();
				loop = false;
				break;
			}
		}
		move_to(scury, scurx);
	}
	std::cout << "\nBye\n";
}

void Manager::save()
{
	file = std::fstream(name, std::ios::out | std::ios::trunc);
	for(Line* l: doc.lines){
		file << l->data;
		if(l != doc.lines.back())
			file << '\n';
	}
	file.close();
}

void Manager::key_printable(char c)
{
	(*cur_line)->data.insert(curx, 1, c);
	++curx, ++scurx;
	(*cur_line)->render();
}

void Manager::key_arrow(char c)
{
	switch(c){
		case 'A':
			if(cur_line != doc.lines.begin()){
				--cur_line;
				--cury;
				curx = std::min(curx, (*cur_line)->data.length());
				update_scur();
			}
			break;
		case 'B':
			if(*cur_line != doc.lines.back()){
				++cur_line;
				++cury;
				curx = std::min(curx, (*cur_line)->data.length());
				update_scur();
			}
			break;
		case 'C':
			if(curx < (*cur_line)->data.length()){
				++curx;
				update_scur();
			}else if(*cur_line != doc.lines.back()){
				++cur_line;
				++cury;
				curx = 0;
				update_scur();
			}
			break;
		case 'D':
			if(curx > 0){
				--curx;
				update_scur();
			}else if(cur_line != doc.lines.begin()){
				--cur_line;
				--cury;
				curx = (*cur_line)->data.length();
				update_scur();
			}
			break;
	}
}

void Manager::key_backspace()
{
	if(curx > 0){								// same line
		(*cur_line)->data.erase((*cur_line)->data.begin() + curx - 1);
		(*cur_line)->render();
		--curx;
		update_scur();
	}else if(cur_line != doc.lines.begin()){	// delete from prev line
		auto hold = cur_line;
		--cur_line;
		--cury;
		curx = (*cur_line)->data.length();

		(*cur_line)->data += (*hold)->data;
		doc.lines.erase(hold);
		delete *hold;
		doc.render();
		update_scur();

		// clean last line
		move_to(cury + 1, 0);
		printf("%c[%dK", 0x1B, 2);
	}
}

void Manager::key_delete()
{
	if(curx < (*cur_line)->data.length()){		// same line
		(*cur_line)->data.erase((*cur_line)->data.begin() + curx);
		(*cur_line)->render();
		update_scur();
	}else if(*cur_line != doc.lines.back()){	// delete from next line
		auto next = cur_line;
		++next;

		(*cur_line)->data += (*next)->data;
		doc.lines.erase(next);
		delete *next;
		doc.render();
		update_scur();

		// clean last line
		move_to(cury + 1, 0);
		printf("%c[%dK", 0x1B, 2);
	}
}

void Manager::key_enter()
{
	// create a new line with the [curx:] slice of cur_line, adjust cury
	doc.add_new_line((*cur_line)->data.substr(curx), ++cury);
	// delete the [curx:] slice from cur_line
	(*cur_line)->data.erase((*cur_line)->data.begin() + curx, (*cur_line)->data.end());
	// adjust curx and cur_line
	++cur_line;
	curx = 0;
	doc.render();
	update_scur();
}

void Manager::key_tab()
{
	(*cur_line)->data.insert(curx, 1, '\t');
	curx += 1;
	(*cur_line)->render();
	update_scur();
}

inline void Manager::update_scur()
{

	// offset from last tab margin
	// and total gaps incurred by \t s
	size_t offset = 0, total_gap = 0;
	int passes = 0;
	for(auto c: (*cur_line)->data){
		if(passes++ == curx) break;
		++offset;
		offset %= TAB_SIZE;
		if(c == '\t'){
			total_gap += TAB_SIZE - offset;
			offset = 0;
		}
	}

	scurx = curx + total_gap;
	scury = cury;
}