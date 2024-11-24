#include "utils.h"
#include <csignal>
#include <fstream>

#ifdef _WIN32
#include <conio.h>
#else
#include <termio.h> 
#include <unistd.h>
#include <cstdlib>

static struct termios oldt;

void cleanup(int) {
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    exit(0);
}

void initializeExitSignals() {
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);
    signal(SIGSEGV, cleanup);
    signal(SIGABRT, cleanup);
}

#endif 

bool useCol = 1;

std::wstring stw(const std::string& s) {
    return std::wstring(s.begin(), s.end());
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

Gamesave::Gamesave(const std::string& s) {
    Path = s;
    DetermineTitle();
    LoadData();
}

void Gamesave::LoadData() {
    DetermineTitle();

    std::ifstream is (Path);
    is >> BoardSize;

    board = u_board(BoardSize, std::vector<u32>(BoardSize,0));

    for(u32 y=0; y < BoardSize; y++)
        for(u32 x=0; x < BoardSize; x++)
            is >> board[y][x];
    
    is >> BiggestCellCifCount 
       >> BiggestCell 
       >> Score 
       >> newPos.first 
       >> newPos.second 
       >> moved;

    is.close();
}

void Gamesave::SaveData() {
    std::ofstream os (Path);
    os << BoardSize << '\n';
    for(u32 y=0; y < BoardSize; y++) {
        for(u32 x=0; x < BoardSize ; x++) {
            os << board[y][x];
            if(x != BoardSize-1) os << ' ';
        } os << '\n';
    } 

    os << BiggestCellCifCount 
       << ' ' << BiggestCell 
       << ' ' << Score 
       << '\n' << newPos.first 
       << ' ' << newPos.second 
       << '\n' << moved;

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

char getch() {
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
    tcgetattr(STDIN_FILENO, &oldt);
    struct termios newt;
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    char c = getchar();
    if(c == '\033') {
        getchar();
        switch(getchar()) {
            case 'A': return 'w';
            case 'B': return 's';
            case 'C': return 'd';
            case 'D': return 'a';
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return c == '\n' ? ' ' : c;
    #endif
}
