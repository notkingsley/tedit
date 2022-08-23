#include "tedit.hpp"

Manager::Manager() : 
	name("untitled.txt"),
	doc(Document()), cur_line(doc.cur_line)
{
	valid_filename = false;
	scurx = curx = 0;
	scury = cury = 0;

	Renderer::initialise_renderer(&doc);
	Renderer::initialize_syntax_coloring(name);

	init_screen();
}

Manager::Manager(const char* filename) :
	file(filename), 
	doc(file), cur_line(doc.cur_line)
{
	file.close();
	strcpy(name, filename);
	valid_filename = true;

	#ifndef _WIN32
	char command[strlen(filename) + 10] = "touch ";
	#else
	char command[strlen(filename) + 10] = "copy NUL ";
	#endif
	strcat(command, filename);
	system(command);

	Renderer::initialise_renderer(&doc);
	Renderer::initialize_syntax_coloring(name);

	init_screen();

	doc.render();
	scury = cury = (*cur_line)->position - 1;
	curx = (*cur_line)->length();
	update_scur();
	move_to(scury, scurx);

}

Manager::~Manager()
{
	end_screen();
}


#ifndef _WIN32

void Manager::init_screen()
{
	// save old terminal state, then disable buffering
	// and echo
	struct termios newt;
	tcgetattr(0, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(0, TCSANOW, &newt);
	
	signal(SIGWINCH, Renderer::update_terminal_size);
}

void Manager::end_screen()
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
			{	// possible ctrl_arrow, or alt_arrow
				c = getchar();
				if(c == 59){
					c = getchar();
					if(c == 53){
						c = getchar();
						if(c >= 65 && c <= 68){
							return {CharType::CTRL_ARROW, c};
						}
					}
					else if(c == 51){
						c = getchar();
						if(c >= 65 && c <= 68){
							return {CharType::ALT_ARROW, c};
						}
					}
				}
			}
			else if(c == 70)
			{
				return {CharType::END_KEY, c};
			}
			else if(c == 72)
			{
				return {CharType::HOME_KEY, c};
			}
			else if(c == 53)
			{
				c = getchar();
				if(c == 126)
					return {CharType::PAGE_UP, c};
			}
			else if(c == 54)
			{
				c = getchar();
				if(c = 126)
					return {CharType::PAGE_DOWN, c};
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
		else if(c == 113)
		{	// alt-q
			return {CharType::EXIT, c};
		}
		else if(c == 83)
		{	// alt-shift-s
			return {CharType::SAVE_AS, c};
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
		return {CharType::PRINTABLE, c};
	}
	else if (c == 2)
	{	// ctrl-b
		return {CharType::EXIT, c};
	}
	else if( c == 8 || c == 23)
	{
		return {CharType::CTRL_BACKSPACE, c};
	}
	else if(c == 24)
	{
		return {CharType::CTRL_X, c};
	}
	else if(c == 31)
	{	// ctrl+/
		return {CharType::COMMENT_LINE, c};
	}
	return {CharType::INVALID, c};
}

#endif


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
			case CharType::HOME_KEY:
			{
				key_home();
				break;
			}
			case CharType::END_KEY:
			{
				key_end();
				break;
			}
			case CharType::PAGE_UP:
			{
				key_page_up();
				break;
			}
			case CharType::PAGE_DOWN:
			{
				key_page_down();
				break;
			}
			case CharType::CTRL_X:
			{
				key_ctrl_x();
				break;
			}
			case CharType::ALT_ARROW:
			{
				key_alt_arrow(act.second);
				break;
			}
			case CharType::COMMENT_LINE:
			{
				key_comment_line();
				break;
			}
			case CharType::SAVE:
			{
				save();
				break;
			}
			case CharType::SAVE_AS:
			{
				valid_filename = false;
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
		update_scur();
		move_to(scury, scurx);
	}
	move_to(doc.lines.size() + 1, 0);
}

void Manager::save()
{
	if(not valid_filename){
		// clear screen
		printf("%c[%dJ", 0x1B, 2);
		printf("%c[%d;%dH", 0x1B, 1, 1);

		#ifndef _WIN32
		int (*get)(void) = getchar;
		#else
		int (*get)(void) = getch;
		#endif

		std::string str;
		char c;
		do{
			std::cout << "\rEnter name to save as: " << str;
			std::cout.flush();
			c = get();
			if(isalnum(c) || ispunct(c))
				str += c;
			else if((c == 127 or c == 8) and not str.empty())
				str.pop_back();
			#ifndef _WIN32
			else if(c == 27)
				return doc.render();
			#endif
		}while(c != '\n' and c != 13);

		strcpy(name, str.c_str());

		valid_filename = true;
		Renderer::initialize_syntax_coloring(name);
		doc.render();
	}
	file = std::fstream(name, std::ios::out | std::ios::trunc);
	doc.save(file);
	file.close();
}

inline void Manager::update_scur()
{
	// offset from last tab margin
	// and total gaps incurred by \t s
	size_t offset = 0, total_gap = 0;
	int passes = 0;
	for(auto c: **cur_line){
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