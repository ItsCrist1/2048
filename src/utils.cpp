#include "utils.h"
#include <iostream>
#include <fstream>
#include <exception>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#undef RGB
#else
#include <termio.h>5
#include <csignal>
#include <unistd.h>

struct termios oldt, newt;

void initTerminalStates() {
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);

    std::signal(SIGINT, cleanup);
    std::signal(SIGTERM, cleanup);
    std::signal(SIGSEGV, cleanup);
}

void setTerminalState(const struct termios& s) {
    tcsetattr(STDIN_FILENO, TCSANOW, &s);
}

void cleanup(i32 sig) {
    setTerminalState(oldt);
    std::exit(sig);
}

#endif 

bool useCol = true, useTitle = true;
const u8 MinTitleWidth = 40;

std::wstring stw(const std::string& s) {
    return {s.begin(), s.end()};
}

std::wstring ga(u32 a, u32 b) { 
    return a==b && !useCol ? L"*\n" : L"\n";
}

u32 getWidth() {
    #ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    HANDLE hc = GetStdHandle(STD_OUTPUT_HANDLE);
    
    if(hc == INVALID_HANDLE_VALUE
       || !GetConsoleScreenBufferInfo(hc, &csbi)) return 0;
    
    return csbi.srWindow.Right - csbi.srWindow.Left + 1;

    #else
    struct winsize wsz;
    return ioctl(STDOUT_FILENO, TIOCGWINSZ, &wsz)==-1? 0 : wsz.ws_col;
    #endif
}

void clearScreen() {
    #ifdef _WIN32
    std::system("cls");
    #else
    std::wcout << L"\033[2J\033[H";
    #endif
}

void outputTitle() {
    if(!useTitle || getWidth() < MinTitleWidth) {
        std::wcout << getCol(ValueColor) << L"--- 2048 ---\n\n";
        return;
    }

    std::wcout << getCol(ValueColor)
               << LR"(  /$$$$$$   /$$$$$$  /$$   /$$  /$$$$$$ )" << L'\n'
               << LR"( /$$__  $$ /$$$_  $$| $$  | $$ /$$__  $$)" << L'\n'
               << LR"(|__/  \ $$| $$$$\ $$| $$  | $$| $$  \ $$)" << L'\n'
               << LR"(  /$$$$$$/| $$ $$ $$| $$$$$$$$|  $$$$$$/)" << L'\n'
               << LR"( /$$____/ | $$\ $$$$|_____  $$ >$$__  $$)" << L'\n'
               << LR"(| $$      | $$ \ $$$      | $$| $$  \ $$)" << L'\n'
               << LR"(| $$$$$$$$|  $$$$$$/      | $$|  $$$$$$/)" << L'\n'
               << LR"(|________/ \______/       |__/ \______/ )" << L"\n\n";
}

template u8 readBF<u8>(std::ifstream& is);
template u32 readBF<u32>(std::ifstream& is);
template Difficulty readBF<Difficulty>(std::ifstream& is);
template bool readBF<bool>(std::ifstream& is);

template <typename T>
T readBF(std::ifstream& is) {
    T n;
    is.read(reinterpret_cast<char*>(&n), sizeof(T));
    return n;
}

template void writeBF<u8>(std::ofstream& os, u8 n);
template void writeBF<u32>(std::ofstream& os, u32 n);
template void writeBF<Difficulty>(std::ofstream& os, Difficulty n);
template void writeBF<bool>(std::ofstream& os, bool n);

template <typename T>
void writeBF(std::ofstream& os, T n) {
    os.write(reinterpret_cast<char*>(&n), sizeof(T));
}

bool ValidateMagicNumber(const std::string& s) {
    try {
        std::ifstream is (s, std::ios::binary);
        return readBF<u8>(is) == ValidationMagicNumber;
    } catch(const std::exception& ex) { return false; }
}

Gamesave::Gamesave(const std::string& s, u32 sz, Difficulty dif) 
: BoardSize(sz),
  difficulty(dif) {
    Path = s;
    DetermineTitle();
    board = u_board(sz, std::vector<u8>(sz,0));
    BiggestCellCifCount = 1;
    BiggestCell = 1;
    Score = 0;
    moved = true;
}

Gamesave::Gamesave(const std::string& s) {
    Path = s;
    DetermineTitle();
    if((isValid = ValidateMagicNumber(s))) LoadData();
}


void Gamesave::LoadData() {
    DetermineTitle();

    std::ifstream is (Path, std::ios::binary);
    readBF<u8>(is);

    BoardSize = readBF<u32>(is);
    difficulty = readBF<Difficulty>(is);

    board = u_board(BoardSize, std::vector<u8>(BoardSize));
    for(u32 y=0; y < BoardSize; y++)
        for(u32 x=0; x < BoardSize; x++)
            board[y][x] = readBF<u8>(is);

    BiggestCellCifCount = readBF<u32>(is);
    BiggestCell = readBF<u8>(is);
    Score = readBF<u64>(is);
    newPos = { readBF<u32>(is), readBF<u32>(is) };
    moved = is.get() != 0;
    is.close();
}

void Gamesave::SaveData() {
    std::ofstream os (Path, std::ios::binary);
    writeBF<u8>(os, ValidationMagicNumber);

    writeBF<u32>(os, BoardSize);
    writeBF<Difficulty>(os, difficulty);

    for(const std::vector<u8>& v : board)
        for(const u32& i : v)
            writeBF<u8>(os, i);

    writeBF<u32>(os, BiggestCellCifCount);
    writeBF<u8>(os, BiggestCell);
    writeBF<u64>(os, Score);

    writeBF<u32>(os, newPos.first);
    writeBF<u32>(os, newPos.second);
    writeBF<bool>(os, moved);
    os.close();
}

void Gamesave::DetermineTitle() {
    Title = fs::path(Path).stem().string();
}

Stats::Stats(const std::string& s, bool b) {
    path = s;

    if(b) {
        if((isValid = ValidateMagicNumber(s)))
            LoadData();
    } else {
        moves.fill(0);
        biggestTile = 1;
        biggestScore = 0;
        SaveData();
    }
}

void Stats::SaveData() {
    std::ofstream os (path, std::ios::binary);
    writeBF<u8>(os, ValidationMagicNumber);
    
    for(u32 i=0; i < 4; i++) writeBF<u32>(os, moves[i]);
    writeBF<u8>(os, biggestTile);
    writeBF<u64>(os, biggestScore);

    os.close();
};

void Stats::LoadData() {
    std::ifstream is (path, std::ios::binary);
    readBF<u8>(is);

    for(u32 i=0; i < 4; i++) moves[i] = readBF<u32>(is);
    biggestTile = readBF<u8>(is);
    biggestScore = readBF<u64>(is);

    is.close();
}

RGB::RGB(u8 r, u8 g, u8 b) : r(r), g(g), b(b) {}
RGB::RGB(u8 c) : r(c), g(c), b(c) {}

std::wstring getCol(RGB rgb) {
    if(!useCol) return L"";
    std::wstringstream wss;
    wss << L"\033[38;2;" << rgb.r  << L';' << rgb.g << L';' << rgb.b << L'm';
    return wss.str();
}

std::wstring getCol() {
    return L"\033[0m";
}

void flushInputBuffer() {
    #ifdef _WIN32
    while(_kbhit()) _getch(); 
    #else
    tcflush(STDIN_FILENO, TCIFLUSH);
    #endif
}

#include <limits>

void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
}

char getChar() {
    #ifdef _WIN32
    int c = _getch();
    if (c == 224 || c == 0) {
        switch(_getch()) {
            case 72: return 'w';
            case 80: return 's';
            case 75: return 'a';
            case 77: return 'd';
            
            default: return 0;
        }
    } return c == 13 ? ' ' : (char)c;
    #else
    setTerminalState(newt);
    char c = getchar();
    if(c == '\033') {
        getchar();
        switch(getchar()) {
            case 'A': return 'w';
            case 'B': return 's';
            case 'C': return 'd';
            case 'D': return 'a';
        }
    } setTerminalState(oldt);

    return c == '\n' ? ' ' : c;
    #endif
}
