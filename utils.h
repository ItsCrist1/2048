#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

using u8 = uint8_t;
using u32 = uint32_t;
using i32 = int32_t;
using u_board = std::vector<std::vector<u32>>;
using b_board = std::vector<std::vector<bool>>;

extern bool useCol;

#ifndef _WIN32
void initTerminalStates();
void cleanup(i32);
#endif

const u32 ValidationMagicNumber = 0x2048;

std::wstring stw(const std::string&);
std::wstring ga(const u32&, const u32);

u32 getWidth();

void clearScreen();
u32 readu32(std::ifstream&);
void writeu32(std::ofstream&, u32);

struct RGB {
    const u8 r,g,b;

    RGB(const u8& r, const u8& g, const u8& b);
    RGB(const u8& c=0);
};

struct Gamesave {
    std::string Title, Path;
    bool isValid;

    u32 BoardSize, BiggestCellCifCount, BiggestCell, Score;
    u_board board;
    std::pair<u32,u32> newPos;
    bool moved;
    u32 difficulty;
    
    Gamesave(const std::string&, const u32&, const u32);
    Gamesave(const std::string&);
    void SaveData();
    void LoadData();
    void DetermineTitle();
};

struct Stats {
    std::string path;
    bool isValid;

    std::array<u32,4> moves;
    u32 biggestTile;
    u32 biggestScore;
    
    Stats();
    Stats(const std::string& s, bool);
    void SaveData();
    void LoadData();
};

std::wstring getCol(const RGB& rgb);
std::wstring getCol();
void flushInputBuffer();
void clearInputBuffer();
char getChar();
