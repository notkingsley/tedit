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
	// match exactly
	if (c >= 32 && c <= 126)
	{
		return {CharType::PRINTABLE, c};
	}
	else if (c == 27)
	{ // escape key
		c = getchar();
		if (c == 91)
		{ // arrow key or delete key, or ctrl_key
			c = getchar();
			if (c == 51)
			{ // delete key or ctrl_delete, verify
				c = getchar();
				if (c == 126){
					return {CharType::DELETE_KEY, c};
				}
				else if(c == 59)
				{	// ctrl_delete possible, verify
					c = getchar();
					if(c == 53){
						c = getchar();
						if(c == 126)
							return {CharType::CTRL_DELETE, c};
					}
				}
			}
			else if (c >= 65 && c <= 68)
			{
				return {CharType::ARROW_KEY, c};
			}
			else if(c == 49)
			{	// possible ctrl_arrow
				c = getchar();
				if(c == 59){
					c = getchar();
					if(c == 53){
						c = getchar();
						if(c >= 65 && c <= 68){
							return {CharType::CTRL_ARROW, c};
						}
					}
				}
			}
		}
		else if (c == 115)
		{ // alt-s or esc + s
			return {CharType::SAVE, c};
		}
		else if (c == 99)
		{ // alt-c or esc + c
			return {CharType::DISCARD_SESSION, c};
		}
		else if( c == 100)
		{	// alt-d or esc-d or ctrl+del
			return {CharType::CTRL_DELETE, c};
		}
	}
	else if (c == 127)
	{ // backspace
		return {CharType::BACKSPACE, c};
	}
	else if (c == 10 || c == 13)
	{ // enter key
		return {CharType::ENTER_KEY, c};
	}
	else if (c == 9)
	{ // tab
		return {CharType::TAB_KEY, c};
	}
	else if (c == 2)
	{
		return {CharType::EXIT, c};
	}
	else if( c == 8 || c == 23)
	{
		return {CharType::CTRL_BACKSPACE, c};
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
			case CharType::CTRL_ARROW:
			{
				key_ctrl_arrow(act.second);
				break;
			}
			case CharType::CTRL_BACKSPACE:
			{
				key_ctrl_backspace();
				break;
			}
			case CharType::CTRL_DELETE:
			{
				key_ctrl_delete();
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
	move_to(doc.lines.size() + 1, 0);
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

	// evaluate indent level
	size_t indent = (*cur_line)->data.find_first_not_of('\t');
	if(indent == 18446744073709551615ul)
		indent = 0;
	if((*cur_line)->data.back() == '{')
		++indent;

	// adjust curx and cur_line
	++cur_line;
	curx = indent;

	// insert indent and render
	(*cur_line)->data.insert(0, indent, '\t');
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

void Manager::key_ctrl_arrow(char c)
{
	switch(c){
		case 'C':
		{
			// end of line?
			if(curx == (*cur_line)->data.length()){
				return key_arrow(c);
			}
			// skip all whitespace
			while(curx < (*cur_line)->data.length() 
				&& isblank((*cur_line)->data[curx])) ++curx;

			if(curx < (*cur_line)->data.length()){
				if(isalnum((*cur_line)->data[curx]))
					// letters
					while(curx < (*cur_line)->data.length() 
						&& isalnum((*cur_line)->data[curx])) ++curx;
				else if(ispunct((*cur_line)->data[curx]))
					// punctuations
					while(curx < (*cur_line)->data.length() 
						&& ispunct((*cur_line)->data[curx])) ++curx;
			}

			update_scur();
			break;
		}
		case 'D':
		{
			// beginning of line?
			if(curx == 0){
				return key_arrow(c);
			}
			// skip all whitespace
			while(curx > 0 && isblank((*cur_line)->data[curx - 1])) --curx;

			if(curx > 0){
				if(isalnum((*cur_line)->data[curx - 1]))
					// letters
					while(curx > 0 && isalnum((*cur_line)->data[curx - 1])) 
						--curx;
				else if(ispunct((*cur_line)->data[curx - 1]))
					// punctuations
					while(curx > 0 && ispunct((*cur_line)->data[curx - 1]))
						--curx;
			}

			update_scur();
			break;
		}

	}
}

void Manager::key_ctrl_backspace()
{
	// beginnng of line
	if(curx == 0){
		return key_backspace();
	}

	int start = curx;
	// skip whitespace
	while(curx > 0 && isblank((*cur_line)->data[curx - 1])) --curx;

	if(curx > 0){
		if(isalnum((*cur_line)->data[curx - 1]))
			while(curx > 0 && isalnum((*cur_line)->data[curx - 1])) --curx;
		else if(ispunct((*cur_line)->data[curx - 1]))
			while(curx > 0 && ispunct((*cur_line)->data[curx - 1])) --curx;
	}

	(*cur_line)->data.erase(curx, start - curx);
	update_scur();
	doc.render();
}

void Manager::key_ctrl_delete()
{
	// end of line?
	if(curx == (*cur_line)->data.length()){
		return key_delete();
	}

	int end = curx;
	// skip all whitespace
	while(end < (*cur_line)->data.length() 
		&& isblank((*cur_line)->data[end])) ++end;

	if(end < (*cur_line)->data.length()){
		if(isalnum((*cur_line)->data[end]))
			// letters
			while(end < (*cur_line)->data.length() 
				&& isalnum((*cur_line)->data[end])) ++end;
		else if(ispunct((*cur_line)->data[curx]))
			// punctuations
			while(end < (*cur_line)->data.length() 
				&& ispunct((*cur_line)->data[end])) ++end;
	}

	(*cur_line)->data.erase(curx, end - curx);
	update_scur();
	doc.render();
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