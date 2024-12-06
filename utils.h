#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <filesystem>
#include <array>

namespace fs = std::filesystem;

using u8 = uint8_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i32 = int32_t;
using u_board = std::vector<std::vector<u8>>;
using b_board = std::vector<std::vector<bool>>;

extern bool useCol;

enum Difficulty { Potato, Easy, Medium, Hard };

#ifndef _WIN32
void initTerminalStates();
void cleanup(i32);
#endif

const u32 ValidationMagicNumber = 0x2D; // radical of 2048 is ~45.25, 45 in hexadecimal is 0x2D :)

std::wstring stw(const std::string&);
std::wstring ga(const u32&, const u32);

u32 getWidth();

void clearScreen();

template <typename T>
T readBF(std::ifstream&);
template <typename T>
void writeBF(std::ofstream&, T);

struct RGB {
    const u8 r,g,b;

    RGB(const u8& r, const u8& g, const u8& b);
    RGB(const u8& c=0);
};

struct Gamesave {
    std::string Title, Path;
    bool isValid;
    
    u64 Score;
    u32 BoardSize, BiggestCellCifCount;
    u8 BiggestCell;
    u_board board;
    std::pair<u32,u32> newPos;
    bool moved;
    Difficulty difficulty;
    
    Gamesave(const std::string&, const u32&, const Difficulty&);
    Gamesave(const std::string&);
    void SaveData();
    void LoadData();
    void DetermineTitle();
};

struct Stats {
    std::string path;
    bool isValid;

    std::array<u32,4> moves;
    u8 biggestTile;
    u64 biggestScore;
    
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
