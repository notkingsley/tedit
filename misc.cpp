#include "tedit.hpp"

int move_to(int y, int x)
{
	printf("%c[%d;%dH", 0x1B, y + 1, x + 1);
	return 0;
}

static std::map<std::string, std::string> colors{
	{"alignas", "\u001b[38;5;33"},
	{"alignof", "\u001b[38;5;33m"},
	{"asm", "\u001b[38;5;33m"},
	{"auto", "\u001b[38;5;33m"},
	{"bool", "\u001b[38;5;33m"},
	{"char", "\u001b[38;5;33m"},
	{"class", "\u001b[38;5;33m"},
	{"concept", "\u001b[38;5;33m"},
	{"const", "\u001b[38;5;33m"},
	{"constexpr", "\u001b[38;5;33m"},
	{"const_cast", "\u001b[38;5;33m"},
	{"decltype", "\u001b[38;5;33m"},
	{"double", "\u001b[38;5;33m"},
	{"dynamic_cast", "\u001b[38;5;33m"},
	{"enum", "\u001b[38;5;33m"},
	{"explicit", "\u001b[38;5;33m"},
	{"export", "\u001b[38;5;33m"},
	{"extern", "\u001b[38;5;33m"},
	{"false", "\u001b[38;5;33m"},
	{"float", "\u001b[38;5;33m"},
	{"friend", "\u001b[38;5;33m"},
	{"inline", "\u001b[38;5;33m"},
	{"int", "\u001b[38;5;33m"},
	{"long", "\u001b[38;5;33m"},
	{"mutable", "\u001b[38;5;33m"},
	{"namespace", "\u001b[38;5;33m"},
	{"noexcept", "\u001b[38;5;33m"},
	{"nullptr", "\u001b[38;5;33m"},
	{"private", "\u001b[38;5;33m"},
	{"protected", "\u001b[38;5;33m"},
	{"public", "\u001b[38;5;33m"},
	{"register", "\u001b[38;5;33m"},
	{"reinterpret_cast", "\u001b[38;5;33m"},
	{"requires", "\u001b[38;5;33m"},
	{"short", "\u001b[38;5;33m"},
	{"signed", "\u001b[38;5;33m"},
	{"sizeof", "\u001b[38;5;33m"},
	{"static", "\u001b[38;5;33m"},
	{"static_assert", "\u001b[38;5;33m"},
	{"static_cast", "\u001b[38;5;33m"},
	{"struct", "\u001b[38;5;33m"},
	{"synchronized", "\u001b[38;5;33m"},
	{"template", "\u001b[38;5;33m"},
	{"this", "\u001b[38;5;33m"},
	{"thread_local", "\u001b[38;5;33m"},
	{"typedef", "\u001b[38;5;33m"},
	{"typeid", "\u001b[38;5;33m"},
	{"typename", "\u001b[38;5;33m"},
	{"union", "\u001b[38;5;33m"},
	{"unsigned", "\u001b[38;5;33m"},
	{"virtual", "\u001b[38;5;33m"},
	{"void", "\u001b[38;5;33m"},
	{"volatile", "\u001b[38;5;33m"},
	{"wchar_t", "\u001b[38;5;33m"},


	{"break", "\u001b[38;5;128m"},
	{"case", "\u001b[38;5;128m"},
	{"catch", "\u001b[38;5;128m"},
	{"continue", "\u001b[38;5;128m"},
	{"default", "\u001b[38;5;128m"},
	{"delete", "\u001b[38;5;128m"},
	{"do", "\u001b[38;5;128m"},
	{"else", "\u001b[38;5;128m"},
	{"for", "\u001b[38;5;128m"},
	{"goto", "\u001b[38;5;128m"},
	{"if", "\u001b[38;5;128m"},
	{"new", "\u001b[38;5;128m"},
	{"operator", "\u001b[38;5;128m"},
	{"return", "\u001b[38;5;128m"},
	{"switch", "\u001b[38;5;128m"},
	{"throw", "\u001b[38;5;128m"},
	{"try", "\u001b[38;5;128m"},
	{"using", "\u001b[38;5;128m"},
	{"while", "\u001b[38;5;128m"},
};

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

Line::Line(std::string str, size_t pos) : std::string(str), position(pos) { }

Line::~Line() { }

void Line::render()
{
	move_to(position - 1, 0);
	std::cout << "\u001b[2K";
	render_alpha(*this, 0);
	std::cout.flush();
}