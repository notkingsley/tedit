#include "tedit.hpp"

/* This contains definitions for Linux systems
 * Complementary definitions exist in windows.cpp
 */

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
		else if(c == 109)
		{	// alt-m
			return {CharType::SHOW_MANUAL, c};
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
	else if(c == 7)
	{	// ctrl+g
		return {CharType::SHOW_MANUAL, c};
	}
	return {CharType::INVALID, c};
}

std::string Manager::manual_message = {
R"(Hello! Welcome to the Tedit Manual page. Press CTRL+G at any time to show this manual.

Tedit is a command-line text editor that allows you to quickly write or edit short programs 
on the go without opening any heavy editor application.

It comes with all the basic functionalities for a text editor.
Below are the shortcuts/key mappings supported by your distribution
of Tedit not including the basic keys of the keyboard:

CTRL + G ->                  			show this manual page
ALT + M ->                   			show this manual page
ALT + S ->                   			save the contents to file
ALT + C ->                   			close the editor without saving
ALT + Q ->                   			save and close the editor
CTRL + B ->                  			save and close the editor
ALT + SHIFT + S ->           			save contents to another file name
CTRL + / ->                  			comment out this line( C/C++ )
ALT + ARROW KEY UP, DOWN ->  			move the current line up or down
CTRL + X ->                  			delete line
CTRL + ARROW KEY UP, DOWN -> 			move the page up or down
HOME / END KEY ->            			jump to the beginning/ end of the line
PAGE UP / DOWN ->            			jump to the beginning/ end of the document

Press enter key to continue.
)"
};

#endif