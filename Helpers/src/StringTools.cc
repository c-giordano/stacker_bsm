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
