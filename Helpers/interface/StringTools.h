#ifndef STRINGTOOLS_H
#define STRINGTOOLS_H

#include <TString.h>

// Takes TString and makes it prettier (e.g. tt-> t#Bar{t}) for legend
TString cleanTString(TString& toClean);

#endif