#ifndef TEDIT_HPP
#define TEDIT_HPP 1

#include <vector>
#include <list>
#include <iostream>
#include <cmath>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <map>
#include <signal.h>

#ifndef _WIN32

#include <termios.h>
#include <sys/ioctl.h>

#else

#include <windows.h>
#include <conio.h>

#endif

// represent the kind of action 
// a sequence of input characters
// represent
enum class CharType{
	PRINTABLE,
	ARROW_KEY,
	ENTER_KEY,
	BACKSPACE,
	DELETE_KEY,
	CTRL_ARROW,
	CTRL_BACKSPACE,
	CTRL_DELETE,
	CTRL_X,
	HOME_KEY,
	END_KEY,
	PAGE_UP,
	PAGE_DOWN,
	ALT_ARROW,
	COMMENT_LINE,
	SAVE,
	EXIT,
	DISCARD_SESSION,
	INVALID
};

class Document;
class Manager;
class Renderer;

class Line : public std::string{
	// where this line is
	size_t position;

	friend Document;
	friend Manager;
	friend Renderer;
public:
	// create a new line
	Line(std::string str, size_t pos);

	// delete a line 
	~Line();

	// render this line again
	void render();

};

class Document{
	// lines in document
	std::list<Line*> lines;

	// iterator pointing to current line
	std::__cxx11::list<Line *>::iterator cur_line;

	friend Manager;
	friend Renderer;
public:
	// create a document object for text editing
	Document();

	// open this document with the contents of file
	Document(std::fstream& fs);

	// close and delete this document
	~Document();

	// display the document on the terminal
	void render();

	// insert a new line with given string
	// before given position
	// a negative number puts it at the end
	void add_new_line(std::string str, int pos);

	// discard present contents and read 
	// contents of file
	void read_file(std::fstream& fs);

	// save the present contents of document 
	// to output filestream
	void save(std::fstream& fs);

	// insert a single printable char to
	// position idx of current line
	void insert(char c, size_t idx);

	// remove the character before idx
	// on current line
	void backspace(size_t idx);

	// remove the character at idx 
	// on cuurent line
	void del_char(size_t idx);

	// remove a line. cur_line is removed 
	// if pos is not given or is null
	void remove_line(std::__cxx11::list<Line *>::iterator pos);
};

class Manager{
	// current file, not left open
	std::fstream file;

	// name of file
	const char* name;

	// document representing current open file
	Document doc;

	// cursor position in string data
	size_t curx, cury;

	// cursor position on screen
	// scury should be redundant, and
	// scurx is curx if no tabs
	size_t scurx, scury;

#	ifndef _WIN32
	// old state of terminal
	termios oldt;

#	else
	// old state of terminal
	HANDLE stdOutHandle;
	HANDLE stdInHandle;
	DWORD dwOriginalOutMode;
	DWORD dwOriginalInMode;
#	endif

	// reference to iterator at current
	// line of document
	std::__cxx11::list<Line *>::iterator& cur_line;

	// save old terminal state, disable buffering and echo
	void init_screen();

	// restore old terminal state
	void end_screen();

	// save contents of document to file
	void save();

	// insert a printable character to the 
	// current position
	void key_printable(char c);

	// update current position according to
	// arrow key
	void key_arrow(char c);

	// removes the corresponding character
	// at current position for a backspace
	void key_backspace();

	// removes the corresponding character 
	// at current position for a delete key
	void key_delete();

	// moves all character after current 
	// position to a new line
	void key_enter();

	// update current position according to
	// ctrl_arrow key
	void key_ctrl_arrow(char c);

	// remove the corresponding characters 
	// at current position for ctrl_backspace
	void key_ctrl_backspace();

	// remove the corresponding character from
	// current position for ctrl_delete
	void key_ctrl_delete();

	// completely delete the current line
	void key_ctrl_x();

	// move to the beginning of current line
	void key_home();

	// move to the end of the current line
	void key_end();

	// move to the beginning of the first
	void key_page_up();

	// move to the end of the last line
	void key_page_down();

	// alt-arrow keys, move a line up or down
	void key_alt_arrow(char c);

	// comment or uncomment out cur_line
	void key_comment_line();

	// update scurx and scury according to cur_line
	// and curx and cury
	inline void update_scur();

	// get the next character/ action from stdin
	std::pair<CharType, char> get_next();

	const int TAB_SIZE = 8;
public:
	// create a new manager object
	Manager();

	// create a manager object with a given file
	Manager(const char* filename);

	//destroy this manager object;
	~Manager();

	// listens forever on stdin, updating and 
	// rendering internal documents accordingly
	void listen();
};

class Renderer{
	// internal function to actually goto a line
	static void move_to_line(int y, int x);

public:
	// iterator to the first line allowed to be on screen
	static std::list<Line*>::iterator start;

	// height of the terminal
	static int row_size;

	//width of the terminal
	static int col_size;

	// pointer to active document
	static Document* doc;

	// initialize the renderer with docunent
	static void initialise_renderer(Document* doc);

	// print all lines of document
	// considering size of screen
	static void render_doc();

	// re-render a line according to its
	// position and screen variables
	static void render_line(Line* lp);

	// move to position (x, y) on terminal screen
	static void move_to(int y, int x);

	// initialize syntax coloring with a given 
	// filename (extension is deduced)
	static void initialize_syntax_coloring(std::string filename);

	// clear the last line after possible doc shrinking
	static void clean_last_line();

	// warn the renderer that this iterator is about
	// to be invalidated
	static void warn(std::list<Line*>::iterator cur_line);

	// shift the whole page up by one line
	static void shift_page_up();

	// shift the whole page down by one line
	static void shift_page_down();

#	ifndef _WIN32
	// receive window resize signal and update variables
	static void update_terminal_size(int sig);

#	endif	// _WIN32
};

// move to position (x, y) on terminal screen
void move_to(int y, int x);

#endif