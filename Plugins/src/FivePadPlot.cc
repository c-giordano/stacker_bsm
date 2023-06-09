#include "../interface/FivePadPlot.h"


void FivePadPlot::FivePadPlot(Stacker& stacker) {
    std::vector<Histogram*> histsToPlot;
    for (auto& histogram : stacker.GetHistograms()) {
        histogram->setPrintToFile(false);
        //std::cout << std::string(histogramID->getID().Data()) << std::endl;
        //if (std::string(histogramID->getID().Data()) != "BDT_FinalresultSignal_TriClass_SR-2Lee" && std::string(histogramID->getID().Data()) != "BDT_FinalresultSignal_TriClass_SR-3L") continue;
        //if (! stringContainsSubstr(std::string(histogramID->getID().Data()), "CR-3L-Z")) continue;
        if (std::string(histogram->getID().Data()) == "BDT_Finalresult_tanh_Signal_TriClass_SR-2Lee" ||
            std::string(histogram->getID().Data()) == "BDT_Finalresult_tanh_Signal_TriClass_SR-2Lem" ||
            std::string(histogram->getID().Data()) == "BDT_Finalresult_tanh_Signal_TriClass_SR-2Lmm" ||
            std::string(histogram->getID().Data()) == "BDT_Finalresult_tanh_Signal_TriClass_SR-3L"  ||
            std::string(histogram->getID().Data()) == "BDT_FinalresultSignal_TriClass_SR-4L") {
            
            histsToPlot.push_back(histogram);
        }
    }
    FivePadPlot::BuildMainPlot(stacker, histsToPlot);
}

// main plot function -> collect components and build plot
void FivePadPlot::BuildMainPlot(Stacker& stacker, std::vector<Histogram*>& histograms) {
    // canvas
    // five pads, 2 pads per pad

    TCanvas* canv = new TCanvas("FivePad_canvas", "FivePad_canvas", 3000, 941);
    canv->Draw();
    canv->cd();

    std::vector<TPad*> pads;

    for (int i = 0; i < 5; i++) {
        canv->cd();
        std::string padname = std::string(histograms[i]->getID().Data()) + "_MainPad";
        std::cout << "pad " << i << " positions " << i*0.2 << "  " << (i+1)*0.2 << std::endl;
        TPad* pad = new TPad(padname.c_str(), padname.c_str(), i*0.2, 0., (i+1)*0.2, 1.);
        pads.push_back(pad);

        pad->Draw();
        BuildSubPlot(stacker, histograms[i], pad, i);
    }

    TString histID = histograms[0]->getID();

    std::string fullPath = stacker.GetPathToOutput();
    if (stacker.GetUseT2B()) {
        std::string id = histID.Data();
        fullPath += getChannel(id);
        fullPath += "/";
    }

    canv->Print(fullPath + histID + ".png");
    canv->Print(fullPath + histID + ".pdf");
}

void FivePadPlot::BuildSubPlot(Stacker& stacker, Histogram* histogram, TPad* plotPad, int plotNb) {
    plotPad->cd();
    
    TString histID = histogram->getID();
    THStack* histStack = new THStack(histID, histID);
    
    TLegend* legend = stacker.getLegend();
    std::vector<TH1D*>* signalVector = new std::vector<TH1D*>;
    TH1D** sysUnc = new TH1D*();
    *sysUnc = nullptr; 
    std::vector<TH1D*> histVec = stacker.GetProcesses()->fillStack(histStack, histogram, legend, stacker.GetOutputFile(), signalVector, sysUnc);
    
    TH1D* dataHistogram = nullptr;
    if (stacker.getData()) {
        // get data histogram. Either from specialised process(list) or fake it
        // pass to stackfiller and to ratiodrawer
        if (stacker.getFakeData()) {
            dataHistogram = sumVector(histVec);
            dataHistogram->SetTitle("Data (expected)");
            dataHistogram->SetName("Data (expected)");

            for (int bin = 1; bin < dataHistogram->GetNbinsX() + 1; ++bin) {
                dataHistogram->SetBinError(bin, sqrt(dataHistogram->GetBinContent(bin)));
            }
        } else {
            dataHistogram = stacker.GetDataProcess()->getHistogram(histogram);
            //std::cout << "data: " << dataHistogram->Integral() << " events" << std::endl;

            if (stacker.GetDataProcess()->getName() != "Obs") {
                dataHistogram->SetTitle(stacker.GetDataProcess()->getName());
            } else {
                dataHistogram->SetTitle("Data");
            }
            dataHistogram->SetName("Data");
        }

        legend->AddEntry(dataHistogram, dataHistogram->GetName());
        // change dataHistogram settings
    }

    TPad** mainPad = new TPad*();
    TH1D* totalUnc = stacker.drawStack(histogram, histStack, histVec, sysUnc, dataHistogram, mainPad);

    if (plotNb > 0) {
        histStack->GetYaxis()->SetTitle("");
    }
    stacker.drawSignalYield(legend, *signalVector);
    legend->Draw();
    
    plotPad->cd();

    TPad** smallPad = new TPad*(); 
    TH1D* ratioPlot = nullptr;
    if (stacker.getData()) {
        if (! totalUnc) {
            TH1D* allHistograms = sumVector(histVec);
            ratioPlot = stacker.drawRatioData(histogram, allHistograms, dataHistogram, smallPad);

        } else {
            ratioPlot = stacker.drawRatioData(histogram, totalUnc, dataHistogram, smallPad);
        }
    } else {
        ratioPlot = stacker.drawRatioMC(histogram, histVec, *signalVector, smallPad);
    }

    if (plotNb > 0 && ratioPlot) {
        ratioPlot->SetYTitle("");
        (*smallPad)->Update();
        (*smallPad)->Modified();
    }

    // auto resize axis
    
    TH1* combiHist = (TH1*) histStack->GetStack()->Last();
    double xmin = combiHist->GetBinLowEdge(1);
    double xmax = combiHist->GetBinLowEdge(combiHist->GetNbinsX()) + combiHist->GetBinWidth(combiHist->GetNbinsX());
    bool change = false;
    
    double MinContent = 0.0002; // too small. Fix later because this does not take ratioplots into account
    int counter = 1;
    double currentBinContent = combiHist->GetBinContent(counter);
    if (dataHistogram) currentBinContent += dataHistogram->GetBinContent(counter);

    while (currentBinContent <= MinContent && counter <= combiHist->GetNbinsX()) {
        counter++;
        xmin = combiHist->GetBinLowEdge(counter);
        change = true;

        currentBinContent = combiHist->GetBinContent(counter);
        if (dataHistogram) currentBinContent += dataHistogram->GetBinContent(counter);
    }

    double MaxContent = 0.0002;
    counter = combiHist->GetNbinsX();

    currentBinContent = combiHist->GetBinContent(counter);
    if (dataHistogram) currentBinContent += dataHistogram->GetBinContent(counter);

    while (currentBinContent <= MaxContent && counter >= 1) {
        counter--;
        xmax = combiHist->GetBinLowEdge(counter) + combiHist->GetBinWidth(counter);
        change = true; 

        currentBinContent = combiHist->GetBinContent(counter);
        if (dataHistogram) currentBinContent += dataHistogram->GetBinContent(counter);
    }

    if (change) {
        // temp disabled -> dont really like the effect it has on some plots. Definitely a want for later versions but not now.
        histStack->GetXaxis()->SetRangeUser(xmin, xmax);
        if (ratioPlot) ratioPlot->GetXaxis()->SetRangeUser(xmin, xmax);
    }

    (*mainPad)->Update();
    (*mainPad)->Modified();

    if (*smallPad) {
        (*smallPad)->Update();
        (*smallPad)->Modified();
    }
}