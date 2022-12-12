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

        if (hist->HasXAxisNameOverwrite()) {
            stack->GetXaxis()->SetTitle(hist->GetXAxisName().c_str());
        }
    } else {
        stack->GetXaxis()->SetTitle("");
        stack->GetXaxis()->SetLabelSize(0);
    }

    // Set Y axis name
    if (yAxisOverride != "") {
        stack->GetYaxis()->SetTitle(yAxisOverride.c_str());
    } else if (hist->HasRebin()) {
        stack->GetYaxis()->SetTitle("Events / bin");
    } else {
        stack->GetYaxis()->SetTitle(first->GetYaxis()->GetTitle());
    }

    // Set maximum size of histogram to avoid overlap with legend
    if (noStack) {
        stack->SetMaximum(stack->GetMaximum("NOSTACK") * 1.6);
    } else if (hist->isLogscale()) {
        if (dataHist && dataHist->GetMaximum() > stack->GetMaximum()) {
            stack->SetMaximum(dataHist->GetMaximum() * 10); 
        } else {
            stack->SetMaximum(stack->GetMaximum() * 10);
        }
        if (dataHist) {
            double min = 1.e10;
            for (int i=1; i<dataHist->GetNbinsX()+1; i++) {
                if (dataHist->GetBinContent(i) > 0.1 && dataHist->GetBinContent(i) < min && dataHist->GetBinContent(i) > 0.) {
                    min = dataHist->GetBinContent(i) / 10;
                    std::cout << min << std::endl;
                }
            }
            stack->SetMinimum(min);
        }
        stack->SetMinimum(0.8);
        
    } else {
        if (dataHist && dataHist->GetMaximum() > stack->GetMaximum()) {
            stack->SetMaximum(dataHist->GetMaximum() * 1.6); 
        } else {
            stack->SetMaximum(stack->GetMaximum() * 1.6); 
        }
    }

    // Set logscale if desired
    pad->SetLogy(hist->isLogscale());

    // Set custom bin labels if desired
}
