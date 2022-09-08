#include "tedit.hpp"

static const char* BLUE = "\u001b[38;5;21m";
static const char* PURPLE = "\u001b[38;5;93m";
static const char* GREY = "\u001b[38;5;240m";
static const char* BROWN = "\u001b[38;5;136m";
static const char* RED = "\u001b[38;5;160m";
static const char* RED_BG = "\u001b[48;5;160m";
static const char* YELLOW = "\u001b[38;5;11m";
static const char* PINK = "\u001b[38;5;200m";
static const char* IDENTIFIER_COLOR = "\u001b[38;5;44m";
static const char* FUNCTION_COLOR = "\u001b[38;5;110m";
static const char* RESET = "\u001b[0m";

std::map<std::string, std::string> colors_cpp{
	{"alignas", BLUE},
	{"alignof", BLUE},
	{"asm", BLUE},
	{"auto", BLUE},
	{"bool", BLUE},
	{"char", BLUE},
	{"class", BLUE},
	{"concept", BLUE},
	{"const", BLUE},
	{"constexpr", BLUE},
	{"const_cast", BLUE},
	{"decltype", BLUE},
	{"double", BLUE},
	{"dynamic_cast", BLUE},
	{"enum", BLUE},
	{"explicit", BLUE},
	{"export", BLUE},
	{"extern", BLUE},
	{"false", BLUE},
	{"float", BLUE},
	{"friend", BLUE},
	{"inline", BLUE},
	{"int", BLUE},
	{"long", BLUE},
	{"mutable", BLUE},
	{"namespace", BLUE},
	{"noexcept", BLUE},
	{"nullptr", BLUE},
	{"private", BLUE},
	{"protected", BLUE},
	{"public", BLUE},
	{"register", BLUE},
	{"reinterpret_cast", BLUE},
	{"requires", BLUE},
	{"short", BLUE},
	{"signed", BLUE},
	{"sizeof", BLUE},
	{"static", BLUE},
	{"static_assert", BLUE},
	{"static_cast", BLUE},
	{"struct", BLUE},
	{"synchronized", BLUE},
	{"template", BLUE},
	{"this", BLUE},
	{"thread_local", BLUE},
	{"typedef", BLUE},
	{"typeid", BLUE},
	{"typename", BLUE},
	{"union", BLUE},
	{"unsigned", BLUE},
	{"virtual", BLUE},
	{"void", BLUE},
	{"volatile", BLUE},
	{"wchar_t", BLUE},


	{"break", PINK},
	{"case", PINK},
	{"catch", PINK},
	{"continue", PINK},
	{"default", PINK},
	{"delete", PINK},
	{"do", PINK},
	{"else", PINK},
	{"for", PINK},
	{"goto", PINK},
	{"if", PINK},
	{"new", PINK},
	{"operator", PINK},
	{"return", PINK},
	{"switch", PINK},
	{"throw", PINK},
	{"try", PINK},
	{"using", PINK},
	{"while", PINK},
};


std::map<std::string, std::string> colors_c{
	{"auto", BLUE},
	{"char", BLUE},
	{"const", BLUE},
	{"double", BLUE},
	{"enum", BLUE},
	{"extern", BLUE},
	{"float", BLUE},
	{"inline", BLUE},
	{"int", BLUE},
	{"long", BLUE},
	{"register", BLUE},
	{"restrict", BLUE},
	{"short", BLUE},
	{"signed", BLUE},
	{"sizeof", BLUE},
	{"static", BLUE},
	{"struct", BLUE},
	{"typedef", BLUE},
	{"union", BLUE},
	{"unsigned", BLUE},
	{"void", BLUE},
	{"volatile", BLUE},


	{"break", PINK},
	{"case", PINK},
	{"continue", PINK},
	{"default", PINK},
	{"do", PINK},
	{"else", PINK},
	{"for", PINK},
	{"goto", PINK},
	{"if", PINK},
	{"return", PINK},
	{"switch", PINK},
	{"while", PINK},
};


std::map<std::string, std::string> colors_python{
	{"False", BLUE},
	{"None", BLUE},
	{"True", BLUE},
	{"and", BLUE},
	{"class", BLUE},
	{"def", BLUE},
	{"is", BLUE},
	{"lambda", BLUE},
	{"nonlocal", BLUE},
	{"not", BLUE},
	{"or", BLUE},


	{"as", PINK},
	{"assert", PINK},
	{"async", PINK},
	{"await", PINK},
	{"break", PINK},
	{"else", PINK},
	{"elif", PINK},
	{"else", PINK},
	{"except", PINK},
	{"finally", PINK},
	{"for", PINK},
	{"from", PINK},
	{"global", PINK},
	{"if", PINK},
	{"import", PINK},
	{"in", PINK},
	{"pass", PINK},
	{"raise", PINK},
	{"return", PINK},
	{"try", PINK},
	{"while", PINK},
	{"with", PINK},
	{"yield", PINK},
};

std::map<std::string, std::string> colors_default { };

std::map<std::string, std::string> colors;


static inline bool is_opening_bracket(char c)
{
	return c == '[' or c == '{' or c == '(';
}

static inline bool is_closing_bracket(char c)
{
	return c == ']' or c == '}' or c == ')';
}

static inline bool is_bracket(char c)
{
	return is_opening_bracket(c) or is_closing_bracket(c);
}

static inline bool is_quote(char c)
{
	return c == '\'' or c == '"';
}

static inline bool is_identifier(char  c)
{
	return isalnum(c) or c == '_';
}

static inline bool is_processable_cpp(char c)
{
	return is_identifier(c) or is_quote(c) or is_bracket(c) or c == '/';
}

static inline bool is_processable_py(char c)
{
	return is_identifier(c) or is_quote(c) or is_bracket(c) or c == '#';
}

void begin_render_default(Line* lp)
{
	std::cout << *lp;
}

void begin_render_cpp(Line* lp)
{
	if(lp->length() >= 8 and lp->substr(0, 8) == "#include"){
		std::cout << PINK << lp->substr(0, 8) 
			<< BROWN << lp->substr(8) << RESET;
		return;
	}
	else if(lp->length() >= 7 and lp->substr(0, 7) == "#define"){
		std::cout << PINK << lp->substr(0, 7)
			<< BLUE << lp->substr(7) << RESET;
		return;
	}
	else if(lp->length() >= 6 and lp->substr(0, 6) == "#ifdef"){
		std::cout << PINK << lp->substr(0, 6)
			<< BLUE << lp->substr(6) << RESET;
		return;
	}
	else if(lp->length() >= 7 and lp->substr(0, 6) == "#ifndef"){
		std::cout << PINK << lp->substr(0, 7)
			<< BLUE << lp->substr(6) << RESET;
		return;
	}
	else if(lp->length() >= 6 and lp->substr(0, 6) == "#undef"){
		std::cout << PINK << lp->substr(0, 6)
			<< BLUE << lp->substr(6) << RESET;
		return;
	}
	else if(lp->length() >= 3 and lp->substr(0, 3) == "#if"){
		std::cout << PINK << lp->substr(0, 3)
			<< BLUE << lp->substr(3) << RESET;
		return;
	}
	else if(lp->length() >= 5 and lp->substr(0, 5) == "#elif"){
		std::cout << PINK << lp->substr(0, 5) << RESET;
		Line temp = *lp;
		temp.erase(0, 5);
		begin_render_cpp(&temp);
		return;
	}
	else if(lp->length() >= 5 and lp->substr(0, 5) == "#else"){
		std::cout << PINK << lp->substr(0, 5) << RESET;
		Line temp = *lp;
		temp.erase(0, 5);
		begin_render_cpp(&temp);
		return;
	}
	else if(lp->length() >= 6 and lp->substr(0, 6) == "#endif"){
		std::cout << PINK << lp->substr(0, 6) << RESET;
		Line temp = *lp;
		temp.erase(0, 6);
		begin_render_cpp(&temp);
		return;
	}
	else if(lp->length() >= 6 and lp->substr(0, 6) == "#error"){
		std::cout << PINK << lp->substr(0, 6)
			<< BROWN << lp->substr(6) << RESET;
		return;
	}
	else if(lp->length() >= 8 and lp->substr(0, 6) == "#warning"){
		std::cout << PINK << lp->substr(0, 8)
			<< BROWN << lp->substr(8) << RESET;
		return;
	}
	else if(lp->length() >= 7 and lp->substr(0, 7) == "#pragma"){
		std::cout << PINK << lp->substr(0, 7)
			<< BROWN << lp->substr(7) << RESET;
		return;
	}

	size_t start = 0;
	while(start < lp->length()){
		size_t end = start;
		while(end < lp->length() and not is_processable_cpp((*lp)[end]))
			++ end;
		std::cout << lp->substr(start, end - start);
		start = end;

		if(end == lp->length())
			return;
		
		if((*lp)[end] == '/'){
			if(lp->length() > end + 1 and ((*lp)[end + 1] == '/' or (*lp)[end + 1] == '*')){
				std::cout << GREY << lp->substr(end) << RESET;
				return;
			}
			std::cout << (*lp)[end];
			start = end + 1;
			continue;
		}

		if(is_bracket((*lp)[end])){
			std::cout << PURPLE << (*lp)[end] << RESET;
			start = end + 1;
			continue;
		}

		if(is_quote((*lp)[end])){
			start = end++;
			while(end < lp->length() and (*lp)[end] != (*lp)[start])
				++end;
			if(end == lp->length()){
				std::cout << BROWN << lp->substr(start) << RESET;
				return;
			}
			std::cout << BROWN << lp->substr(start, end - start + 1) << RESET;
			start = end + 1;
			continue;
		}

		if(is_identifier((*lp)[end])){
			while(end < lp->length() and is_identifier((*lp)[end]))
				++end;
			std::string color = colors[lp->substr(start, end - start)];
			if(color.empty())
				if(end < lp->length() and (*lp)[end] == '(')
					std::cout << FUNCTION_COLOR;
				else
					std::cout << IDENTIFIER_COLOR;
			else
				std::cout << color;
			std::cout << lp->substr(start, end - start) << RESET;
			
			if(end == lp->length())
				return;
			start = end;
			continue;
		}
	}
}

void begin_render_py(Line* lp)
{
	size_t start = 0;
	while(start < lp->length()){
		size_t end = start;
		while(end < lp->length() and not is_processable_py((*lp)[end]))
			++ end;
		std::cout << lp->substr(start, end - start);
		start = end;

		if(end == lp->length())
			return;
		
		if((*lp)[end] == '#'){
			std::cout << GREY << lp->substr(end) << RESET;
			return;
		}

		if(is_bracket((*lp)[end])){
			std::cout << PURPLE << (*lp)[end] << RESET;
			start = end + 1;
			continue;
		}

		if(is_quote((*lp)[end])){
			if(end <= lp->length() - 3 and lp->substr(end, 3) == "\"\"\""){
				std::cout << GREY << lp->substr(end) << RESET;
				return;
			}
			start = end++;
			while(end < lp->length() and (*lp)[end] != (*lp)[start])
				++end;
			if(end == lp->length()){
				std::cout << BROWN << lp->substr(start) << RESET;
				return;
			}
			std::cout << BROWN << lp->substr(start, end - start + 1) << RESET;
			start = end + 1;
			continue;
		}

		if(is_identifier((*lp)[end])){
			while(end < lp->length() and is_identifier((*lp)[end]))
				++end;
			std::string color = colors[lp->substr(start, end - start)];
			if(color.empty())
				if(end < lp->length() and (*lp)[end] == '(')
					std::cout << FUNCTION_COLOR;
				else
					std::cout << IDENTIFIER_COLOR;
			else
				std::cout << color;
			std::cout << lp->substr(start, end - start) << RESET;
			
			if(end == lp->length())
				return;
			start = end;
			continue;
		}
	}
}

void Manager::paint_here(char c, int y, int x)
{
	size_t tmpy = cury, tmpx = curx;
	cury = y, curx = x;
	update_scur();
	Renderer::move_to(scury, scurx);
	std::cout << RED << c << RESET;
	cury = tmpy, curx = tmpx;
}