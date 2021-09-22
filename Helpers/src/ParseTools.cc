#include "../interface/ParseTools.h"

bool considerLine(std::string* line) {
    return (line->at(line->find_first_not_of( " \t" )) == '#' || line->find_first_not_of( " \t" ) == std::string::npos);
}