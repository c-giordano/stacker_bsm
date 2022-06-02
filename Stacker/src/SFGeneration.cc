#include "../interface/Stacker.h"
#include <functional>

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

        // split line in process and histogram
        std::string processName;
        std::string histogram;
        std::stringstream stream(line);
        stream >> histogram >> processName;

        TString processNameTString = processName;
        it = std::find_if(histogramVec.begin(), histogramVec.end(), std::bind(Histogram::searchHist, std::placeholders::_1, histogram));
        GenerateSF(*it, processNameTString);
    }
}

void Stacker::GenerateSF(Histogram* histogram, TString& processName) {
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

    TH1D* sf = new TH1D(*dataHistogram);
    // take a deep copy of the process for which we extract a SF
    // histvector should be in the order of the processes
    // remove from histvec, sum histvec, extract from data
    // generate SF with contribution and backgrounds
    size_t index = 0;
    Process* currProc = processes->getHead();
    while (currProc && currProc->getName() != processName) {
        currProc = currProc->getNext();
        index++;
    }
    if (currProc == nullptr) {
        std::cerr << "SF generation failed. Process " << processName << " was not found." << std::endl;
        exit(1);
    }

    TH1D* releventContribution = new TH1D(*histVec[index]);
    std::cout << releventContribution->GetName() << std::endl;
    histVec.erase(histVec.begin()+index);

    TH1D* sum = sumVector(histVec);
    sf->Add(sum, -1.);
    

    sf->SetName("SF_" + histogram->getID());
    sf->SetTitle("SF_" + histogram->getID());

    sf->Divide(releventContribution);

    TFile* sfOutput = new TFile("ScaleFactors/Output/SF_" + histogram->getID() + ".root", "RECREATE");

    sfOutput->cd();

    sf->Write("SF");
    
    sfOutput->Close();
}

