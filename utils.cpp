#include "utils.h"

bool useCol = 1;

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
