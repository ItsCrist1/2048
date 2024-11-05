#include "game.h"
#include "utils.h"

#include <iostream>

#ifdef _WIN32
#include <windows.h>
#else
#include <locale>
#include <termios.h>
#include <unistd.h>
#endif

const RGB SelectedColor = RGB(245,212,66);
const RGB UnselectedColor = RGB(112,109,96);

void draw(const u32& idx) {
    std::wcout << ANSI_CLEAR;
    std::wcout << getCol(!idx?SelectedColor:UnselectedColor) << L"1) New\n";
    std::wcout << getCol(idx==1?SelectedColor:UnselectedColor) << L"2) Load\n";
    std::wcout << getCol(idx==2?SelectedColor:UnselectedColor) << L"3) Quit\n";
    std::wcout << ANSI_RESET;
}

i32 main(i32 argc, char *argv[]) {
    if(argc > 1) {
        for(u32 i=1; i < argc; i++) {
            if(std::string(argv[i]) == "--nocolor") useCol = 0;
        }
    }

	#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8); // set UTF-8 locale for windows
	#else
	std::locale::global(std::locale("")); // set default locale for unix
    // set one character input with no echo in unix
	struct termios oldt, newt;
	tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	#endif
    
    u8 idx = 0;
    bool f = 1;

    while(f) {
        draw(idx);

        switch(getch()) {
            case 'w':
            case 'a': idx += idx ? -1 : 2; break;

            case 's':
            case 'd': idx += idx < 2 ? 1 : -2; break;

            case ' ':
            switch(idx) {
                case 0: Game(4); break;
                case 2: f = false; break;
            } break;

            case 'q': f = false; break;
        }
    }

	#ifndef _WIN32
    // disable one character input in unix
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	#endif

    return 0;
}
