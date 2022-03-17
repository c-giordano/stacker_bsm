#include "../interface/thTools.h"

void normalizeHistograms(std::vector<TH1D*>& histos) {
    for (auto curr : histos) {
        curr->Scale(1/curr->Integral());
    }
}

TH1D* sumVector(std::vector<TH1D*>& histoVec) {
    TH1D* sum = nullptr;
    for (unsigned i = 0; i < histoVec.size(); i++) {
        TH1D* currHist = histoVec[i];
        if (i == 0) {
            sum = new TH1D(*currHist);
        } else {
            sum->Add(currHist);
        }
    }
    return sum;
}