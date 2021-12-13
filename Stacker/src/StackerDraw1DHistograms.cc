#include "../interface/Stacker.h"

void Stacker::printAllHistograms() {
    int tempCount =0 ;
    for (auto histogramID : histogramVec) {
        tempCount++;
        //if (tempCount == 50) break;

        if (! onlyDC || (onlyDC && histogramID->getPrintToFile())) {
            printHistogram(histogramID);
        }
    }
}

void Stacker::printHistogram(Histogram* hist) {
    TString histID = hist->getID();

    THStack* histStack = new THStack(histID, histID);
    TLegend* legend = getLegend();
    std::vector<TH1D*>* signalVector = new std::vector<TH1D*>;
    TH1D** sysUnc = new TH1D*();
    *sysUnc = nullptr; 
    std::vector<TH1D*> histVec = processes->fillStack(histStack, hist, legend, outputfile, signalVector, sysUnc);
    
    if (onlyDC) return;

    TCanvas* canv = getCanvas(histID);
    canv->Draw();
    canv->cd();

    drawStack(hist, histStack, histVec, sysUnc);

    drawSignalYield(legend, *signalVector);
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


void Stacker::drawStack(Histogram* hist, THStack* histStack, std::vector<TH1D*>& histVec, TH1D** sysUnc) {
    stackSettingsPreDraw(histStack, histVec);
    TString histID = hist->getID();
    TPad* pad = getPad(histID, 0);

    pad->Draw();
    pad->cd();

    stackSettingsPreDraw(histStack, histVec);

    histStack->Draw(drawOpt.c_str());

    stackSettingsPostDraw(pad, histStack, hist, histVec[0]);

    TH1D* allHistograms = sumVector(histVec);
    TH1D* totalUnc = nullptr;
    if (*sysUnc) {
        totalUnc = new TH1D(*allHistograms);
        
        for(int bin = 1; bin < totalUnc->GetNbinsX() + 1; ++bin){
            double statError = allHistograms->GetBinError(bin);
            double systError = (*sysUnc)->GetBinContent(bin); // is already squared
            //totalUnc->SetBinError(bin, sqrt( statError*statError + systError) );
            totalUnc->SetBinError(bin, sqrt(systError) );

        }

        totalUnc->SetFillStyle(3244); //3005  3244
        totalUnc->SetFillColor(kGray+2);
        totalUnc->SetMarkerStyle(0); //1
        totalUnc->Draw("E2 SAME");
    }

    pad->Update();
    pad->Modified();

    TLatex* info = getDatasetInfo(pad);

    delete info;
}

void Stacker::drawSignalYield(TLegend* legend, std::vector<TH1D*>& signalVec) {
    if (! isSignalLine) return;

    TH1D* signalTotal = sumVector(signalVec);
    signalTotal->SetTitle("Signal yield");

    legend->AddEntry(signalTotal, "Signal yield");
    
    signalTotal->SetFillColor(0);
    signalTotal->SetLineColor(1);
    signalTotal->SetLineWidth(3);
    signalTotal->SetMarkerStyle(0);
    signalTotal->Draw("SAME HIST");
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
