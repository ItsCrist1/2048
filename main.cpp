#include "game.h"
#include "utils.h"

#include <cctype>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>

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
const std::string SettingsPath = "settings.dat";

const u32 MaxBoardSz = 128;

std::vector<Gamesave> saves;
u32 savesSize;
u8 idx = 0;

u32 boardSz;

void populateSaves() {
    saves.clear();
    u32 j = 0;
    for(const auto& i : fs::directory_iterator(SaveDirectory)) {
        const fs::path fp (i);
        if(!fs::is_regular_file(fp) || fp.extension().string() != SaveExtension) continue;
        const Gamesave gs (i.path().string());
        if(gs.isValid) { saves.push_back(gs); j++; }
    } savesSize = j;
}

void draw(const u32& idx) {
    std::wcout << ANSI_CLEAR
               << getCol(!idx?SelectedColor:UnselectedColor) << L"1) New\n"
               << getCol(idx==1?SelectedColor:UnselectedColor) << L"2) Load\n"
               << getCol(idx==2?SelectedColor:UnselectedColor) << L"3) Settings\n"
               << getCol(idx==3?SelectedColor:UnselectedColor) << L"4) Quit\n"
               << ANSI_RESET;
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
        const char c = std::tolower(getChar());
        
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
            std::wcout << ANSI_CLEAR << L"What do you want to rename " << stw(gs.Title) << L" to? ";
            std::cin >> s;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            p = fs::path(SaveDirectory) / fs::path(s + SaveExtension);
            if(!fs::exists(p)) {
                fs::rename(gs.Path, p);
                gs.Path = p;
                gs.Title = s;
            } else {
                std::wcerr << stw(s) << L" already exists ";
                getChar();
            }
            break;

            case 'e':
            std::wcout << ANSI_CLEAR << L"Are you sure you want to delete ";
            std::wcout << stw(gs.Title) << "?\n";
            std::wcout << L"It has " << fs::file_size(gs.Path) << L" bytes.\n";
            std::wcout << L"[y/N] ";
            if(const char c=getChar(); c == 'y' || c == 'Y') {
                fs::remove(gs.Path);
                saves.erase(saves.begin() + si);
                savesSize--;
            } break;
        }
    }
}

void setDefSettings() {
    boardSz = 4;
}

void loadSettings() {
    std::ifstream is (SettingsPath, std::ios::binary);
    readu32(is);
    boardSz = readu32(is); 
    is.close();
}

void saveSettings() {
    std::ofstream os (SettingsPath, std::ios::binary);
    writeu32(os, ValidationMagicNumber);
    writeu32(os, boardSz);
    os.close();
}

void LoadSettings() {
    if(fs::is_regular_file(SettingsPath)) {
        std::ifstream is (SettingsPath, std::ios::binary);
        if(readu32(is) == ValidationMagicNumber) {
            loadSettings();
            return;
        }
    } setDefSettings(); saveSettings();
}

void drawSettings(const u8& idx) {
    std::wcout << ANSI_CLEAR
               << getCol(!idx?SelectedColor:UnselectedColor) << L"1) Change size: " << boardSz << L'\n'
               << getCol(idx==1?SelectedColor:UnselectedColor) <<  L"2) Back\n"
               << ANSI_RESET;
}

bool execSettings(const u8& idx) {
    switch(idx) {
        case 0:
        std::wcout << ANSI_CLEAR << L"Insert new value: ";
        if(u32 sz; !(std::cin >> sz) || sz < 2|| sz > MaxBoardSz) {
            clearInputBuffer();
            std::wcout << L"\nError, invalid input, must be a value from 1 to " << MaxBoardSz << L"\nPress any key to continue...";
            getChar();
        } else {
            boardSz = sz;
            saveSettings();
        }
        break;

        case 1: return 0;
    } return 1;
}

void launchSettingsMenu() {
    u8 idx = 0;
    while(1) {
        drawSettings(idx);
        const char c = getChar();

        if(std::isdigit(c)) {
            if(!execSettings(c-'0'-1)) return;
            continue;
        }

        switch(c) {
            case 'w':
            case 'a': idx += idx ? -1 : 1; 
            break;

            case 's':
            case 'd': idx += idx < 1 ? 1 : -1;
            break;
            
            case ' ': if(!execSettings(idx)) return;

            case 'q': return;
        }
    }
}

bool exec(const u8& idx) {
    switch(idx) {
        case 1:
        Game(Gamesave(getFirstValidName(),boardSz), 1);
        break;

        case 2:
        populateSaves();
        launchLoadMenu();
        break;

        case 3:
        launchSettingsMenu(); 
        break;

        case 4: return 0;
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
    LoadSettings();

	#ifdef _WIN32
	SetConsoleOutputCP(CP_UTF8);
	#else
	std::locale::global (std::locale(""));
    initTerminalStates();
	#endif

    bool f = 1;
    
    while(f) {
        draw(idx);
        const char c = getChar();

        if(std::isdigit(c)) {
            f = exec(c - '0');
            continue;
        }

        switch(c) {
            case 'w':
            case 'a': idx += idx ? -1 : 3; break;

            case 's':
            case 'd': idx += idx < 3 ? 1 : -3; break;

            case ' ': f = exec(idx+1); break;

            case 'q': f = 0; break;
        }
    }

    return EXIT_SUCCESS;
}
