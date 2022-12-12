#include "../interface/thTools.h"

void normalizeHistograms(std::vector<TH1D*>& histos) {
    for (auto curr : histos) {
        curr->Scale(1/curr->Integral());
    }
}

TH1D* sumVector(std::vector<TH1D*>& histoVec) {
    TH1D* sum = nullptr;
    TH1D* errors = nullptr;
    for (unsigned i = 0; i < histoVec.size(); i++) {
        TH1D* currHist = histoVec[i];
        if (i == 0) {
            sum = new TH1D(*currHist);
            errors = new TH1D(*currHist);
            
            for (int i=1; i<errors->GetNbinsX()+1; i++) {
                errors->SetBinContent(i, sum->GetBinError(i) * sum->GetBinError(i));
            }
        } else {
            sum->Add(currHist);
            for (int i=1; i<errors->GetNbinsX()+1; i++) {
                errors->SetBinContent(i, errors->GetBinContent(i) + currHist->GetBinError(i) * currHist->GetBinError(i));
            }
        }
    }

    for (int i=1; i<errors->GetNbinsX()+1; i++) {
        sum->SetBinError(i, sqrt(errors->GetBinContent(i)));
    }
    return sum;
}