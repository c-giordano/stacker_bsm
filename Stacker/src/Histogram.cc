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