#include "../interface/StringTools.h"

TString cleanTString(TString& toClean) {
    TString cleaned(toClean);

    cleaned.ReplaceAll("gamma", "#gamma");
    cleaned.ReplaceAll("TT", "t#bar{t}");
    cleaned.ReplaceAll("tt", "t#bar{t}");

    /*if (cleaned.Contains("e+")) {

    }*/

    // TODO

    return cleaned;
}

bool stringContains(std::string& str, char toFind) {
    if (str.find(toFind) != std::string::npos) {
        return true;
    } else {
        return false;
    }
}

void cleanWhitespace(std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    if (start == std::string::npos) {
        start = 0;
    }
    str = str.substr(start);
    size_t end = str.find_last_not_of(" \t");
    
    str = str.substr(0, end + 1);
}