#include <vector>
#include <list>
#include <iostream>
#include <cmath>
#include <fstream>
#include <cstring>
#include <termios.h>

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
	EXIT,
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

	// inseert a new line with given string
	// before given position
	// a negative number puts it at the end
	void add_new_line(std::string str, int pos);

	// discard present contents and read 
	// contents of file
	void read_file(std::fstream& fs);
};

class Manager{
	// current file
	std::fstream file;

	// name of file
	const char* name;

	// document representing current open file
	Document doc;

	// cursor position
	size_t curx, cury;

	// old state of terminal
	termios oldt;

	// reference to iterator at current
	// line of document
	std::__cxx11::list<Line *>::iterator& cur_line;

	// save contents of document to file
	void save();
public:
	// create a new manager object
	Manager();

	// create a manager object with a given file
	Manager(const char* filename);

	//destroy this manager object;
	~Manager();

	// get the next character/ action from stdin
	std::pair<CharType, char> get_next();

	// listens forever on stdin, updating and 
	// rendering internal documents accordingly
	void listen();
};

// move to position (x, y) on terminal screen
int move_to(int y, int x);