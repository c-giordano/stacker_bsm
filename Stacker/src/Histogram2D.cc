#include "../interface/Histogram2D.h"

Histogram2D::Histogram2D(TString histID) : Histogram(histID) {

}

bool Histogram2D::search2DHist(Histogram2D* hist, std::string& idToFind) {
    TString id = hist->getID();
    return id.EqualTo(idToFind);
}

void Histogram2D::readSetting(std::string& setting, std::pair<std::string, std::string>& currSetAndVal) {
    Histogram::readSetting(setting, currSetAndVal);

    if (setting == "Ybinlabels") {
        std::istringstream stream(currSetAndVal.second);
        std::string bin;
        yBinLabels = new std::vector<std::string>;

        while (getline(stream, bin, ',')) {
            yBinLabels->push_back(bin);
        }
    }
}
