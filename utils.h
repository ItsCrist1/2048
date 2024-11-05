#ifndef UTILS_H
#define UTILS_H

#include <cstdint>
#include <string>
#include <sstream>

using u8 = uint8_t;
using u32 = uint32_t;
using i32 = int32_t;

extern bool useCol;

const std::wstring ANSI_CLEAR = L"\033[2J\033[H";
const std::wstring ANSI_RESET = L"\033[0m";

struct RGB {
    const u8 r,g,b;

    RGB(const u8& r, const u8& g, const u8& b);
    RGB(const u8& c=0);
};

std::wstring getCol(const RGB& rgb);
char getch();

#endif
