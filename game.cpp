#include "game.h"
#include "utils.h"

#include <cctype>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <conio.h>
#endif

u32 cifCount(const u32& n) {
    return n ? std::log10(n)+1 : 1;
}

u32 getBiggest(const u_board& b) {
    u32 m = 0;
    for(const auto& v : b)
        for(const auto& i : v)
            m = std::max(m,cifCount(i));
    return m;
}

Game::Game(const u32& BOARD_SZ, const u_board* ib, const u32& ic) 
    : COLS {
        { 2, RGB(119, 110, 101) },
        { 4, RGB(119, 110, 101) },
        { 8, RGB(243, 178, 122) },
        { 16, RGB(246, 150, 100) },
        { 32, RGB(247, 124, 95) },
        { 64, RGB(247, 95, 59) },
        { 128, RGB(237, 208, 115) },
        { 256, RGB(237, 204, 98) },
        { 512, RGB(237, 201, 80) },
        { 1024, RGB(237, 197, 63) },
        { 2048, RGB(237, 194, 46) }
    },
    BICSZ(pow(2,COLS.size())),
    BOARD_SZ(BOARD_SZ),
    TitleColor(RGB(17,85,194)),
    GameOverColor(RGB(168,3,3)),
    ScoreColor(RGB(219,222,47)),
    BiggestCellColor(RGB(47,198,222)),
    NewColor(RGB(220,227,25)),
    TBL_CRS(L"─│┌┐└┘┴┬├┤┼"),

    rng(std::mt19937(std::random_device{}())),
    b(ib ? *ib : u_board(BOARD_SZ, std::vector<u32>(BOARD_SZ))),
    biggest(b.empty() ? 10 : getBiggest(b)),
    biggestCell(2), score(0), 
    moved(1) {
    for(u32 i=0; i < ic; i++) AddCell();
    u8 state = 0;
    
    while(!state) {
        DrawBoard();
        const char c = std::tolower(getch());
        if(c == 'q') state = 1;
        if(Slide(c) && !AddCell() && !canMove()) state = 2;
    }
    
    if(state == 2) {
        std::wcout << getCol(GameOverColor) << L"\n\nGame Over!\n";
        std::wcout << getCol(ScoreColor) << L"Score: " << score;
        std::wcout << getCol(BiggestCellColor) << L"\nBiggest: " << biggestCell << ANSI_RESET << std::endl;
    
        std::this_thread::sleep_for(std::chrono::seconds(5));
        getch();
    }
}

void Game::DrawBoard() {
    const u32 CELL_SZ = biggest + 2;
    std::wcout << ANSI_CLEAR << std::wstring(CELL_SZ/2, L' ') << getCol(TitleColor) << L"2048\n" << ANSI_RESET << TBL_CRS[2];
    for(u32 i=0; i < BOARD_SZ; i++) {
        std::wcout << std::wstring(CELL_SZ, TBL_CRS[0]);
        if (i < BOARD_SZ-1) std::wcout << TBL_CRS[7];
    } std::wcout << TBL_CRS[3] << L'\n';

    for(u32 y=0; y < BOARD_SZ; y++) {
        if(y) {
            std::wcout << TBL_CRS[8];
            for(u32 i=0; i < BOARD_SZ; i++)
                std::wcout << std::wstring(CELL_SZ, TBL_CRS[0]) << TBL_CRS[i==BOARD_SZ-1?9:10];
            std::wcout << L'\n';
        }

        std::wcout << TBL_CRS[1];
        for(u32 x=0; x < BOARD_SZ; x++) {
            const u32 n = b[y][x];
            const u32 PAD_SZ = CELL_SZ - cifCount(n);
            std::wcout << std::wstring(PAD_SZ/2+PAD_SZ%2,L' ');
            if(newp.first == x && newp.second == y) std::wcout << getCol(NewColor);
            else if(b[y][x] <= BICSZ && b[y][x]) std::wcout << getCol(COLS.at(b[y][x]));
            if(!n) std::wcout << L' ';
            else std::wcout << n;
            std::wcout << ANSI_RESET;
            std::wcout << std::wstring(PAD_SZ/2, L' ') << TBL_CRS[1];
        } std::wcout << std::endl;
    }

    std::wcout << TBL_CRS[4];
    for(u32 i=0; i < BOARD_SZ; i++) {
        std::wcout << std::wstring(CELL_SZ, TBL_CRS[0]);
        if (i < BOARD_SZ-1) std::wcout << TBL_CRS[6];
    } std::wcout << TBL_CRS[5] << L"\n\n";

    std::wcout << getCol(ScoreColor) << L"Score: " << score;
    std::wcout << getCol(BiggestCellColor) << L"\nBiggestCell: " << biggestCell << ANSI_RESET << L'\n';
    std::wcout << L"Has moved: " << (moved ? L"Yes!" : L"No¡") << std::endl;
}

bool Game::Slide(const char& c) {
    b_board combined = std::vector<std::vector<bool>>(BOARD_SZ, std::vector<bool>(BOARD_SZ, 0));
    moved = 0;
    switch (c) {
        case 'w':
        for(u32 x=0; x < BOARD_SZ; x++) {
            for(u32 y=1; y < BOARD_SZ; y++) {
                if(!b[y][x]) continue;
                u32 i = y;
                while(i && !b[i-1][x]) i--;

                if(i) {
                    u32* v[3] = { &b[y][x], &b[i-1][x], &b[i][x] };
                    Combine_Move(v, {i < BOARD_SZ - 1, i != y}, combined[i-1][x]);
                } else {
                    u32* v[3] = { &b[y][x], nullptr, &b[i][x] };
                    Combine_Move(v, {0, i != y}, combined[i][x]);
                }

                /*if(i && b[i-1][x] == b[y][x] && !combined[i-1][x]) {
                    b[i-1][x] *= 2;
                    b[y][x] = 0;
                    combined[i-1][x] = 1;

                    if(const u32 n=cifCount(b[i-1][x]); n > biggest) biggest = n;
                    biggestCell = biggestCell < b[i-1][x] ? b[i-1][x] : biggestCell;
                    score += b[i-1][x];
                    moved = 1;
                } else if(i != y) {
                    b[i][x] = b[y][x];
                    b[y][x] = 0;
                    moved = 1;
                }*/
            }
        } break;

        case 's':
        for(u32 x=0; x < BOARD_SZ; x++) {
            for(u32 y=BOARD_SZ-2; y != (u32)-1; y--) {
                if(!b[y][x]) continue;
                u32 i = y;
                while(i < BOARD_SZ-1 && !b[i+1][x]) i++;

                u32* v[3] = { &b[y][x], &b[i+1][x], &b[i][x]  };
                Combine_Move(v, {i < BOARD_SZ-1, i != y}, combined[i+1][x]);

                /*if(i < BOARD_SZ-1 && b[i+1][x] == b[y][x] && !combined[i+1][x]) {
                    b[i+1][x] *= 2;
                    b[y][x] = 0;
                    combined[i+1][x] = 1;

                    if(const u32 n=cifCount(b[i+1][x]); n > biggest) biggest = n;
                    biggestCell = biggestCell < b[i+1][x] ? b[i+1][x] : biggestCell;
                    score += b[i+1][x];
                    moved = 1;
                } else if(i != y) {
                    b[i][x] = b[y][x];
                    b[y][x] = 0;
                    moved = 1;
                }*/
            }
        } break;

        case 'a':
        for(u32 y=0; y < BOARD_SZ; y++) {
            for(u32 x=1; x < BOARD_SZ; x++) {
                if(!b[y][x]) continue;
                    u32 i = x;
                    while(i && !b[y][i-1]) i--;

                    u32* v[3] = { &b[y][x], &b[y][i-1], &b[y][i] };
                    Combine_Move(v, {i, i != x}, combined[y][i]);

                    /*if(i && b[y][i-1] == b[y][x] && !combined[y][i-1]) {
                        b[y][i-1] *= 2;
                        b[y][x] = 0;
                        combined[y][i-1] = 1;

                        if(const u32 n=cifCount(b[y][i-1]); n > biggest) biggest = n;
                        biggestCell = biggestCell < b[y][i-1] ? b[y][i-1] : biggestCell;
                        score += b[y][i-1];
                        moved = 1;
                    } else if(i != x) {
                        b[y][i] = b[y][x];
                        b[y][x] = 0;
                        moved = 1;
                    }*/
            }
        } break;

        case 'd':
        for(u32 y=0; y < BOARD_SZ; y++) {
            for(u32 x=BOARD_SZ-2; x != (u32)-1; x--) {
                if(!b[y][x]) continue;
                u32 i = x;
                while(i < BOARD_SZ-1 && !b[y][i+1]) i++;
                
                u32* v[3] = { &b[y][x], &b[y][i+1], &b[y][i] };
                Combine_Move(v, {i<BOARD_SZ-1,i!=x}, combined[y][i+1]);

                /*if(i < BOARD_SZ-1 && b[y][i+1] == b[y][x] && !combined[y][i+1]) {
                    b[y][i+1] *= 2;
                    b[y][x] = 0;
                    combined[y][i+1] = 1;

                    if(const u32 n=cifCount(b[y][i+1]); n > biggest) biggest = n;
                    biggestCell = biggestCell < b[y][i+1] ? b[y][i+1] : biggestCell;
                    score += b[y][i+1];
                    moved = 1;
                } else if(i != x) {
                    b[y][i] = b[y][x];
                    b[y][x] = 0;
                    moved = 1;
                }*/
            }
        } break;

        default: return 0;
    } return 1;
}

void Game::Combine_Move(u32* v[3], const std::pair<bool,bool>& f, std::vector<bool>::reference c) {
    if(f.first && *v[0] == *v[1] && !c) {
        *v[1] *= 2; 
        *v[0] = 0;
        c = 1;

        if(const u32 n=cifCount(*v[1]); n > biggest) biggest = n;
        biggestCell = biggestCell < *v[1] ? *v[1] : biggestCell;
        score += *v[1];
        moved = 1;
    } else if(f.second) {
        *v[2] = *v[0];
        *v[0] = 0;
        moved = 1;
    }
}

bool Game::AddCell() {
    std::vector<std::pair<u32,u32>> v;

    for(u32 y=0; y < BOARD_SZ; y++)
        for(u32 x=0; x< BOARD_SZ; x++)
            if(!b[y][x]) v.push_back(std::pair<u32,u32>(x,y));
    
    const u32 sz = v.size();
    std::wcout << L"Size: " << sz << std::endl;
    
    if(!sz) return 0;

    const std::pair<u32,u32> p = v[getRand(0,sz-1)];
    b[p.second][p.first] = getRand(0,10) ? 2 : 4;
    newp = p;
    return 1;
}

bool Game::canMove() {
    for(u32 y=0; y < BOARD_SZ-1; y++) {
        for(u32 x=0; x < BOARD_SZ-1; x++)
            if(b[y][x] == b[y][x+1] || b[y][x] == b[y+1][x]) return 1;
    } return 0;
}

u32 Game::getRand(const u32& mn, const u32& mx) {
    return std::uniform_int_distribution<u32>(mn,mx)(rng);
}
