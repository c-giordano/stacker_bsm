#include "../interface/Stacker.h"
#include "../../Styles/Styles.h"
// take histogram, get vector of uncertainties, for each entry draw line, or at least for largest ones

/* 
draw canvas, call uncertainty histograms

*/

void Stacker::drawAllUncertaintyImpacts() {
    setUncertaintyImpactStyle();
    gROOT->ForceStyle();
    std::vector<std::string> uncToDraw = {"JEC", "qcdScale", "pdfShapeVar", "bTagShape_hf", "bTagShape_lf", "JER_1p93", "JER_2p5"};
    for (auto histogramID : histogramVec) {
        histogramID->setPrintToFile(false);
        drawUncertaintyImpacts(histogramID, uncToDraw);
    }
}

void Stacker::drawUncertaintyImpacts(Histogram* hist, std::vector<std::string>& uncToDraw) {
    TString name = hist->getID() + "_unc";
    TCanvas* canvas = new TCanvas(name);
    TPad* pad = new TPad(name, name, 0., 0., 1., 1.);

    gStyle->SetPalette(kRainBow);
    auto cols = TColor::GetPalette();

    canvas->Draw();
    canvas->cd();
    pad->Draw();
    pad->cd();

    // generate vector now with uncertainties in the correct way that i need
    // idk what to do now
    std::vector<TH1D*> nominalHists = processes->CreateHistogramAllProcesses(hist);
    std::map<std::string, std::pair<TH1D*, TH1D*>> variations = processes->UpAndDownHistograms(hist);

    TH1D* nominal = sumVector(nominalHists);

    TLegend* legend = new TLegend(0.2, 0.8, 0.93, 0.92);
    legend->SetNColumns(3);

    double max=0.;
    double min=0.5;    

    int colIndex=0;
    int step = (cols.GetSize() - 1) / (uncToDraw.size() - 1);
    for (auto it : uncToDraw) {
        std::pair<TH1D*, TH1D*> upAndDown = variations[it];
        
        // uncertainty / nominal
        TH1D* upVar = upAndDown.first;
        TH1D* downVar = upAndDown.second;

        upVar->Divide(nominal);
        downVar->Divide(nominal);

        if (std::max(upVar->GetMaximum(), downVar->GetMaximum()) > max) {
            max = std::max(upVar->GetMaximum(), downVar->GetMaximum());
        }
        // set style
        upVar->UseCurrentStyle();
        downVar->UseCurrentStyle();
        // no fill color
        upVar->SetLineColor(cols.At(colIndex));
        downVar->SetLineColor(cols.At(colIndex));
        downVar->SetLineStyle(2);

        // add to legend
        legend->AddEntry(upVar, it.c_str());

        // draw in optimal way
        // use SAME option
        upVar->Draw("HIST SAME");
        downVar->Draw("HIST SAME");

        colIndex += step;
    }
    auto it = variations.begin();
    it->second.first->SetMaximum(max * 1.3);
    it->second.first->SetMinimum(min); 

    TLine* line = new TLine(nominal->GetBinLowEdge(1), 1., nominal->GetXaxis()->GetBinUpEdge(nominal->GetNbinsX()), 1.);
    line->Draw("SAME");

    legend->Draw();

    pad->Update();
    pad->Modified();

    std::string fullPath = pathToOutput;
    if (runT2B) {
        std::string id = hist->getID().Data();
        fullPath += getChannel(id);
        fullPath += "/";
    }

    canvas->Print(fullPath + hist->getID() + ".png");
    canvas->Print(fullPath + hist->getID() + ".pdf");

}