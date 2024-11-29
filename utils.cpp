#include "utils.h"
#include <iostream>
#include <fstream>
#include <exception>

#ifdef _WIN32
#include <conio.h>
#else
#include <termio.h>
#include <csignal>

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

constexpr u8 u32sz = sizeof(u32);
bool useCol = 1;

std::wstring stw(const std::string& s) {
    return std::wstring(s.begin(), s.end());
}

std::wstring ga(const u32& a, const u32 b) { 
    return a==b && !useCol ? L"*\n" : L"\n";
}

void clearScreen() {
    #ifdef _WIN32
    std::system("cls");
    #else
    std::wcout << L"\033[2J\033[H";
    #endif
}

u32 readu32(std::ifstream& is) {
    u32 n;
    is.read(reinterpret_cast<char*>(&n), u32sz);
    return n;
}

void writeu32(std::ofstream& os, u32 n) {
    os.write(reinterpret_cast<char*>(&n), u32sz);
}

Gamesave::Gamesave(const std::string& s, const u32& sz, const u32 dif) 
: BoardSize(sz),
  difficulty(dif) {
    Path = s;
    DetermineTitle();
    board = u_board(sz, std::vector<u32>(sz,0));
    BiggestCellCifCount = 1;
    BiggestCell = 2;
    Score = 0;
    moved = 1;
}

bool ValidateMagicNumber(const std::string& s) {
    try {
        std::ifstream is (s, std::ios::binary);
        return readu32(is) == ValidationMagicNumber;
    } catch(std::exception ex) { return 0; }
}

Gamesave::Gamesave(const std::string& s) {
    Path = s;
    DetermineTitle();
    if((isValid = ValidateMagicNumber(s))) LoadData();
}


void Gamesave::LoadData() {
    DetermineTitle();

    std::ifstream is (Path, std::ios::binary);
    readu32(is);

    BoardSize = readu32(is);
    difficulty = readu32(is);

    board = u_board(BoardSize, std::vector<u32>(BoardSize));
    for(u32 y=0; y < BoardSize; y++)
        for(u32 x=0; x < BoardSize; x++)
            board[y][x] = readu32(is);

    BiggestCellCifCount = readu32(is);
    BiggestCell = readu32(is);
    Score = readu32(is);
    newPos = { readu32(is), readu32(is) };
    moved = is.get() != 0;
    is.close();
}

void Gamesave::SaveData() {
    std::ofstream os (Path, std::ios::binary);
    writeu32(os, ValidationMagicNumber);

    writeu32(os, BoardSize);
    writeu32(os, difficulty);

    for(const std::vector<u32>& v : board)
        for(const u32& i : v)
            writeu32(os, i);

    writeu32(os, BiggestCellCifCount);
    writeu32(os, BiggestCell);
    writeu32(os, Score);

    writeu32(os, newPos.first);
    writeu32(os, newPos.second);
    writeu32(os, moved);
    os.close();
}

void Gamesave::DetermineTitle() {
    Title = fs::path(Path).stem().string();
}

RGB::RGB(const u8& r, const u8& g, const u8& b) : r(r), g(g), b(b) {}
RGB::RGB(const u8& c) : r(c), g(c), b(c) {}

std::wstring getCol(const RGB& rgb) {
    if(!useCol) return L"";
    std::wstringstream wss;
    wss << L"\033[38;2;" << rgb.r  << L';' << rgb.g << L';' << rgb.b << L'm';
    return wss.str();
}

std::wstring getCol() {
    #ifdef _WIN32
    return L"";
    #else 
    return L"\033[0m";
    #endif
}

void flushInputBuffer() {
    #ifdef _WIN32
    while(_kbhit()) _getch(); 
    #else
    tcflush(STDIN_FILENO, TCIFLUSH);
    #endif
}

void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
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
