#include <vector>
#include <list>
#include <iostream>
#include <cmath>
#include <fstream>
#include <cstring>
#include <algorithm>
#include <termios.h>

#ifndef TEDIT_HPP
#define TEDIT_HPP 1

// represent the kind of action 
// a sequence of input characters
// represent
enum class CharType{
	PRINTABLE,
	ARROW_KEY,
	ENTER_KEY,
	BACKSPACE,
	DELETE_KEY,
	TAB_KEY,
	CTRL_ARROW,
	CTRL_BACKSPACE,
	CTRL_DELETE,
	SAVE,
	EXIT,
	DISCARD_SESSION,
	INVALID
};

class Document;
class Manager;

class Line{
	// where this line is
	size_t position;

	// string content of line
	std::string data;

	friend Document;
	friend Manager;
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

	// old state of terminal
	termios oldt;

	// reference to iterator at current
	// line of document
	std::__cxx11::list<Line *>::iterator& cur_line;

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

	// inserts a tab at current position
	void key_tab();

	// update current position according to
	// ctrl_arrow key
	void key_ctrl_arrow(char c);

	// remove the corresponding characters 
	// at current position for ctrl_backspace
	void key_ctrl_backspace();

	// remove the corresponding character from
	// current position for ctrl_delete
	void key_ctrl_delete();

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

// move to position (x, y) on terminal screen
int move_to(int y, int x);

#endif