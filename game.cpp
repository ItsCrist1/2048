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
    return n ? std::log10(n)+1 : 1;
}

u32 getBiggest(const u_board& b) {
    u32 m = 0;
    for(const auto& v : b)
        for(const auto& i : v)
            m = std::max(m,cifCount(i));
    return m;
}

Game::Game(const Gamesave& save) 
    : s(save),
    BICSZ(pow(2,COLS.size())) {
    AddCell();
    AddCell();

    u8 state = 0;

    while(!state) {
        DrawBoard();
        const char c = std::tolower(getch());
        if(c == 'q') state = 1;
        if(Slide(c))
            if(s.moved && !AddCell() && !canMove()) state = 2;
    }
    
    if(state == 2) {
        std::wcout << getCol(GameOverColor) << L"\n\nGame Over!\n";
        std::wcout << getCol(ScoreColor) << L"s.Score: " << s.Score;
        std::wcout << getCol(BiggestCellColor) << L"\nBiggest: " << s.BiggestCell << ANSI_RESET << std::endl;
    
        std::this_thread::sleep_for(std::chrono::seconds(5));
        getch();
    } s.SaveData("saves/test.txt");
}

void Game::DrawBoard() {
    const u32 CELL_SZ = s.BiggestCellCifCount + 2;
    std::wcout << ANSI_CLEAR << std::wstring(CELL_SZ/2, L' ') << getCol(TitleColor) << L"2048\n" << ANSI_RESET << TBL_CRS[2];
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
            const u32 n = s.board[y][x];
            const u32 PAD_SZ = CELL_SZ - cifCount(n);
            std::wcout << std::wstring(PAD_SZ/2+PAD_SZ%2,L' ');
            if(s.newPos.first == x && s.newPos.second == y) std::wcout << getCol(NewColor);
            else if(s.board[y][x] <= BICSZ && s.board[y][x]) std::wcout << getCol(COLS.at(s.board[y][x]));
            if(!n) std::wcout << L' ';
            else std::wcout << n;
            std::wcout << ANSI_RESET;
            std::wcout << std::wstring(PAD_SZ/2, L' ') << TBL_CRS[1];
        } std::wcout << std::endl;
    }

    std::wcout << TBL_CRS[4];
    for(u32 i=0; i < s.BoardSize; i++) {
        std::wcout << std::wstring(CELL_SZ, TBL_CRS[0]);
        if (i < s.BoardSize-1) std::wcout << TBL_CRS[6];
    } std::wcout << TBL_CRS[5] << L"\n\n";

    std::wcout << getCol(ScoreColor) << L"s.Score: " << s.Score;
    std::wcout << getCol(BiggestCellColor) << L"\ns.BiggestCell: " << s.BiggestCell << ANSI_RESET << std::endl;
}

bool Game::Slide(const char& c) {
    b_board combined = std::vector<std::vector<bool>>(s.BoardSize, std::vector<bool>(s.BoardSize, 0));
    s.moved = 0;
    switch (c) {
        case 'w':
        for(u32 x=0; x < s.BoardSize; x++) {
            for(u32 y=1; y < s.BoardSize; y++) {
                if(!s.board[y][x]) continue;
                u32 i = y;
                while(i && !s.board[i-1][x]) i--;

                if(i) {
                    u32* v[3] = { &s.board[y][x], &s.board[i-1][x], &s.board[i][x] };
                    Combine_Move(v, {1, i != y}, combined[i-1][x]);
                } else {
                    u32* v[3] = { &s.board[y][x], nullptr, &s.board[i][x] };
                    Combine_Move(v, {0, i != y}, combined[i][x]);
                }
            }
        } break;

        case 's':
        for(u32 x=0; x < s.BoardSize; x++) {
            for(u32 y=s.BoardSize-2; y != (u32)-1; y--) {
                if(!s.board[y][x]) continue;
                u32 i = y;
                while(i < s.BoardSize-1 && !s.board[i+1][x]) i++;

                u32* v[3] = { &s.board[y][x], &s.board[i+1][x], &s.board[i][x]  };
                Combine_Move(v, {i < s.BoardSize-1, i != y}, combined[i+1][x]);
            }
        } break;

        case 'a':
        for(u32 y=0; y < s.BoardSize; y++) {
            for(u32 x=1; x < s.BoardSize; x++) {
                if(!s.board[y][x]) continue;
                    u32 i = x;
                    while(i && !s.board[y][i-1]) i--;

                    u32* v[3] = { &s.board[y][x], &s.board[y][i-1], &s.board[y][i] };
                    Combine_Move(v, {i, i != x}, combined[y][i]);
            }
        } break;

        case 'd':
        for(u32 y=0; y < s.BoardSize; y++) {
            for(u32 x=s.BoardSize-2; x != (u32)-1; x--) {
                if(!s.board[y][x]) continue;
                u32 i = x;
                while(i < s.BoardSize-1 && !s.board[y][i+1]) i++;
                
                u32* v[3] = { &s.board[y][x], &s.board[y][i+1], &s.board[y][i] };
                Combine_Move(v, {i<s.BoardSize-1,i!=x}, combined[y][i+1]);
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

        if(const u32 n=cifCount(*v[1]); n > s.BiggestCellCifCount) s.BiggestCellCifCount = n;
        s.BiggestCell = s.BiggestCell < *v[1] ? *v[1] : s.BiggestCell;
        s.Score += *v[1];
        s.moved = 1;
    } else if(f.second) {
        *v[2] = *v[0];
        *v[0] = 0;
        s.moved = 1;
    }
}

bool Game::AddCell() {
    std::vector<std::pair<u32,u32>> v;

    for(u32 y=0; y < s.BoardSize; y++)
        for(u32 x=0; x< s.BoardSize; x++)
            if(!s.board[y][x]) v.push_back(std::pair<u32,u32>(x,y));
    
    const u32 sz = v.size();
    std::wcout << L"Size: " << sz << std::endl;
    
    if(!sz) return 0;

    const std::pair<u32,u32> p = v[getRand(0,sz-1)];
    s.board[p.second][p.first] = getRand(0,10) ? 2 : 4;
    s.newPos = p;
    return 1;
}

bool Game::canMove() {
    for(u32 y=0; y < s.BoardSize-1; y++) {
        for(u32 x=0; x < s.BoardSize-1; x++)
            if(s.board[y][x] == s.board[y][x+1] || s.board[y][x] == s.board[y+1][x]) return 1;
    } return 0;
}

u32 Game::getRand(const u32& mn, const u32& mx) {
    return std::uniform_int_distribution<u32>(mn,mx)(rng);
}
