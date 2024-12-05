#include "game.h"
#include "utils.h"

#include <cctype>
#include <chrono>
#include <cmath>
#include <iostream>
#include <thread>

#ifdef _WIN32
#include <conio.h>
#endif

u32 cifCount(const u32& n) {
    return (u32)(n ? std::log10(n)+1 : 1);
}

void Game::SaveStats() {
    stats->biggestTile = std::max(stats->biggestTile, s.BiggestCell);
    stats->biggestScore = std::max(stats->biggestScore, s.Score);
    stats->SaveData();
}

Game::Game(const Gamesave& save, const std::shared_ptr<Stats>& sp, bool f) 
    : s(save),
      stats(sp),
    BICSZ(1 << COLS.size()) {
    if(f) {
        AddCell();
        AddCell();
    }
    u8 state = 0;

    while(!state) {
        DrawBoard();
        const char c = std::tolower(getChar());

        if(c == 'q') {
            state = 1;
            s.SaveData();
        }

        if(Slide(c)) {
            if(IsOver()) state = 2;
            else if(s.moved) AddCell();
        }
    }
    
    if(state == 2) {
        std::wcout << getCol(GameOverColor) << L"\n\nGame Over!\n";
        std::wcout << getCol(ScoreColor) << L"Score: " << s.Score;
        std::wcout << getCol(BiggestCellColor) << L"\nBiggest: " << s.BiggestCell << getCol() << std::endl;
        
        std::this_thread::sleep_for(std::chrono::seconds(3));
        flushInputBuffer();
        std::wcout << L"\nPress any key to continue...";
        getChar();

        SaveStats();
        // questionable logic leaving the last save there, TODO fix when adding stats saving for PB's
    }
}

void Game::DrawBoard() {
    const u32 CELL_SZ = s.BiggestCellCifCount + 2;
    clearScreen();
    std::wcout << std::wstring(CELL_SZ/2, L' ') << getCol(TitleColor) << L"2048\n" << getCol() << TBL_CRS[2];
    for(u32 i=0; i < s.BoardSize; i++) {
        std::wcout << std::wstring(CELL_SZ, TBL_CRS[0]);
        if (i < s.BoardSize-1) std::wcout << TBL_CRS[7];
    } std::wcout << TBL_CRS[3] << L'\n';

    for(u32 y=0; y < s.BoardSize; y++) {
        if(y) {
            std::wcout << TBL_CRS[8];
            for(u32 i=0; i < s.BoardSize; i++)
                std::wcout << std::wstring(CELL_SZ, TBL_CRS[0]) << TBL_CRS[i==s.BoardSize-1?9:10];
            std::wcout << L'\n';
        }

        std::wcout << TBL_CRS[1];
        for(u32 x=0; x < s.BoardSize; x++) {
            const u32 n = s.board[y][x] ? (1 << s.board[y][x]) : 0, cc = cifCount(n);
            s.BiggestCellCifCount = std::max(s.BiggestCellCifCount, cc);
            const u32 PAD_SZ = CELL_SZ - cc;
            std::wcout << std::wstring(PAD_SZ/2+PAD_SZ%2,L' ');
            if(s.newPos.first == x && s.newPos.second == y) std::wcout << getCol(NewColor);
            else if(n <= BICSZ && n) std::wcout << getCol(COLS.at(n));
            if(!n) std::wcout << L' ';
            else std::wcout << n;
            std::wcout << getCol()
                       << std::wstring(PAD_SZ/2, L' ') << TBL_CRS[1];
        } std::wcout << std::endl;
    }

    std::wcout << TBL_CRS[4];
    for(u32 i=0; i < s.BoardSize; i++) {
        std::wcout << std::wstring(CELL_SZ, TBL_CRS[0]);
        if (i < s.BoardSize-1) std::wcout << TBL_CRS[6];
    } std::wcout << TBL_CRS[5] << L"\n\n";

    std::wcout << getCol(ScoreColor) << L"Score: " << s.Score;
    std::wcout << getCol(BiggestCellColor) << L"\nBiggestCell: " << s.BiggestCell << getCol() << std::endl;
}

bool Game::Slide(const char& c) {
    b_board combined = std::vector<std::vector<bool>>(s.BoardSize, std::vector<bool>(s.BoardSize, 0));
    s.moved = 0;
    switch (c) {
        case 'w':
        stats->moves[0]++;
        for(u32 x=0; x < s.BoardSize; x++) {
            for(u32 y=1; y < s.BoardSize; y++) {
                if(!s.board[y][x]) continue;
                u32 i = y;
                while(i && !s.board[i-1][x]) i--;

                if(i) {
                    u8* v[3] = { &s.board[y][x], &s.board[i-1][x], &s.board[i][x] };
                    Combine_Move(v, {1, i != y}, combined[i-1][x]);
                } else {
                    u8* v[3] = { &s.board[y][x], nullptr, &s.board[i][x] };
                    Combine_Move(v, {0, i != y}, combined[i][x]);
                }
            }
        } break;

        case 's':
        stats->moves[1]++;
        for(u32 x=0; x < s.BoardSize; x++) {
            for(u32 y=s.BoardSize-2; y != (u32)-1; y--) {
                if(!s.board[y][x]) continue;
                u32 i = y;
                while(i < s.BoardSize-1 && !s.board[i+1][x]) i++;
                
                if(i < s.BoardSize-1) {
                    u8* v[3] = { &s.board[y][x], &s.board[i+1][x], &s.board[i][x]  };
                    Combine_Move(v, {1, i != y}, combined[i+1][x]);
                } else {
                    u8* v[3] = { &s.board[y][x], nullptr, &s.board[i][x] };
                    Combine_Move(v, {0, i != y}, combined[i][x]);
                }
            }
        } break;

        case 'a':
        stats->moves[2]++;
        for(u32 y=0; y < s.BoardSize; y++) {
            for(u32 x=1; x < s.BoardSize; x++) {
                if(!s.board[y][x]) continue;
                
                u32 i = x;
                while(i && !s.board[y][i-1]) i--;

                if(i) {
                    u8* v[3] = { &s.board[y][x], &s.board[y][i - 1], &s.board[y][i] };
                    Combine_Move(v, { 1, i != x }, combined[y][i]);
                } else {
                    u8* v[3] = { &s.board[y][x], nullptr, &s.board[y][i] };
                    Combine_Move(v, { 0, i != x }, combined[y][i]);
                }
            }
        } break;

        case 'd':
        stats->moves[3]++;
        for(u32 y=0; y < s.BoardSize; y++) {
            for(u32 x=s.BoardSize-2; x != (u32)-1; x--) {
                if(!s.board[y][x]) continue;
                u32 i = x;
                while(i < s.BoardSize-1 && !s.board[y][i+1]) i++;
                
                if(i < s.BoardSize-1) {
                    u8* v[3] = { &s.board[y][x], &s.board[y][i+1], &s.board[y][i] };
                    Combine_Move(v, { 1, i != x }, combined[y][i+1]);
                } else {
                    u8* v[3] = { &s.board[y][x], nullptr, &s.board[y][i] };
                    Combine_Move(v, { 0, i != x }, combined[y][i]);
                }
            }
        } break;

        default: return 0;
    } return 1;
}

void Game::Combine_Move(u8* v[3], const std::pair<bool,bool>& f, std::vector<bool>::reference c) {
    if(f.first && *v[0] == *v[1] && !c) {
        (*v[1])++; 
        *v[0] = 0;
        c = 1;

        s.BiggestCell = s.BiggestCell < *v[1] ? *v[1] : s.BiggestCell;
        s.Score += *v[1];
        s.moved = 1;
    } else if(f.second) {
        *v[2] = *v[0];
        *v[0] = 0;
        s.moved = 1;
    }
}

void Game::AddCell() {
    std::vector<std::pair<u32,u32>> v;

    for(u32 y=0; y < s.BoardSize; y++)
        for(u32 x=0; x< s.BoardSize; x++)
            if(!s.board[y][x]) v.push_back(std::pair<u32,u32>(x,y));
    
    const std::pair<u32,size_t> p = v[getRand(0,v.size()-1)];
    s.board[p.second][p.first] = getCell(s.difficulty);
    s.newPos = p;
}

bool Game::IsOver() {
    const u32 bszd = s.BoardSize - 1;
    for(u32 y=0; y < s.BoardSize; y++) {
        for(u32 x=0; x < s.BoardSize; x++) {
            if(!s.board[y][x]) return 0;
            if((x < bszd && s.board[y][x] == s.board[y][x+1])
                || (y < bszd && s.board[y][x] == s.board[y+1][x])) return 0;
        }
    } return 1;
}

u32 Game::getCell(const Difficulty& dif) {
    switch(dif) {
        case Potato: return 2;
        case Easy: return getRand(0,6) ? 1 : 2; 
        case Medium: return getRand(0,10) ? 1 : 2;
        case Hard: return 1;
    } return 0;
}

u32 Game::getRand(const u32& mn, const u32& mx) {
    return std::uniform_int_distribution<u32>(mn,mx)(rng);
}
