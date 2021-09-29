#include "../interface/Histogram.h"

Histogram::Histogram(TString histID) : id(histID) {
    logScale = false;
}

Histogram::Histogram(TString histID, bool reqLog) : id(histID), logScale(reqLog) {
    // Do stuff
}

bool Histogram::searchHist(Histogram* hist, std::string& idToFind) {
    TString id = hist->getID();
    return id.EqualTo(idToFind);
}

void Histogram::readSettings(std::istringstream& settingLine) {
    std::string setting;

    for (; settingLine >> setting;) {
        std::pair<std::string, std::string> currSetAndVal;
        if (stringContains(setting, '=')) {
            currSetAndVal = splitSettingAndValue(setting);
            setting = currSetAndVal.first;
        }
        
        if (setting == "logscale") {
            logScale = true;
            continue;
        } //else if ()

        if (setting == "binlabels") {
            std::istringstream stream(currSetAndVal.second);
            std::string bin;
            binLabels = new std::vector<std::string>;

            while (getline(stream, bin, ',')) {
                binLabels->push_back(bin);
            }
        }
    }
}
