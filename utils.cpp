#include "utils.h"
#include <exception>
#include <fstream>

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

u32 readu32(std::ifstream& is) {
    u32 n;
    is.read(reinterpret_cast<char*>(&n), u32sz);
    return n;
}

void writeu32(std::ofstream& os, u32 n) {
    os.write(reinterpret_cast<char*>(&n), u32sz);
}

Gamesave::Gamesave(const std::string& s, const u32& sz) 
: BoardSize(sz) {
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
    if(isValid = ValidateMagicNumber(s)) LoadData();
}


void Gamesave::LoadData() {
    DetermineTitle();

    std::ifstream is (Path, std::ios::binary);
    readu32(is);

    BoardSize = readu32(is);
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

void flushInputBuffer() {
    #ifdef _WIN32
    while(_kbhit()) _getch(); 
    #else
    tcflush(STDIN_FILENO, TCIFLUSH);
    #endif
}

char getChar() {
    #ifdef _WIN32
    char c = _getch();
    if(c == 224) {
        switch(_getch()) {
            case 72: return 'w';
            case 80: return 's';
            case 75: return 'a';
            case 77: return 'd';
        }
    } return c == 13 ? ' ' : c;
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
