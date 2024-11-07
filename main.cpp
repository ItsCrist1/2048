#include "game.h"
#include "utils.h"

#include <cstdlib>
#include <iostream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#else
#include <locale>
#include <termios.h>
#include <unistd.h>
#endif

namespace fs = std::filesystem;

const RGB SelectedColor = RGB(245,212,66);
const RGB UnselectedColor = RGB(112,109,96);
const std::string SaveDirectory = "saves";

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
            const std::string arg = std::string(argv[i]);
            if(arg == "--nocolor") useCol = 0;
        }
    }

    if(!fs::is_directory(SaveDirectory)) fs::create_directory(SaveDirectory);

	#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	#else
	std::locale::global(std::locale(""));
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
                case 0: 
                // TODO
                break;
                case 2: f = false; break;
            } break;

            case 'q': f = false; break;
        }
    }

	#ifndef _WIN32
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	#endif

    return EXIT_SUCCESS;
}
