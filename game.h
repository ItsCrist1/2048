#ifndef GAME_H
#define GAME_H

#include "utils.h"

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <random>

using u_board = std::vector<std::vector<u32>>;
using b_board = std::vector<std::vector<bool>>;

u32 cifCount(const u32& n);
u32 getBiggest(const u_board&);

class Game {
public:
    Game(const u32& BOARD_SZ=4, const u_board* ib=nullptr, const u32& ic=2);

private:
    const std::unordered_map<u32,RGB> COLS;
    const u32 BICSZ;
    const u32 BOARD_SZ;
    const RGB TitleColor;
    const RGB GameOverColor;
    const RGB ScoreColor;
    const RGB BiggestCellColor;
    const RGB NewColor;
    const std::wstring TBL_CRS;

    std::mt19937 rng;
    u_board b;
    u32 biggest, biggestCell, score;
    std::pair<u32,u32> newp;
    bool moved;

    void DrawBoard();
    bool Slide(const char&);
    void Combine_Move(u32*[3], const std::pair<bool,bool>&, std::vector<bool>::reference);
    bool AddCell();
    bool canMove();
    u32 getRand(const u32& mn=0, const u32& mx=1);
};

#endif
