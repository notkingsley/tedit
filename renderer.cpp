#include "tedit.hpp"

static const char* BLUE = "\u001b[38;5;33m";
static const char* PURPLE = "\u001b[38;5;128m";

static std::map<std::string, std::string> colors_cpp{
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


	{"break", PURPLE},
	{"case", PURPLE},
	{"catch", PURPLE},
	{"continue", PURPLE},
	{"default", PURPLE},
	{"delete", PURPLE},
	{"do", PURPLE},
	{"else", PURPLE},
	{"for", PURPLE},
	{"goto", PURPLE},
	{"if", PURPLE},
	{"new", PURPLE},
	{"operator", PURPLE},
	{"return", PURPLE},
	{"switch", PURPLE},
	{"throw", PURPLE},
	{"try", PURPLE},
	{"using", PURPLE},
	{"while", PURPLE},
};


static std::map<std::string, std::string> colors_c{
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


	{"break", PURPLE},
	{"case", PURPLE},
	{"continue", PURPLE},
	{"default", PURPLE},
	{"do", PURPLE},
	{"else", PURPLE},
	{"for", PURPLE},
	{"goto", PURPLE},
	{"if", PURPLE},
	{"return", PURPLE},
	{"switch", PURPLE},
	{"while", PURPLE},
};


static std::map<std::string, std::string> colors_python{
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


	{"as", PURPLE},
	{"assert", PURPLE},
	{"async", PURPLE},
	{"await", PURPLE},
	{"break", PURPLE},
	{"else", PURPLE},
	{"elif", PURPLE},
	{"else", PURPLE},
	{"except", PURPLE},
	{"finally", PURPLE},
	{"for", PURPLE},
	{"from", PURPLE},
	{"global", PURPLE},
	{"if", PURPLE},
	{"import", PURPLE},
	{"in", PURPLE},
	{"pass", PURPLE},
	{"raise", PURPLE},
	{"return", PURPLE},
	{"try", PURPLE},
	{"while", PURPLE},
	{"with", PURPLE},
	{"yield", PURPLE},
};

static std::map<std::string, std::string> colors_default { };

static std::map<std::string, std::string> colors; 


// print a word
static inline void render_word(std::string word)
{
	std::cout << colors[word] << word << "\u001b[0m";
}

static void render_others(const std::string& word, size_t start);

static void render_alpha(const std::string& word, size_t start)
{
	size_t end = start;
	while(end < word.length() and isalpha(word[end]))
		++end;
	render_word(word.substr(start, end - start));

	if(end == word.length())
		return;
	render_others(word, end);
}

static void render_others(const std::string& word, size_t start)
{
	size_t end = start;
	while(end < word.length() and not isalpha(word[end]))
		++end;
	std::cout << word.substr(start, end - start);

	if(end == word.length())
		return;
	render_alpha(word, end);
}

static void get_win_size(int* res)
{
	winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	res[0] = w.ws_row, res[1] = w.ws_col;
}


Document* Renderer::doc = nullptr;
std::list<Line*>::iterator Renderer::start;
int Renderer::row_size = 0;
int Renderer::col_size = 0;

void Renderer::initialise_renderer(Document* dp)
{
	if(not dp)
		return;
	doc = dp;
	start = doc->lines.begin();
	int res[2];
	get_win_size(res);
	row_size = res[0], col_size = res[1];
}

void Renderer::render_doc()
{
	for(Line* l: doc->lines)
		l->render();
}

void Renderer::render_line(Line* lp)
{
	move_to(lp->position - 1, 0);
	std::cout << "\u001b[2K";
	render_alpha(*lp, 0);
	std::cout.flush();
}

void Renderer::move_to(int y, int x)
{
	printf("%c[%d;%dH", 0x1B, y + 1, x + 1);
}

void Renderer::initialize_syntax_coloring(std::string filename)
{
	colors = colors_default;
	size_t last = filename.find_last_of('.');
	if(last == filename.npos)
		return;
	std::string ext = filename.substr(last + 1);

	if(ext == "cpp" or ext == "cp" or ext == "hpp")
		colors = colors_cpp;
	else if(ext == "c" or ext == "h")
		colors = colors_c;
	else if(ext == "py")
		colors = colors_python;
}
