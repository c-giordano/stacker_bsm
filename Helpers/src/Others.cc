#include "../interface/Others.h"

bool copyFileCustom(const char *SRC, const char* DEST) {
    std::ifstream src(SRC, std::ios::binary);
    std::ofstream dest(DEST, std::ios::binary);
    dest << src.rdbuf();
    return src && dest;
}