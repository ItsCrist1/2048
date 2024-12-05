#pragma once

#include "utils.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <random>

u32 cifCount(const u32& n);
u32 getBiggest(const u_board&);

class Game {
public:
    Game(const Gamesave&, const std::shared_ptr<Stats>&, bool);

private:
    const std::unordered_map<u32,RGB> COLS {
        { 2, RGB(238, 228, 218) },
        { 4, RGB(237, 224, 200) },
        { 8, RGB(242, 177, 121) },
        { 16, RGB(245, 149, 99) },
        { 32, RGB(246, 124, 95) },
        { 64, RGB(246, 94, 59) },
        { 128, RGB(237, 207, 114) },
        { 256, RGB(237, 204, 97) },
        { 512, RGB(237, 200, 80) },
        { 1024, RGB(237, 197, 63) },
        { 2048, RGB(237, 194, 46) }
    };
    const u32 BICSZ;
    const RGB TitleColor = RGB(17,85,194);
    const RGB GameOverColor = RGB(168,3,3);
    const RGB ScoreColor = RGB(219,222,47);
    const RGB BiggestCellColor = RGB(47,198,222);
    const RGB NewColor = RGB(220,227,25);
    const std::wstring TBL_CRS = L"─│┌┐└┘┴┬├┤┼";
    
    Gamesave s;
    std::shared_ptr<Stats> stats;
    std::mt19937 rng = std::mt19937(std::random_device{}());
    
    void SaveStats();
    void DrawBoard();
    bool Slide(const char&);
    void Combine_Move(u32*[3], const std::pair<bool,bool>&, std::vector<bool>::reference);
    void AddCell();
    bool IsOver();
    u32 getCell(const u32);
    u32 getRand(const u32& mn=0, const u32& mx=1);
};
