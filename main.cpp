#include "game.h"
#include "utils.h"

#include <cctype>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#else
#include <locale>
#endif

const RGB SelectedColor = RGB(245,212,66);
const RGB UnselectedColor = RGB(112,109,96);
const std::string SaveDirectory = "saves";
const std::string DefaultSaveTitle = "Unnamed-Save-";
const std::string SaveExtension = ".dat";

std::vector<Gamesave> saves;
u32 savesSize;
u8 idx = 0;

void populateSaves() {
    saves.clear();
    u32 j = 0;
    for(const auto& i : fs::directory_iterator(SaveDirectory)) {
        const fs::path fp (i);
        if(!fs::is_regular_file(fp) || fp.extension().string() != SaveExtension) continue;
        Gamesave gs (i.path().string());
        if(gs.MagicNumber == ValidationMagicNumber) saves.push_back(gs);
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
        std::wcout << i+1 << L") " << stw(g.Title) << L" (" << fs::file_size(g.Path) << L" bytes)\n";
        std::wcout << ANSI_RESET << L"Board Size: " << g.BoardSize << L" | Score: " << g.Score << L"\n\n";
    }
}

void launchLoadMenu() {
    u32 si = 0;
    bool r = 0;
    
    while(1) {
        if(r) {
            populateSaves();
            r = 0;
        }

        drawLoad(si);
        const char c = std::tolower(getch());
        
        if(const u32 cd=c-'0'; 
            std::isdigit(c) && cd > 0
            && cd <= savesSize) {
            Game(saves[cd-1], 0);
            saves[cd-1].LoadData();
            continue;
        }

        Gamesave& gs = saves[si];
        std::string s, p;

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

            case 'r': r = 1; break;
            
            case 'y':
            std::wcout << ANSI_CLEAR << L"What do you want to rename " << stw(gs.Title) << "? ";
            std::cin >> s;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            p = fs::path(SaveDirectory) / fs::path(s + SaveExtension);
            if(!fs::exists(p)) {
                fs::rename(gs.Path, p);
                gs.Path = p;
                gs.Title = s;
            } else {
                std::wcerr << stw(s) << L" already exists ";
                getch();
            }
            break;

            case 'e':
            std::wcout << ANSI_CLEAR << L"Are you sure you want to delete ";
            std::wcout << stw(gs.Title) << "?\n";
            std::wcout << L"It has " << fs::file_size(gs.Path) << L" bytes.\n";
            std::wcout << L"[y/N] ";
            if(const char c=getch(); c == 'y' || c == 'Y') {
                fs::remove(gs.Path);
                saves.erase(saves.begin() + si);
                savesSize--;
            } break;
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
    initTerminalStates();
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

            case 'q': f = 0; break;
        }
    }

    return EXIT_SUCCESS;
}
