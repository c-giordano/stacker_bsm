#include "../interface/Histogram.h"

Histogram::Histogram(TString histID) : id(histID) {
    logScale = false;
    std::string histIDstr = histID.Data();
    channel = getChannel(histIDstr);
}

Histogram::Histogram(TString histID, bool reqLog) : id(histID), logScale(reqLog) {
    // Do stuff
    std::string histIDstr = histID.Data();
    channel = getChannel(histIDstr);
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

        readSetting(setting, currSetAndVal);
    }
}

void Histogram::readSetting(std::string& setting, std::pair<std::string, std::string>& currSetAndVal) {
    if (setting == "logscale") {
        logScale = true;
        return;
    }

    if (setting == "Xbinlabels") {
        std::istringstream stream(currSetAndVal.second);
        std::string bin;
        xBinLabels = new std::vector<std::string>;

        while (getline(stream, bin, ',')) {
            xBinLabels->push_back(bin);
        }
        return;
    }

    if (setting == "rebin") {
        RebinFixed = std::stoi(currSetAndVal.second);
    }

    if (setting == "rebinVar") {
        std::istringstream stream(currSetAndVal.second);
        std::string bin;
        xBinLabels = new std::vector<std::string>;
        getline(stream, bin, ',');

        RebinFixed = std::stoi(bin);
        RebinVar = new double[RebinFixed];
        int currEntry = 0;
        while (getline(stream, bin, ',')) {
            RebinVar[currEntry] = std::stod(bin);
            currEntry++;
        }
        return;
    }

    if (setting == "xAxisName") {
        xAxisName = currSetAndVal.second;
    }

    if (setting == "UnifyBinWidth") {
        uniWidthBins = true;
    }
}
