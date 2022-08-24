#include "tedit.hpp"

/* File containing definitions specific to 
 * Windows system's screens and keyboard
 */

#ifdef _WIN32

//terminal process init for windows screen
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

void Manager::init_screen()
{
	// Set output mode to handle virtual terminal sequences
	stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	if (stdOutHandle == INVALID_HANDLE_VALUE)
	{
		exit(GetLastError());
	}
	stdInHandle = GetStdHandle(STD_INPUT_HANDLE);
	if (stdInHandle == INVALID_HANDLE_VALUE)
	{
		exit(GetLastError());
	}

	dwOriginalOutMode = 0;
	dwOriginalInMode = 0;
	if (!GetConsoleMode(stdOutHandle, &dwOriginalOutMode))
	{
		exit(GetLastError());
	}
	if (!GetConsoleMode(stdInHandle, &dwOriginalInMode))
	{
		exit(GetLastError());
	}

	DWORD dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN;
	DWORD dwRequestedInModes = ENABLE_VIRTUAL_TERMINAL_INPUT;

	DWORD dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
	if (!SetConsoleMode(stdOutHandle, dwOutMode))
	{
		// we failed to set both modes, try to step down mode gracefully.
		dwRequestedOutModes = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
		dwOutMode = dwOriginalOutMode | dwRequestedOutModes;
		if (!SetConsoleMode(stdOutHandle, dwOutMode))
		{
			// Failed to set any VT mode, can't do anything here.
			exit(GetLastError());
		}
	}

	DWORD dwInMode = dwOriginalInMode | ENABLE_VIRTUAL_TERMINAL_INPUT;
	dwInMode &= ~ENABLE_ECHO_INPUT;
	if (!SetConsoleMode(stdInHandle, dwInMode))
	{
		// Failed to set VT input mode, can't do anything here.
		exit(GetLastError());
	}
}

void Manager::end_screen()
{
	//reset console mode
	if(!SetConsoleMode(stdOutHandle, dwOriginalOutMode))
		exit(GetLastError());
	if(!SetConsoleMode(stdInHandle, dwOriginalInMode))
		exit(GetLastError());
}

std::pair<CharType, char> Manager::get_next()
{
	char c = getch();
	// match exactly
	if (c >= 32 && c <= 126){
		return {CharType::PRINTABLE, c};
	}
	switch(c){
		case -32:
			c = getch();
			switch(c){
				case 83:
					return {CharType::DELETE_KEY, c};
				case -109:
					return {CharType::CTRL_DELETE, c};
				case 72:
					return {CharType::ARROW_KEY, 'A'};
				case 80:
					return {CharType::ARROW_KEY, 'B'};
				case 77:
					return {CharType::ARROW_KEY, 'C'};
				case 75:
					return {CharType::ARROW_KEY, 'D'};
				case -115:
					return {CharType::CTRL_ARROW, 'A'};
				case -111:
					return {CharType::CTRL_ARROW, 'B'};
				case 116:
					return {CharType::CTRL_ARROW, 'C'};
				case 115:
					return {CharType::CTRL_ARROW, 'D'};
				case 79:
					return {CharType::END_KEY, c};
				case 71:
					return {CharType::HOME_KEY, c};
				case 73:
					return {CharType::PAGE_UP, c};
				case 81:
					return {CharType::PAGE_DOWN, c};
			}
		case 0:
			c = getch();
			switch(c){
				case -104:
					return {CharType::ALT_ARROW, 'A'};
				case -96:
					return {CharType::ALT_ARROW, 'B'};
				case -99:
					return {CharType::ALT_ARROW, 'C'};
				case -101:
					return {CharType::ALT_ARROW, 'D'};
			}
		case 2:		// ctrl+b
			return {CharType::EXIT, c};
		case 19:	// ctrl+s
			return {CharType::SAVE, c};
		case 17:	// ctrl+q
			return {CharType::DISCARD_SESSION, c};
		case 8:
			return {CharType::BACKSPACE, c};
		case 10:	// fall through
		case 13:	// enter or new_line
			return {CharType::ENTER_KEY, c};
		case 9:		// tab key
			return {CharType::PRINTABLE, c};
		case 23:
			return {CharType::CTRL_BACKSPACE, c};
		case 24:	// ctrl+x
			return {CharType::CTRL_X, c};
		case 31:	// ctrl+/
			return {CharType::COMMENT_LINE, c};
		case 20:	// ctrl+t
			return {CharType::SAVE_AS, c};
		case 7:		// ctrl+g
			return {CharType::SHOW_MANUAL, c};
		default:
			return {CharType::INVALID, c};
	}
}

std::string Manager::manual_message = {
R"(Hello! Welcome to the Tedit Manual page. Press CTRL+G at any time to show this manual.

Tedit is a command-line text editor that allows you to quickly write or edit short programs on
the go without opening any heavy editor application.

It comes with all the basic functionalities for a text editor.
Below are the shortcuts/key mappings supported by your distribution
of Tedit not including the basic keys of the keyboard:

CTRL + G ->                  			show this manual page
CTRL + S ->                  			save the contents to file
CTRL + Q ->                  			close the editor without saving
CTRL + B ->                  			save and close the editor
CTRL + T ->                  			save contents to another file name
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