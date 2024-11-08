#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>

using u8 = uint8_t;
using u32 = uint32_t;
using i32 = int32_t;
using u_board = std::vector<std::vector<u32>>;
using b_board = std::vector<std::vector<bool>>;

extern bool useCol;

const std::wstring ANSI_CLEAR = L"\033[2J\033[H";
const std::wstring ANSI_RESET = L"\033[0m";

struct RGB {
    const u8 r,g,b;

    RGB(const u8& r, const u8& g, const u8& b);
    RGB(const u8& c=0);
};

struct Gamesave {
    std::string Title;
    u32 BoardSize, BiggestCellCifCount, BiggestCell, Score;
    u_board board;
    std::pair<u32,u32> newPos;
    bool moved;
    
    Gamesave(const std::string&, const u32& sz);
    Gamesave(const std::string&);
    void SaveData(const std::string&);
};

std::wstring getCol(const RGB& rgb);
char getch();

#endif
