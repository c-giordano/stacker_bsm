#include "../interface/Stacker.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

Stacker::Stacker(const char* rootFilename, std::string& settingFile) {
    // Constructer should either call parser or take output of parser
    // Constructer should build processlist and set its own settings

    // Processlist should be built based on:
    // - Directory's in root file: set of directories for all categories,
    //   under these a set of directories related to different processes/years 
    //   with all correctly normalized histograms belonging to this category
    // - Some component telling which directory in the file belongs with which process
    // - Order + colors defined in settings -> from this it sounds best to use parser inside of constructor
    //   OR build processlist in parser
    // 

    TH1::AddDirectory(false);
    setTDRStyle();

    inputfile = new TFile(rootFilename, "read");
    processes = new ProcessList();

    std::ifstream infile(settingFile);
    std::string line;

    if (! infile.is_open()) {
        std::cout << "ERROR: Settingsfile not found" << std::endl;
        exit(1);
    }

    while (getline(infile, line)) {
        // IF CONTAINS # or only whitespace: ignore line
        if(! considerLine(&line)){
            continue;
        }
        // TODO: check if some sort of switch command is present. If so, go to other loop for other reading
        
        if (line.find("BROAD SETTINGS") != std::string::npos || line.find("HISTOGRAMS") != std::string::npos) {
            break;
        }
        // first, a processname is read, from this, a processelement is built and for this processname a set of directories is achieved
        std::istringstream stream(line);
        
        std::string processName;
        std::string colorString;
        stream >> processName >> colorString;
        
        // Color_t currentColor = std::stoi(colorString);
        TString processNameAlt(processName);
        processes->addProcess(processNameAlt, std::stoi(colorString), inputfile);
    }

    while (getline(infile, line)) {
        if (!considerLine(&line)) {
            continue;
        }

        if (line.find("HISTOGRAMS") != std::string::npos) {
            break;
        }

        // Set gen settings
    }

    histogramVec = new std::vector<TString>;
    while (getline(infile, line)) {
        if (!considerLine(&line)) {
            continue;
        }
        
        // Set histograms to check
    }
    
    if (histogramVec->empty()) {
        // walk in inputfile to first process, check all histogram names
        inputfile->cd(processes->getHead()->getName());
        gDirectory->cd(gDirectory->GetListOfKeys()->At(0)->GetName());

        TList* histogramsAvailable = gDirectory->GetListOfKeys();

        for (auto const&& obj : *histogramsAvailable) {
            histogramVec->push_back(TString(obj->GetName()));
        }
    }

    //outputfile = new TFile("Combinefile.root", "recreate");
}

Stacker::~Stacker() {
    inputfile->Close();
    outputfile->Close();

    delete inputfile;
    delete outputfile;
    delete processes;
}

void Stacker::printAllHistograms() {
    for (auto histogramID : *histogramVec) {
        printHistogram(histogramID);
    }
}

void Stacker::printHistogram(TString& histID) {
    THStack* histStack = new THStack(histID, histID);
    TLegend* legend = getLegend();
    processes->fillStack(histStack, histID, legend);

    TCanvas* canv = getCanvas(histID);
    canv->cd();
    TPad* pad = getPad(histID);
    pad->cd();

    histStack->Draw();
    legend->Draw();

    canv->Print(histID + ".png");
    
}