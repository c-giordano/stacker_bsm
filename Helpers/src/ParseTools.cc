#include "../interface/ParseTools.h"

bool considerLine(std::string* line) {
    if (line->find_first_not_of( " \t" ) == std::string::npos) return false;

    if (line->at(line->find_first_not_of( " \t" )) == '#') return false;

    return true;
}

std::pair<std::string, std::string> splitSettingAndValue(std::string& line) {
    int pos = line.find('=');
    
    return {line.substr(0, pos), line.substr(pos+1, line.size())};
}