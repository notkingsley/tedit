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

	Renderer::initialise_renderer(&doc);
	Renderer::initialize_syntax_coloring(name);

	init_screen();

	scury = cury = (*cur_line)->position - 1;
	curx = (*cur_line)->length();
	update_scur();
	move_to(scury, scurx);

}

Manager::~Manager()
{
	end_screen();
}

void Manager::listen()
{
	// clear the screen
	printf("%c[%dJ", 0x1B, 2);
	doc.render();
	paint_brackets();
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
			case CharType::SHOW_MANUAL:
			{
				key_show_manual();
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
		if(last_curx != -1 and last_curx < curx)
			throw 1;
		paint_brackets();
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

void Manager::paint_brackets()
{
	static Line* l1 = nullptr;
	static Line* l2 = nullptr;
	if(l1)
		Renderer::render_line(l1);
	if(l2)
		Renderer::render_line(l2);
	l1 = l2 = nullptr;

	auto fit = [](char c){
		return c == '(' or c == '[' or c == '{' or c == ')' or c == ']' or c == '}';
	};

	int start;
	if(curx < (**cur_line).length() and fit((**cur_line)[curx]))
		start = curx;
	else if(curx > 0 and fit((**cur_line)[curx - 1]))
		start = curx - 1;
	else
		return;

	char c = (**cur_line)[start];
	char comp;
	std::list<Line*>::iterator l = cur_line;
	switch(c){
		case '{':
			comp = '}';
			break;
		case '(':
			comp = ')';
			break;
		case '[':
			comp = ']';
			break;
		case '}':
			comp = '{';
			break;
		case ')':
			comp = '(';
			break;
		case ']':
			comp = '[';
			break;
	}

	int skips = 0;
	int height = 0;
	switch(c){
		case '{':
		case '(':
		case '[':
			paint_here(c, cury + height, start);
			l1 = *l;
			++start;
			while(l != doc.lines.end() and Renderer::is_on_screen(*l)){
				while(start < (**l).length() and (**l)[start] != comp and (**l)[start] != c)
					++start;

				if(start == (**l).length()){
					++l;
					++height;
					start = 0;
					continue;
				}
				if((**l)[start] == c){
					++skips;
					++start;
					continue;
				}
				if(skips){
					--skips;
					++start;
					continue;
				}
				paint_here(comp, cury + height, start);
				l2 = *l;
				return;
			}
			break;

		case '}':
		case ')':
		case ']':
			paint_here(c, cury + height, start);
			l1 = *l;
			--start;
			while(Renderer::is_on_screen(*l)){
				while(start >= 0 and (**l)[start] != comp and (**l)[start] != c)
					--start;

				if(start < 0){
					if(l == doc.lines.begin())
						return;
					--l;
					--height;
					start = (**l).length() - 1;
					continue;
				}
				if((**l)[start] == c){
					++skips;
					--start;
					continue;
				}
				if(skips){
					--skips;
					--start;
					continue;
				}
				paint_here(comp, cury + height, start);
				l2 = *l;
				return;
			}
			break;
	}
}
