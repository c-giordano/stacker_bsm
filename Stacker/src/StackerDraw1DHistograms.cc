#include "../interface/Stacker.h"

void Stacker::printAllHistograms() {
    int tempCount =0 ;
    
    if (onlyDC) return;
    for (auto histogramID : histogramVec) {
        tempCount++;
        histogramID->setPrintToFile(false);
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

    TH1D* dataHistogram = nullptr;
    if (getData()) {
        // get data histogram. Either from specialised process(list) or fake it
        // pass to stackfiller and to ratiodrawer
        if (getFakeData()) {
            dataHistogram = sumVector(histVec);
            dataHistogram->SetTitle("Data (expected)");
            dataHistogram->SetName("Data (expected)");

            for (int bin = 1; bin < dataHistogram->GetNbinsX() + 1; ++bin) {
                dataHistogram->SetBinError(bin, sqrt(dataHistogram->GetBinContent(bin)));
            }
        } else {
            dataHistogram = dataProcess->getHistogram(histID);
            dataHistogram->SetTitle("Data");
            dataHistogram->SetName("Data");
        }

        legend->AddEntry(dataHistogram, dataHistogram->GetName());
        // change dataHistogram settings
    }

    TH1D* totalUnc = drawStack(hist, histStack, histVec, sysUnc, dataHistogram);

    drawSignalYield(legend, *signalVector);
    legend->Draw();
    
    canv->cd();

    if (getData()) {
        drawRatioData(hist, totalUnc, dataHistogram);
    } else {
        drawRatioMC(hist, histVec, *signalVector);
    }

    std::string fullPath = pathToOutput;
    if (runT2B) {
        std::string id = histID.Data();
        fullPath += getChannel(id);
        fullPath += "/";
    }

    canv->Print(fullPath + histID + ".png");
    canv->Print(fullPath + histID + ".pdf");

}


TH1D* Stacker::drawStack(Histogram* hist, THStack* histStack, std::vector<TH1D*>& histVec, TH1D** sysUnc, TH1D* data) {
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
        
        //std::cout << "printing uncertainties:\t";
        for(int bin = 1; bin < totalUnc->GetNbinsX() + 1; ++bin){
            double statError = allHistograms->GetBinError(bin);
            double systError = (*sysUnc)->GetBinContent(bin); // is already squared
            //totalUnc->SetBinError(bin, sqrt( statError*statError + systError) );
            totalUnc->SetBinError(bin, sqrt(systError) );

            //std::cout << totalUnc->GetBinError(bin) << "\t";

        }
        //std::cout << std::endl;
        totalUnc->SetFillStyle(3244); //3005  3244
        totalUnc->SetFillColor(kGray+2);
        totalUnc->SetMarkerStyle(0); //1
        totalUnc->Draw("E2 SAME");
    }

    if (data != nullptr) {
        data->SetMarkerColor(kBlack);
        data->SetFillColor(0);
        data->SetLineColor(kBlack);
        data->SetLineWidth(2);
        data->SetMarkerSize(1.1);


        data->Draw("E1 SAME");
    }

    pad->Update();
    pad->Modified();

    TLatex* info = getDatasetInfo(pad);

    delete info;

    return totalUnc;
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

void Stacker::drawRatioData(Histogram* hist, TH1D* uncHist, TH1D* data) {
    if (! isRatioPlot) return;
    
    TString histID = hist->getID() + "_ratio";
    TPad* smallPad = getPad(histID, 1);

    smallPad->Draw();
    smallPad->cd();

    TH1D* dataTotal = new TH1D(*data);
    TH1D* mcTotal = new TH1D(*uncHist);
    
    dataTotal->Divide(uncHist);

    dataTotal->SetTitleSize(0.192, "X");
    dataTotal->SetTitleSize(0.17, "Y");

    dataTotal->SetTitle("");
    dataTotal->SetYTitle("Data/pred.");

    //signalTotal->SetMaximum(1.1);
    dataTotal->SetFillColor(0);
    dataTotal->SetLineWidth(2);
    dataTotal->SetLineColor(1);
    dataTotal->SetLabelSize(0.16, "XY");
    dataTotal->SetTitleOffset(0.38, "Y");
    dataTotal->SetTickLength(0.096, "X");
    dataTotal->SetNdivisions(504, "Y");

    dataTotal->Draw("E1");

    //signalTotal->GetXaxis()->SetTitle(first->GetXaxis()->GetTitle());

    if (hist->getXBinLabels()) {
        std::vector<std::string>* bins = hist->getXBinLabels();
        for (unsigned i = 1; i != bins->size() + 1; i++) {
            dataTotal->GetXaxis()->SetBinLabel(i, TString(bins->at(i - 1)));
        }
    }


    int nrBins = mcTotal->GetNbinsX();
    for (int i = 1; i < nrBins + 1; i++) {
        if (mcTotal->GetBinContent(i) <= 0.0001) {
            mcTotal->SetBinError(i, 0.00001);
        } else {
            mcTotal->SetBinError(i, mcTotal->GetBinError(i) / mcTotal->GetBinContent(i));
        }
        mcTotal->SetBinContent(i, 1.);
    }
    mcTotal->Draw("SAME E2");

    TLine* line = new TLine(dataTotal->GetBinLowEdge(1), 1., dataTotal->GetXaxis()->GetBinUpEdge(dataTotal->GetNbinsX()), 1.);
    line->Draw("SAME");

    smallPad->Update();
    smallPad->Modified();
    //signalTotal->UseCurrentStyle();
}
