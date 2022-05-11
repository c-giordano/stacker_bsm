#include "../interface/Stacker.h"


void Stacker::GenerateSFs(std::string& SFFile) {
    // read input file ->parse to a vector which histograms we need
    // for each histogram, generate it
    std::ifstream infile(SFFile);

    std::string line;
    if (! infile.is_open()) {
        std::cout << "ERROR: SF inputfile " << SFFile << " not found" << std::endl;
        exit(1);
    }

    while (getline(infile, line)) {
        std::vector<Histogram*>::iterator it;
        it = std::find_if(histogramVec.begin(), histogramVec.end(), std::bind(Histogram::searchHist, std::placeholders::_1, line));
        GenerateSF(*it);
    }
}

void Stacker::GenerateSF(Histogram* histogram) {
    // generate stack + data info
    // also plot the inputdistribution and the output somewhere
    // generate outputroot file for each distribution. Renaming will be done manually anyway
    printHistogram(histogram); // improve on how outputdir is specified

    TString histID = histogram->getID();
    TLegend* legend = getLegend();
    THStack* histStack = new THStack(histID, histID);
    std::vector<TH1D*>* signalVector = new std::vector<TH1D*>;

    std::vector<TH1D*> histVec = processes->fillStack(histStack, histogram, legend, outputfile, signalVector, nullptr);

    TH1D* dataHistogram;
    if (getFakeData()) {
        dataHistogram = sumVector(histVec);
        dataHistogram->SetTitle("Data (expected)");
        dataHistogram->SetName("Data (expected)");

        for (int bin = 1; bin < dataHistogram->GetNbinsX() + 1; ++bin) {
            dataHistogram->SetBinError(bin, sqrt(dataHistogram->GetBinContent(bin)));
        }
    } else {
        dataHistogram = dataProcess->getHistogram(histogram);
        dataHistogram->SetTitle("Data");
        dataHistogram->SetName("Data");
    }

    TH1D* sf = sumVector(histVec);
    sf->SetName(("SF_" + histogram->getCleanName()).c_str());
    sf->SetTitle(("SF_" + histogram->getCleanName()).c_str());

    sf->Divide(dataHistogram);

    TFile* sfOutput = new TFile(("ScaleFactors/Output/SF_" + histogram->getCleanName()).c_str(), "RECREATE");
    sfOutput->cd();

    sf->Write("SF");
    
    sfOutput->Close();
}

