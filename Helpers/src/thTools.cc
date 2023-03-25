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


TH1D* rebin(TH1D* input, int nbins, double binLow, double binHigh) {
    TString nameOld = input->GetName();
    input->SetName(nameOld+"OLD");
    TH1D* output = new TH1D(nameOld, input->GetTitle(), nbins, binLow, binHigh);

    double binContent = 0.;
    double binError = 0.;
    
    int j = 1;
    while (input->GetBinLowEdge(j) + input->GetBinWidth(j) <= binLow+output->GetBinWidth(1)) {
        binContent += input->GetBinContent(j);
        binError += input->GetBinError(j) * input->GetBinError(j);
        j++;
    }
    output->SetBinContent(1, binContent);
    output->SetBinError(1, sqrt(binContent));

    for (int i = 2; i < output->GetNbinsX(); i++) {
        output->SetBinContent(i, input->GetBinContent(j));
        output->SetBinError(i, input->GetBinError(j));
        j++;
    }

    binContent = 0.;
    binError = 0.;

    while (j < input->GetNbinsX()+1) {
        binContent += input->GetBinContent(j);
        binError += input->GetBinError(j) * input->GetBinError(j);
        j++;
    }

    output->SetBinContent(nbins, binContent);
    output->SetBinError(nbins, sqrt(binContent));

    return output;
}