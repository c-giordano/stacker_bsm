#ifndef PARSETOOLS_H
#define PARSETOOLS_H

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

bool considerLine(std::string* line);
std::pair<std::string, std::string> splitSettingAndValue(std::string& line);


#endif