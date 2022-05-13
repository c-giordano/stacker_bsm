#include "../interface/Stacker.h"


void Stacker::stackSettingsPreDraw(THStack* stack, std::vector<TH1D*>& histVec) {
    // settings to be applied before drawing the THStack. 

    // Normalization of histograms
    if (noStack) normalizeHistograms(histVec);
}

void Stacker::stackSettingsPostDraw(TPad* pad, THStack* stack, Histogram* hist, TH1* first, TH1* dataHist) {
    // Settings to be applied after drawing the THStack

    // Set X axis name
    if (!isRatioPlot){
        stack->GetXaxis()->SetTitle(first->GetXaxis()->GetTitle());

        if (hist->getXBinLabels()) {
            std::vector<std::string>* bins = hist->getXBinLabels();

            //stack->GetXaxis()->SetLabelSize(0.06);

            for (unsigned i = 1; i != bins->size() + 1; i++) {
                stack->GetXaxis()->SetBinLabel(i, TString(bins->at(i - 1)));
            }
        }
    } else {
        stack->GetXaxis()->SetTitle("");
        stack->GetXaxis()->SetLabelSize(0);
    }

    // Set Y axis name
    if (yAxisOverride != "") {
        stack->GetYaxis()->SetTitle(yAxisOverride.c_str());
    } else {
        stack->GetYaxis()->SetTitle(first->GetYaxis()->GetTitle());
    }

    // Set maximum size of histogram to avoid overlap with legend
    if (noStack) {
        stack->SetMaximum(stack->GetMaximum("NOSTACK") * 1.5);
    } else if (hist->isLogscale()) {
        if (dataHist && dataHist->GetMaximum() > stack->GetMaximum()) {
            stack->SetMaximum(dataHist->GetMaximum() * 100); 
        } else {
            stack->SetMaximum(stack->GetMaximum() * 100); 
        }
    } else {
        if (dataHist && dataHist->GetMaximum() > stack->GetMaximum()) {
            stack->SetMaximum(dataHist->GetMaximum() * 1.5); 
        } else {
            stack->SetMaximum(stack->GetMaximum() * 1.5); 
        }
    }

    // Set logscale if desired
    pad->SetLogy(hist->isLogscale());

    // Set custom bin labels if desired
}
