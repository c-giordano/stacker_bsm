#include "../interface/Stacker.h"

void Stacker::printHistogram(Histogram* hist) {
    TString histID = hist->getID();

    THStack* histStack = new THStack(histID, histID);
    TLegend* legend = getLegend();
    std::vector<TH1D*>* signalVector = new std::vector<TH1D*>;
    std::vector<TH1D*> histVec = processes->fillStack(histStack, histID, legend, outputfile, signalVector);

    TCanvas* canv = getCanvas(histID);
    canv->Draw();
    canv->cd();

    drawStack(hist, histStack, histVec);
    legend->Draw();
    
    canv->cd();
    drawRatioMC(hist, histVec, *signalVector);

    std::string fullPath = pathToOutput;
    if (runT2B) {
        std::string id = histID.Data();
        fullPath += getChannel(id);
        fullPath += "/";
    }

    canv->Print(fullPath + histID + ".png");
}


void Stacker::drawStack(Histogram* hist, THStack* histStack, std::vector<TH1D*>& histVec) {
    stackSettingsPreDraw(histStack, histVec);
    TString histID = hist->getID();
    TPad* pad = getPad(histID, 0);

    pad->Draw();
    pad->cd();

    histStack->Draw(drawOpt.c_str());

    stackSettingsPostDraw(pad, histStack, hist, histVec[0]);

    pad->Update();
    pad->Modified();

    TLatex* info = getDatasetInfo(pad);

    delete info;
}

void Stacker::drawRatioMC(Histogram* hist, std::vector<TH1D*>& histoVec, std::vector<TH1D*>& signalVec) {
    /* 
    TODO:
        Make a function deciding if data or not, let it decide after what to do... ofzo
    */
    if (! isRatioPlot) return;

    TString histID = hist->getID() + "_ratio";
    TPad* smallPad = getPad(histID, 1);

    smallPad->Draw();
    smallPad->cd();

    TH1D* signalTotal = sumVector(signalVec);
    TH1D* allHistograms = sumVector(histoVec);
    
    signalTotal->Divide(allHistograms);


    signalTotal->SetTitleSize(0.192, "X");
    signalTotal->SetTitleSize(0.17, "Y");

    signalTotal->SetTitle("");
    signalTotal->SetYTitle("Signal fraction");

    //signalTotal->SetMaximum(1.1);
    signalTotal->SetFillColor(0);
    signalTotal->SetLineWidth(2);
    signalTotal->SetLineColor(1);
    signalTotal->SetLabelSize(0.16, "XY");
    signalTotal->SetTitleOffset(0.38, "Y");
    signalTotal->SetTickLength(0.096, "X");
    signalTotal->SetNdivisions(504, "Y");

    signalTotal->Draw("HIST");

    //signalTotal->GetXaxis()->SetTitle(first->GetXaxis()->GetTitle());

    if (hist->getXBinLabels()) {
        std::vector<std::string>* bins = hist->getXBinLabels();
        for (unsigned i = 1; i != bins->size() + 1; i++) {
            signalTotal->GetXaxis()->SetBinLabel(i, TString(bins->at(i - 1)));
        }
    }

    smallPad->Update();
    smallPad->Modified();
    //signalTotal->UseCurrentStyle();
}
