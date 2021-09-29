#include "../interface/Stacker.h"


void Stacker::stackSettingsPreDraw(THStack* stack, std::vector<TH1D*>& histVec) {
    if (noStack) normalizeHistograms(histVec);
}

void Stacker::stackSettingsPostDraw(TPad* pad, THStack* stack, Histogram* hist, TH1D* first) {
    stack->GetXaxis()->SetTitle(first->GetXaxis()->GetTitle());

    if (yAxisOverride != "") {
        stack->GetYaxis()->SetTitle(yAxisOverride.c_str());
    } else {
        stack->GetYaxis()->SetTitle(first->GetYaxis()->GetTitle());
    }

    if (noStack) {
        stack->SetMaximum(stack->GetMaximum("NOSTACK") * 1.4);
    } else {
        stack->SetMaximum(stack->GetMaximum() * 1.4); 
    }

    pad->SetLogy(hist->isLogscale());

    if (hist->getBinLabels()) {
        std::vector<std::string>* bins = hist->getBinLabels();
        for (int i = 1; i != bins->size() + 1; i++) {
            stack->GetXaxis()->SetBinLabel(i, TString(bins->at(i - 1)));
        }
    }
}
