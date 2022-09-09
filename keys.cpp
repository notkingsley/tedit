#include "tedit.hpp"

void Manager::key_printable(char c)
{
	doc.insert(c, curx++);
	last_curx = -1;

	// brackets and quotes autocomplete
	if(c == '[' || c == '{')
		doc.insert(c + 2, curx);
	else if(c == '(')
		doc.insert(')', curx);
	else if((c == ']' || c == ')' || c == '}') && (**cur_line)[curx] == c)
		doc.del_char(curx);
	else if(c == '\'' || c == '"')
		if((**cur_line)[curx] == c)
			doc.del_char(curx);
		else
			doc.insert(c, curx);
}

void Manager::key_arrow(char c) 
{
	switch(c){
		case 'A':
			if(cur_line != doc.lines.begin()){
				--cur_line;
				--cury;

				if(last_curx > 0)
					curx = std::max(last_curx, (int)curx);
				
				last_curx = std::max(last_curx, (int)curx);
				curx = std::min(curx, (*cur_line)->length());
			}
			break;
		case 'B':
			if(*cur_line != doc.lines.back()){
				++cur_line;
				++cury;

				if(last_curx > 0)
					curx = std::max(last_curx, (int)curx);
				
				last_curx = std::max(last_curx, (int)curx);
				curx = std::min(curx, (*cur_line)->length());
			}
			break;
		case 'C':
			if(curx < (*cur_line)->length()){
				++curx;
			}else if(*cur_line != doc.lines.back()){
				++cur_line;
				++cury;
				curx = 0;
			}
			last_curx = -1;
			break;
		case 'D':
			if(curx > 0){
				--curx;
			}else if(cur_line != doc.lines.begin()){
				--cur_line;
				--cury;
				curx = (*cur_line)->length();
			}
			last_curx = -1;
			break;
	}
}

void Manager::key_backspace()
{
	if(curx > 0){								// same line

		// complementary delete
		if(curx < (*cur_line)->length())
			if(((**cur_line)[curx - 1] == '[' or (**cur_line)[curx - 1] == '{')
				and (**cur_line)[curx] == (**cur_line)[curx - 1] + 2)
				doc.del_char(curx);

			else if(((**cur_line)[curx - 1] == '\'' or (**cur_line)[curx - 1] == '"')
				and (**cur_line)[curx] == (**cur_line)[curx - 1])
				doc.del_char(curx);

			else if((**cur_line)[curx - 1] == '(' and (**cur_line)[curx] == ')')
				doc.del_char(curx);

		doc.backspace(curx--);
	}
	else if(cur_line != doc.lines.begin())
	{											// delete from prev line
		Renderer::warn(cur_line);
		std::string hold = **cur_line;
		doc.remove_line(cur_line);
		--cury;
		curx = (*cur_line)->length();
		**cur_line += hold;
		(*cur_line)->render();

		Renderer::clean_last_line();
	}
	last_curx = -1;
}

void Manager::key_delete()
{
	if(curx < (*cur_line)->length()){			// same line
		doc.del_char(curx);
	}
	else if(*cur_line != doc.lines.back())
	{											// delete from next line
		auto next = cur_line;
		++next;
		**cur_line += **next;
		doc.remove_line(next);

		Renderer::clean_last_line();
	}
	last_curx = -1;
}

void Manager::key_enter()
{
	// create a new line with the [curx:] slice of cur_line, adjust cury
	doc.add_new_line((*cur_line)->substr(curx), ++cury);
	// delete the [curx:] slice from cur_line
	(*cur_line)->erase((*cur_line)->begin() + curx, (*cur_line)->end());

	// evaluate indent level
	size_t indent = (*cur_line)->find_first_not_of('\t');
	if(indent == (*cur_line)->npos)
		indent = 0;
	if(std::count((*cur_line)->begin(), (*cur_line)->end(), '\t') == 
		(*cur_line)->length()) indent = (*cur_line)->length();
	if((*cur_line)->back() == '{')
		++indent;

	Line* last = *cur_line;
	// adjust curx and cur_line
	++cur_line;
	curx = indent;

	// {} case
	if(**cur_line == "}" and last->back() == '{'){
		(*cur_line)->erase();
		std::string str;
		str.insert(0, indent - 1, '\t');
		str += '}';
		doc.add_new_line(str, cury + 1);
	}

	// insert indent and render
	(*cur_line)->insert(0, indent, '\t');
	doc.render();
	last_curx = -1;
}

void Manager::key_ctrl_arrow(char c)
{
	switch(c){
		case 'A':
		{
			Renderer::shift_page_down();
			if(not Renderer::is_on_screen(*cur_line))
				key_arrow(c);
			break;
		}
		case 'B':
		{
			Renderer::shift_page_up();
			if(not Renderer::is_on_screen(*cur_line))
				key_arrow(c);
			break;
		}
		case 'C':
		{
			// end of line?
			if(curx == (*cur_line)->length()){
				return key_arrow(c);
			}
			// skip all whitespace
			while(curx < (*cur_line)->length() 
				&& isblank((**cur_line)[curx])) ++curx;

			if(curx < (*cur_line)->length()){
				if(isalnum((**cur_line)[curx]))
					// letters
					while(curx < (*cur_line)->length() 
						&& isalnum((**cur_line)[curx])) ++curx;
				else if(ispunct((**cur_line)[curx]))
					// punctuations
					while(curx < (*cur_line)->length() 
						&& ispunct((**cur_line)[curx])) ++curx;
			}

			last_curx = -1;
			break;
		}
		case 'D':
		{
			// beginning of line?
			if(curx == 0){
				return key_arrow(c);
			}
			// skip all whitespace
			while(curx > 0 && isblank((**cur_line)[curx - 1])) --curx;

			if(curx > 0){
				if(isalnum((**cur_line)[curx - 1]))
					// letters
					while(curx > 0 && isalnum((**cur_line)[curx - 1])) 
						--curx;
				else if(ispunct((**cur_line)[curx - 1]))
					// punctuations
					while(curx > 0 && ispunct((**cur_line)[curx - 1]))
						--curx;
			}

			last_curx = -1;
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
	while(curx > 0 && isblank((**cur_line)[curx - 1])) --curx;

	if(curx > 0){
		if(isalnum((**cur_line)[curx - 1]))
			while(curx > 0 && isalnum((**cur_line)[curx - 1])) --curx;
		else if(ispunct((**cur_line)[curx - 1]))
			while(curx > 0 && ispunct((**cur_line)[curx - 1])) --curx;
	}

	(*cur_line)->erase(curx, start - curx);
	doc.render();
	last_curx = -1;
}

void Manager::key_ctrl_delete()
{
	// end of line?
	if(curx == (*cur_line)->length()){
		return key_delete();
	}

	int end = curx;
	// skip all whitespace
	while(end < (*cur_line)->length() 
		&& isblank((**cur_line)[end])) ++end;

	if(end < (*cur_line)->length()){
		if(isalnum((**cur_line)[end]))
			// letters
			while(end < (*cur_line)->length() 
				&& isalnum((**cur_line)[end])) ++end;
		else if(ispunct((**cur_line)[curx]))
			// punctuations
			while(end < (*cur_line)->length() 
				&& ispunct((**cur_line)[end])) ++end;
	}

	(*cur_line)->erase(curx, end - curx);
	doc.render();
	last_curx = -1;
}

void Manager::key_ctrl_x()
{
	(*cur_line)->erase();
	paint_brackets();	// legally invalidate internal Line pointers
	size_t hold = curx;
	curx = 0;
	if((*cur_line) != doc.lines.back())
		key_delete();
	else
		key_backspace();
	curx = std::min(hold, (*cur_line)->length());
	last_curx = -1;
}

void Manager::key_home()
{
	curx = 0;
	last_curx = -1;
}

void Manager::key_end()
{
	curx = (*cur_line)->length();
	last_curx = -1;
}

void Manager::key_page_down()
{
	cur_line = doc.lines.end();
	--cur_line;
	curx = (*cur_line)->length();
	cury = doc.lines.size() - 1;
	last_curx = -1;
}

void Manager::key_page_up()
{
	cur_line = doc.lines.begin();
	cury = curx = 0;
	last_curx = -1;
}

void Manager::key_alt_arrow(char c)
{
	switch(c){
		case 'A':
		{
			if(cur_line == doc.lines.begin())
				return;
			auto prev = cur_line--;
			--cury;
			std::string hold = **cur_line;
			(*cur_line)->assign(**prev);
			(*prev)->assign(hold);
			doc.render();
			break;
		}
		case 'B':
		{
			if(*cur_line == doc.lines.back())
				return;
			auto next = cur_line++;
			++cury;
			std::string hold = **cur_line;
			(*cur_line)->assign(**next);
			(*next)->assign(hold);
			doc.render();
			break;
		}
	}
	last_curx = -1;
}

void Manager::key_comment_line()
{
	std::string ns{name};
	size_t last = ns.find_last_of('.');
	if(last == ns.npos)
		return;
	ns.erase(0, last + 1);

	size_t start = 0;
	// skip blanks
	while(start < (*cur_line)->length() and isblank((**cur_line)[start]))
		++start;
	if(start == (*cur_line)->length())
		return;
		
	if(ns == "py"){
		if((**cur_line)[start] == '#'){
			(*cur_line)->erase(start, 1);
			--curx;
			if((**cur_line)[start] == ' ')
				(*cur_line)->erase(start, 1), --curx;
		}
		else{
			(*cur_line)->insert(start, "# ");
			curx += 2;
		}
		(*cur_line)->render();
		last_curx = -1;
	}
	else if(ns == "cpp" or ns == "cp" or ns == "hpp" or ns == "c" or ns == "h")
	{
		if((**cur_line)[start] == '/' and (**cur_line).length() > start + 1 
			and (**cur_line)[start + 1] == '/'){
			(*cur_line)->erase(start, 2);
			curx -= 2;
			if((**cur_line)[start] == ' ')
				(*cur_line)->erase(start, 1), --curx;
		}
		else{
			(*cur_line)->insert(start, "// ");
			curx += 3;
		}
		(*cur_line)->render();
		last_curx = -1;
	}
}

void Manager::key_show_manual()
{
	// clear the screen
	printf("%c[%dJ", 0x1B, 2);
	printf("%c[%d;%dH", 0x1B, 1, 1);

	std::cout << manual_message;
	while(getchar() != '\n');

	// clear screen again
	printf("%c[%dJ", 0x1B, 2);
	doc.render();
}