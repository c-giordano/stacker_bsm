#ifndef STRINGTOOLS_H
#define STRINGTOOLS_H

#include <TString.h>

// Takes TString and makes it prettier (e.g. tt-> t#Bar{t}) for legend
TString cleanTString(TString& toClean);
bool stringContains(std::string& str, char toFind);
void cleanWhitespace(std::string& str);
std::string removeExt(std::string& str);
std::string getChannel(std::string& str);
std::string getFilename(std::string& str);
std::string splitAtUnderscore(std::string& str);

#endif