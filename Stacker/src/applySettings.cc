#include "../interface/Stacker.h"


void Stacker::stackSettingsPreDraw(THStack* stack, std::vector<TH1D*>& histVec) {
    // settings to be applied before drawing the THStack. 

    // Normalization of histograms
    if (noStack) normalizeHistograms(histVec);
}

void Stacker::stackSettingsPostDraw(TPad* pad, THStack* stack, Histogram* hist, TH1D* first) {
    // Settings to be applied after drawing the THStack

    // Set X axis name
    stack->GetXaxis()->SetTitle(first->GetXaxis()->GetTitle());

    // Set Y axis name
    if (yAxisOverride != "") {
        stack->GetYaxis()->SetTitle(yAxisOverride.c_str());
    } else {
        stack->GetYaxis()->SetTitle(first->GetYaxis()->GetTitle());
    }

    // Set maximum size of histogram to avoid overlap with legend
    if (noStack) {
        stack->SetMaximum(stack->GetMaximum("NOSTACK") * 1.4);
    } else {
        stack->SetMaximum(stack->GetMaximum() * 1.4); 
    }

    // Set logscale if desired
    pad->SetLogy(hist->isLogscale());

    // Set custom bin labels if desired
    if (hist->getXBinLabels()) {
        std::vector<std::string>* bins = hist->getXBinLabels();
        for (unsigned i = 1; i != bins->size() + 1; i++) {
            stack->GetXaxis()->SetBinLabel(i, TString(bins->at(i - 1)));
        }
    }
}
