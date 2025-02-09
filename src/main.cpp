#include "game.h"
#include "utils.h"

#include <cctype>
#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#include <corecrt_io.h>
#include <fcntl.h>
#else
#include <locale>
#endif

const RGB SelectedColor = RGB(245,212,66);
const RGB UnselectedColor = RGB(112,109,96);

const fs::path SaveDirectory = "saves";
const fs::path DataDirectory = "data";
const std::string DefaultSaveTitle = "Unnamed-Save-";
const std::string SaveExtension = ".dat";
const std::string SettingsPath = (SaveDirectory / "settings").concat(SaveExtension);
const std::string StatsPath = (SaveDirectory / "stats").concat(SaveExtension);

const u32 MaxBoardSz = 128;

const std::wstring DIFF [4] = { L"Potato", L"Easy", L"Medium", L"Hard" };

std::vector<Gamesave> saves;
u32 savesSize;
u8 idx, si;

u32 boardSz;
Difficulty difficulty;

std::shared_ptr<Stats> stats;

bool LoadStats() {
    if(!fs::is_regular_file(StatsPath)) {
        stats = std::make_shared<Stats>(StatsPath, false);
        return true;
    }

    stats = std::make_shared<Stats>(StatsPath, true);
    if(!stats->isValid) {
        std::wcerr << L"Statistics file " << stw(StatsPath) << L" is not valid, would you like to delete it to be able to load in? [Y/n] ";
        if(std::tolower(getChar()) == 'n') return false;
        
        stats = std::make_shared<Stats>(StatsPath, false);
    } return true;
}

void populateSaves() {
    saves.clear();
    u32 j = 0;
    for(const auto& i : fs::directory_iterator(SaveDirectory)) {
        const fs::path& fp (i);
        if(!fs::is_regular_file(fp) || fp.extension().string() != SaveExtension) continue;
        const Gamesave gs (i.path().string());
        if(gs.isValid) { saves.push_back(gs); j++; }
    } savesSize = j;
}

void draw(u8 idx) {
    clearScreen();

    outputTitle();

    std::wcout << getCol(idx==0?SelectedColor:UnselectedColor) << L"1) New " << ga(idx,0)
               << getCol(idx==1?SelectedColor:UnselectedColor) << L"2) Load " << ga(idx,1)
               << getCol(idx==2?SelectedColor:UnselectedColor) << L"3) Settings " << ga(idx,2)
               << getCol(idx==3?SelectedColor:UnselectedColor) << L"4) Stats " << ga(idx,3)
               << getCol(idx==4?SelectedColor:UnselectedColor) << L"5) Help " << ga(idx,4)
               << getCol(idx==5?SelectedColor:UnselectedColor) << L"6) Quit " << ga(idx,5)
               << getCol();
}

std::string getFirstValidName() {
    u32 i = 1;
    for(; fs::is_regular_file(SaveDirectory / (DefaultSaveTitle + std::to_string(i) + SaveExtension)); i++);
    return (SaveDirectory / (DefaultSaveTitle + std::to_string(i) + SaveExtension)).string();
}

void drawLoad(u8 idx) {
    clearScreen();
    outputTitle();

    for(u32 i=0; i < savesSize; i++) {
        const Gamesave& g = saves[i];
        std::wcout << getCol(idx==i?SelectedColor:UnselectedColor)
                   << i+1 << L") " << stw(g.Title) << L" (" << fs::file_size(g.Path) << L" bytes) " << ga(idx,i)
                   << getCol() << L"Board Size: " << g.BoardSize << L" | Difficulty: " << DIFF[g.difficulty] << L'\n'
                   << L"Score: " << g.Score << L" Biggest Cell: " << (1<<g.BiggestCell) << L"\n\n";
    }
}

void launchLoadMenu() {
    populateSaves();

    if(saves.empty()) {
        std::wcout << L"No saves created yet\nPress any key to continue...";
        getChar();
        return;
    }

    bool f = true;
    std::wstring title;
    
    while(f) {
        populateSaves();

        if(si >= savesSize) si = savesSize - 1;

        drawLoad(si);
        const char c = std::tolower(getChar());
        
        if(const u32 cd=c-'0'; 
            std::isdigit(c) && cd > 0
            && cd <= savesSize) {
            Game(saves[cd-1], stats, false);
            saves[cd-1].LoadData();
            continue;
        }

        Gamesave& gs = saves[si];
        std::string s, p;

        switch(c) {
            case 'w':
            case 'a':
            si += si != 0 ? -1 : savesSize - 1; 
            break;

            case 's':
            case 'd':
            si += si < savesSize-1 ? 1 : 1 - savesSize; 
            break;

            case ' ': 
            Game(saves[si], stats, false); 
            saves[si].LoadData();
            break;

            case 'q': f = false; break;
            
            case 'y':
            clearScreen();
            title = stw(gs.Title);

            if (!fs::is_regular_file(gs.Path)) {
                std::wcerr << L"An error occured while trying to rename " << title << L"\nPress any key to continue...";
                getChar();
                break;
            }

            std::wcout << L"What do you want to rename " << title << L" to? ";
            std::cin >> s;
            clearInputBuffer();
            p = (SaveDirectory / fs::path(s + SaveExtension)).string();
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
            title = stw(gs.Title);
            
            if(!fs::is_regular_file(gs.Path)) {
                std::wcerr << L"An error occured while trying to delete " << title << L"\nPress any key to continue...";
                getChar();
                break;
            }

            std::wcout << L"Are you sure you want to delete ";
            std::wcout << title << "?\n";
            std::wcout << L"It has " << fs::file_size(gs.Path) << L" bytes.\n";
            std::wcout << L"[y/N] ";
            if(const char c=std::tolower(getChar()); c == 'y') {
                fs::remove(gs.Path);
                saves.erase(saves.begin() + si);
                savesSize--;
                if(si == savesSize) si--;

                if(savesSize == 0) {
                    std::wcout << L"\nNo saves left\nPress any key to continue...";
                    getChar();
                    f = false;
                }
            } break;
        }
    }
}

void setDefSettings() {
    boardSz = 4;
    difficulty = Medium;
    useCol = useTitle = true; 
}

void loadSettings() {
    std::ifstream is (SettingsPath, std::ios::binary);
    readBF<u8>(is);
    boardSz = readBF<u32>(is);
    difficulty = readBF<Difficulty>(is);
    useCol = is.get() != 0;
    useTitle = is.get() != 0;
    is.close();
}

void saveSettings() {
    std::ofstream os (SettingsPath, std::ios::binary);
    writeBF<u8>(os, ValidationMagicNumber);
    writeBF<u32>(os, boardSz);
    writeBF<Difficulty>(os, difficulty);
    writeBF<bool>(os, useCol);
    writeBF<bool>(os, useTitle);
    os.close();
}

bool LoadSettings() {
    if(!fs::is_regular_file(SettingsPath)) {
        setDefSettings();
        saveSettings();
        return true;
    }

    std::ifstream is (SettingsPath, std::ios::binary);
    if(readBF<u8>(is) == ValidationMagicNumber) {
        is.close();
        loadSettings();
        return true;
    } else {
        is.close();
        std::wcerr << L"Settings file " << stw(SettingsPath) << L" is not valid, would you like to delete it to be able to load in? [Y/n] ";
        if(std::tolower(getChar()) == 'n') return false;
        
        setDefSettings();
        saveSettings();
    } return true;
}

void drawSettings(u8 idx) {
    clearScreen();
    outputTitle();

    std::wcout << getCol(idx==0?SelectedColor:UnselectedColor) << L"1) Change size: " << boardSz << L' ' << ga(idx,0)
               << getCol(idx==1?SelectedColor:UnselectedColor) << L"2) Change difficulty: " << DIFF[difficulty] << L' ' << ga(idx,1)
               << getCol(idx==2?SelectedColor:UnselectedColor) << L"3) Color Support: " << (useCol?L"On ":L"Off ") << ga(idx,2)
               << getCol(idx==3?SelectedColor:UnselectedColor) << L"4) Draw Full Title: "<< (useTitle?L"Yes ":L"No ") << ga(idx,3)
               << getCol(idx==4?SelectedColor:UnselectedColor) <<  L"5) Back " << ga(idx,4)
               << getCol();
}

void drawDifficulty(u8 idx) {
    clearScreen();
    std::wcout << getCol(idx==0?RGB(73,245,85):UnselectedColor) << L"1) [Potato] " << ga(idx,0)
               << getCol(idx==1?RGB(47,176,56):UnselectedColor) << L"2) [Easy]" << ga(idx,1)
               << getCol(idx==2?RGB(190,232,3):UnselectedColor) << L"3) [Medium]" << ga(idx,2)
               << getCol(idx==3?RGB(227,40,11):UnselectedColor) << L"4) [Hard] " << ga(idx,3)
               << getCol(idx==4?SelectedColor:UnselectedColor) << L"5) Back " << ga(idx,4) << L'\n' << getCol();
}

void launchDifficulties() {
    u8 idx = difficulty;
    bool f = true;

    while(f) {
        drawDifficulty(idx);
        const char c = getChar();
            
        if(const u32 n=c-'0'-1; std::isdigit(c)) {
            if(n < 4) {
                difficulty = (Difficulty)n;
                saveSettings();
                f = false;
            } else f = n != 4;
        }
            

        switch(c) {
            case 'w':
            case 'd':
            idx += idx != 0 ? -1 : 4;
            break;

            case 's':
            case 'a':
            idx += idx < 4 ? 1 : -4;
            break;

            case ' ':
            if(idx != 4) { difficulty = (Difficulty)idx; saveSettings(); }
            f = false;
            break;

            case 'q': f = false; break;
        }
    }
}

bool execSettings(u8 idx) {
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
        case 3: useTitle = !useTitle; saveSettings(); break;
        case 4: return false;
    } 
    return true;
}

void launchSettingsMenu() {
    u8 idx = 0;
    bool f = true;
    while(f) {
        drawSettings(idx);
        const char c = getChar();

        if(std::isdigit(c)) {
            f = execSettings(c-'0'-1);
            continue;
        }

        switch(c) {
            case 'w':
            case 'a': idx += idx != 0 ? -1 : 4; 
            break;

            case 's':
            case 'd': idx += idx < 4 ? 1 : -4;
            break;
            
            case ' ': f = execSettings(idx); break;

            case 'q': return;
        }
    }
}

bool exec(u8 idx) {
    switch(idx) {
        case 1:
        Game(
             Gamesave(getFirstValidName(),boardSz,difficulty), 
             stats, true);
        break;

        case 2:
        launchLoadMenu();
        break;

        case 3:
        launchSettingsMenu(); 
        break;

        case 4:
        std::wcout << L"\nSlides:\n"
                   << L"Right: " << getCol(ValueColor) << stats->moves[3]
                   << getCol() << L" Left: " << getCol(ValueColor) << stats->moves[2]
                   << getCol() << L"\nUp: " << getCol(ValueColor) << stats->moves[0]
                   << getCol() << L" Down: " << getCol(ValueColor) << stats->moves[1]

                   << getCol() << L"\n\nBiggest Score Achieved: " << getCol(ValueColor) << stats->biggestScore
                   << getCol() << L"\nBiggest Tile Achieved: " << getCol(ValueColor) << (1<<stats->biggestTile)

                   << L"\n\nPress any key to continue...";
        getChar();
        break;

        case 5:
        std::wcout << L"Instructions\n"
                   << getCol(ValueColor) << L"--- UI ---\n\n"
                   << getCol() << L"WASD or arrow keys to move\nEnter or space to select\nQ to quit/go back\nAny digit to select a menu item\n\n"
                   << getCol(ValueColor) << L"--- Load Menu ---\n\n"
                   << getCol() << L"Same controls as all the other UI\nr -> refresh saves\ny -> rename save\ne -> erase save\n\n"
                   << getCol(ValueColor) << L"--- Game ---\n\n"
                   << getCol() << L"WASD or arrow keys to move\nq -> quit"

                   << getCol(ValueColor) << L"\n\nPress any key to continue...";

        getChar();
        break;

        case 6: return false;
    } return true;
}

i32 main(i32 argc, char **argv) {
    if(!fs::is_directory(SaveDirectory)) fs::create_directory(SaveDirectory);

	#ifdef _WIN32
    if(!_setmode(_fileno(stdout), _O_U16TEXT)) {
        std::wcerr << L"Unable to set UTF-16 to the terminal, drawing the board will probably not work, do you wish to continue anyways? [y/N] ";
        if(std::tolower(getChar()) != 'y') return 1;
    }
	#else
	std::locale::global (std::locale(""));
    initTerminalStates();
	#endif

    if(!LoadSettings() || !LoadStats()) return 1;
    bool f = true;
    
    while(f) {
        draw(idx);
        const char c = getChar();

        if(std::isdigit(c)) {
            f = exec(c - '0');
            continue;
        }

        switch(c) {
            case 'w':
            case 'a': idx += idx != 0 ? -1 : 5; break;

            case 's':
            case 'd': idx += idx < 5 ? 1 : -5; break;

            case ' ': f = exec(idx+1); break;

            case 'q': f = false; break;
        }
    } return 0;
}
