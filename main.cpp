#include "game.h"
#include "utils.h"

#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <locale>
#include <termios.h>
#include <unistd.h>
#endif

const RGB SelectedColor = RGB(245,212,66);
const RGB UnselectedColor = RGB(112,109,96);
const std::string SaveDirectory = "saves";
const std::string DefaultSaveTitle = "Unnamed-Save-";
const std::string SaveExtension = ".2048";

std::vector<Gamesave> saves;
u32 savesSize;
u8 idx = 0;

void populateSaves() {
    saves.clear();
    u32 j = 0;
    for(const auto& i : fs::directory_iterator(SaveDirectory)) {
        if(!i.is_regular_file()) continue;
        saves.push_back(Gamesave(i.path().string()));
        j++;
    } savesSize = j;
}

void draw(const u32& idx) {
    std::wcout << ANSI_CLEAR;
    std::wcout << getCol(!idx?SelectedColor:UnselectedColor) << L"1) New\n";
    std::wcout << getCol(idx==1?SelectedColor:UnselectedColor) << L"2) Load\n";
    std::wcout << getCol(idx==2?SelectedColor:UnselectedColor) << L"3) Quit\n";
    std::wcout << ANSI_RESET;
}

std::string getFirstValidName() {
    fs::path p (SaveDirectory);
    u32 i = 1;
    for(; fs::is_regular_file(p/fs::path(DefaultSaveTitle+std::to_string(i)+SaveExtension)); i++);
    return p / fs::path(DefaultSaveTitle + std::to_string(i) + SaveExtension);
}

void drawLoad(const u32& si) {
    std::wcout << ANSI_CLEAR;
    for(u32 i=0; i < savesSize; i++) {
        const Gamesave& g = saves[i];
        std::wcout << getCol(si==i?SelectedColor:UnselectedColor);
        std::cout << i+1 << ") " << g.Title << '\n';
        std::wcout << ANSI_RESET;
        std::cout << fs::file_size(g.Path) << " bytes\n";
        std::cout << "Board Size: " << g.BoardSize << "\n\n";
    }
}

void launchLoadMenu() {
    u32 si = 0;
    
    while(1) {
        drawLoad(si);
        const char c = getch();
        
        if(const u32 cd=c-'0'; 
            std::isdigit(c) && cd > 0
            && cd <= savesSize) {
            Game(saves[cd-1], 0);
            saves[cd-1].LoadData();
            continue;
        }

        switch(c) {
            case 'w':
            case 'a':
            si += si ? -1 : savesSize - 1; 
            break;

            case 's':
            case 'd':
            si += si < savesSize-1 ? 1 : -savesSize+1; 
            break;

            case ' ': 
            Game(saves[si], 0); 
            saves[si].LoadData();
            break;

            case 'q': return;
        }
    }
}

bool exec(const u8& idx) {
    switch(idx) {
        case 3: return 0;

        case 1:
        Game(Gamesave(getFirstValidName(),4), 1);
        break;

        case 2:
        populateSaves();
        launchLoadMenu();
        break;
    } return 1;
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

    bool f = 1;
    
    while(f) {
        draw(idx);
        const char c = getch();

        if(std::isdigit(c)) {
            f = exec(c - '0');
            continue;
        }

        switch(c) {
            case 'w':
            case 'a': idx += idx ? -1 : 2; break;

            case 's':
            case 'd': idx += idx < 2 ? 1 : -2; break;

            case ' ': f = exec(idx+1); break;

            case 'q': f = false; break;
        }
    }

	#ifndef _WIN32
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	#endif

    return EXIT_SUCCESS;
}
