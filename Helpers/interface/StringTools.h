#ifndef STRINGTOOLS_H
#define STRINGTOOLS_H

#include <TString.h>

// Takes TString and makes it prettier (e.g. tt-> t#Bar{t}) for legend
TString cleanTString(TString& toClean);
bool stringContains(std::string& str, char toFind);
bool stringContainsSubstr( const std::string& s, const std::string& substring );
void cleanWhitespace(std::string& str);
std::string removeExt(std::string& str);
std::string getChannel(std::string& str);
std::string getFilename(std::string& str);
std::string splitAtUnderscore(std::string& str);

std::string getYearFromRootFile(std::string& rootfile);

std::string replace( const std::string& s, const std::string& oldstring, const std::string& newstring );
std::string removeOccurencesOf( const std::string& s, const std::string& substring );
std::vector< std::string > split( const std::string& s, const std::string& substring );

#endif