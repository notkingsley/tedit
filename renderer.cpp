#include "tedit.hpp"

// defined in render_color

extern std::map<std::string, std::string> colors;

extern std::map<std::string, std::string> colors_c;

extern std::map<std::string, std::string> colors_cpp;

extern std::map<std::string, std::string> colors_python;

extern std::map<std::string, std::string> colors_default;

void begin_render_cpp(Line* lp);

void begin_render_py(Line* lp);

void begin_render_default(Line* lp);

void (*begin_render)(Line* lp);


#ifndef _WIN32

static void get_win_size(int* res)
{
	winsize w;
	ioctl(0, TIOCGWINSZ, &w);
	res[0] = w.ws_row, res[1] = w.ws_col;
}

#else

static void get_win_size(int* res)
{
	CONSOLE_SCREEN_BUFFER_INFO info;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
	res[0] = info.srWindow.Bottom - info.srWindow.Top + 1;
	res[1] = info.srWindow.Right - info.srWindow.Left + 1;
}

#endif


Document* Renderer::doc = nullptr;
std::list<Line*>::iterator Renderer::start;
int Renderer::row_size = 0;
int Renderer::col_size = 0;

void Renderer::move_to_line(int y, int x)
{
	printf("%c[%ld;%dH", 0x1B, y + 1 - ((*start)->position - 1), x + 1);
}

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
	int i = 0;
	for(auto iter = start; iter != doc->lines.end() && i != row_size; ++iter, ++i)
		render_line(*iter);
}

void Renderer::render_line(Line* lp)
{
	// set the stage for rendering the line
	if(lp->position < (*start)->position)
		return;
	if(lp->position > (*start)->position + (row_size - 1))
		return;
	if(abs((int)(lp->position - (*doc->cur_line)->position)) >= row_size)
		return;
	move_to_line(lp->position - 1, 0);
	std::cout << "\u001b[2K";
	begin_render(lp);
	std::cout.flush();
}

void Renderer::move_to(int y, int x)
{
	if((*doc->cur_line)->position < (*start)->position){
		start = doc->cur_line;
		render_doc();
	}
	else if((*doc->cur_line)->position > (*start)->position + row_size - 1){
		std::advance(start, 
			(*doc->cur_line)->position - (row_size - 1) - (*start)->position);
		render_doc();
	}

	bool shifted = false;
	while(start != doc->lines.begin() and 
	doc->lines.back()->position - (*start)->position < row_size - 1){
		--start;
		shifted = true;
	}
	if(shifted)
		render_doc();

	move_to_line(y, x);
}

void Renderer::initialize_syntax_coloring(std::string filename)
{
	colors = colors_default;
	begin_render = begin_render_default;
	size_t last = filename.find_last_of('.');
	if(last == filename.npos)
		return;
	std::string ext = filename.substr(last + 1);

	if(ext == "cpp" or ext == "cp" or ext == "hpp"){
		colors = colors_cpp;
		begin_render = begin_render_cpp;
	}
	else if(ext == "c" or ext == "h"){
		colors = colors_c;
		begin_render = begin_render_cpp;
	}
	else if(ext == "py"){
		colors = colors_python;
		begin_render = begin_render_py;
	}
}

void Renderer::clean_last_line()
{
	if(doc->lines.back()->position - (*start)->position >= row_size)
		return;
	move_to_line(doc->lines.size(), 0);
	printf("%c[%dK", 0x1B, 2);
}

void Renderer::warn(std::list<Line*>::iterator cur_line)
{
	if(cur_line != start)
		return;
	if(start == doc->lines.begin())
		++start;
	else
		--start;
}

void Renderer::shift_page_up()
{
	if(doc->lines.back()->position - (*start)->position > row_size){
		++start;
		render_doc();
	}
}

void Renderer::shift_page_down()
{
	if(start != doc->lines.begin()){
		--start;
		render_doc();
	}
}

#ifndef _WIN32

void Renderer::update_terminal_size(int sig)
{
	if(sig != SIGWINCH)
		return;
	int res[2];
	get_win_size(res);
	row_size = res[0], col_size = res[1];
	move_to((*doc->cur_line)->position - 1, 0);
}

#endif		// _WIN32