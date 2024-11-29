#include "game.h"
#include "utils.h"

#include <cctype>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <iomanip>

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

const std::wstring DIFF [4] = { L"Potato", L"Easy", L"Medium", L"Hard" };

std::vector<Gamesave> saves;
u32 savesSize;
u8 idx = 0;

u32 boardSz;
u32 difficulty;

std::wstring getLastEdit(const std::string& s) {
    auto ftime = std::filesystem::last_write_time(s);
    auto sctp = std::chrono::system_clock::from_time_t(
        std::chrono::duration_cast<std::chrono::seconds>(ftime.time_since_epoch()).count());
    std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);

    std::wstringstream wss;
    wss << std::put_time(std::localtime(&cftime), L"%d.%m.%Y | %H:%M:%S");
    return wss.str();
}

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
    clearScreen();
    std::wcout << getCol(!idx?SelectedColor:UnselectedColor) << L"1) New " << ga(idx,0)
               << getCol(idx==1?SelectedColor:UnselectedColor) << L"2) Load " << ga(idx,1)
               << getCol(idx==2?SelectedColor:UnselectedColor) << L"3) Settings " << ga(idx,2)
               << getCol(idx==3?SelectedColor:UnselectedColor) << L"4) Quit " << ga(idx,3)
               << getCol();
}

std::string getFirstValidName() {
    fs::path p (SaveDirectory);
    u32 i = 1;
    for(; fs::is_regular_file(p/fs::path(DefaultSaveTitle+std::to_string(i)+SaveExtension)); i++);
    return p / fs::path(DefaultSaveTitle + std::to_string(i) + SaveExtension);
}

void drawLoad(const u32& idx) {
    clearScreen();
    for(u32 i=0; i < savesSize; i++) {
        const Gamesave& g = saves[i];
        std::wcout << getCol(idx==i?SelectedColor:UnselectedColor)
                   << i+1 << L") " << stw(g.Title) << L" (" << fs::file_size(g.Path) << L" bytes) " << ga(idx,i)
                   << getCol() << L"Board Size: " << g.BoardSize << L" | Difficulty: " << DIFF[g.difficulty] << L'\n'
                   << L"Score: " << g.Score << L" Biggest Cell: " << g.BiggestCell << L'\n'
                   << L"Last edited: " << getLastEdit(g.Path) << L"\n\n"; 
    }
}

void launchLoadMenu() {
    if(saves.empty()) {
        std::wcout << L"No saves created yet\nPress any key to continue...";
        getChar();
        return;
    }

    u32 si = 0;
    bool r = 0, f = 1;
    
    while(f) {
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

            case 'q': f = 0; break;

            case 'r': r = 1; break;
            
            case 'y':
            clearScreen();
            std::wcout << L"What do you want to rename " << stw(gs.Title) << L" to? ";
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
            clearScreen();
            std::wcout << L"Are you sure you want to delete ";
            std::wcout << stw(gs.Title) << "?\n";
            std::wcout << L"It has " << fs::file_size(gs.Path) << L" bytes.\n";
            std::wcout << L"[y/N] ";
            if(const char c=getChar(); c == 'y' || c == 'Y') {
                fs::remove(gs.Path);
                saves.erase(saves.begin() + si);
                savesSize--;

                if(!savesSize) {
                    std::wcout << L"\nNo saves left\nPress any key yo continue...";
                    getChar();
                    f = 0;
                }
            } break;
        }
    }
}

void setDefSettings() {
    boardSz = 4;
    difficulty = 2;
    
    #ifdef _WIN32
    useCol = 0;
    #else
    useCol = 0;
    #endif
}

void loadSettings() {
    std::ifstream is (SettingsPath, std::ios::binary);
    readu32(is);
    boardSz = readu32(is);
    difficulty = readu32(is);
    useCol = is.get() != 0;
    is.close();
}

void saveSettings() {
    std::ofstream os (SettingsPath, std::ios::binary);
    writeu32(os, ValidationMagicNumber);
    writeu32(os, boardSz);
    writeu32(os, difficulty);
    writeu32(os, useCol);
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
    clearScreen();
    std::wcout << getCol(!idx?SelectedColor:UnselectedColor) << L"1) Change size: " << boardSz << L' ' << ga(idx,0)
               << getCol(idx==1?SelectedColor:UnselectedColor) << L"2) Change difficulty: " << DIFF[difficulty] << L' ' << ga(idx,1)
               << getCol(idx==2?SelectedColor:UnselectedColor) << L"3) Color Support: " << (useCol?L"On ":L"Off ") << ga(idx,2)
               << getCol(idx==3?SelectedColor:UnselectedColor) <<  L"4) Back " << ga(idx,3)
               << getCol();
}

void drawDifficulty(const u8& idx) {
    clearScreen();
    std::wcout << getCol(!idx?RGB(73,245,85):UnselectedColor) << L"1) [Potato] " << ga(idx,0)
               << getCol(idx==1?RGB(47,176,56):UnselectedColor) << L"2) [Easy]" << ga(idx,1)
               << getCol(idx==2?RGB(190,232,3):UnselectedColor) << L"3) [Medium]" << ga(idx,2)
               << getCol(idx==3?RGB(227,40,11):UnselectedColor) << L"4) [Hard] " << ga(idx,3)
               << getCol(idx==4?SelectedColor:UnselectedColor) << L"5) Back " << ga(idx,4) << L'\n' << getCol();
}

void launchDifficulties() {
    u8 idx = difficulty;
    bool f = 1;

    while(f) {
        drawDifficulty(idx);
        const char c = getChar();
            
        if(const u32 n=c-'0'-1; std::isdigit(c)) {
            if(n < 4) {
                difficulty = n;
                saveSettings();
                f = 0;
            } else f = n != 4;
        }
            

        switch(c) {
            case 'w':
            case 'd':
            idx += idx ? -1 : 4;
            break;

            case 's':
            case 'a':
            idx += idx < 4 ? 1 : -4;
            break;

            case ' ':
            if(idx != 4) { difficulty = idx; saveSettings(); }
            f = 0;
            break;

            case 'q': f = 0; break;
        }
    }
}

bool execSettings(const u8& idx) {
    switch(idx) {
        case 0:
        clearScreen();
        std::wcout << L"Insert new value: ";
        if(u32 sz; !(std::cin >> sz) || sz < 2|| sz > MaxBoardSz) {
            std::wcout << L"\nError, invalid input, must be a value from 1 to " << MaxBoardSz << L"\nPress any key to continue...";
            clearInputBuffer();
            getChar();
        } else {
            boardSz = sz;
            saveSettings();
            clearInputBuffer();
        }
        break;

        case 1: launchDifficulties(); break;

        case 2: useCol = !useCol; saveSettings(); break;

        case 3: return 0;
    } 
    return 1;
}

void launchSettingsMenu() {
    u8 idx = 0;
    bool f = 1;
    while(f) {
        drawSettings(idx);
        const char c = getChar();

        if(std::isdigit(c)) {
            f = execSettings(c-'0'-1);
            continue;
        }

        switch(c) {
            case 'w':
            case 'a': idx += idx ? -1 : 3; 
            break;

            case 's':
            case 'd': idx += idx < 3 ? 1 : -3;
            break;
            
            case ' ': f = execSettings(idx); break;

            case 'q': return;
        }
    }
}

bool exec(const u8& idx) {
    switch(idx) {
        case 1:
        Game(Gamesave(getFirstValidName(),boardSz,difficulty), 1);
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

i32 main(i32 argc, char **argv) {
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
    } return 0;
}
