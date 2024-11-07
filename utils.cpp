#include "utils.h"
#include <fstream>

bool useCol = 1;

Gamesave::Gamesave(const std::string& title, const u32& sz) 
: Title(title),
  BoardSize(sz) {
    board = u_board(sz, std::vector<u32>(sz,0));
    BiggestCellCifCount = 1;
    BiggestCell = 2;
    Score = 0;
    moved = 1;
}

Gamesave::Gamesave(const std::string& s) {
    std::ifstream is (s);
    is >> Title >> BoardSize;

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

void Gamesave::SaveData(const std::string& s) {
    std::ofstream os (s);
    os << Title << '\n' << BoardSize << '\n';
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

RGB::RGB(const u8& r, const u8& g, const u8& b) : r(r), g(g), b(b) {}
RGB::RGB(const u8& c) : r(c), g(c), b(c) {}

std::wstring getCol(const RGB& rgb) {
    if(!useCol) return L"";
    std::wstringstream wss;
    wss << L"\033[38;2;" << rgb.r  << L';' << rgb.g << L';' << rgb.b << L'm';
    return wss.str();
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
    char c = getchar();
    if(c == '\033') {
        getchar();
        switch(getchar()) {
            case 'A': return 'w';
            case 'B': return 's';
            case 'C': return 'd';
            case 'D': return 'a';
        }
    } return c == '\n' ? ' ' : c;
    #endif
}
