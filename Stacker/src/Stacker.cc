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
        std::cout << line << std::endl;
        if(! considerLine(&line)){
            continue;
        }
        
        if (line.find("BROAD SETTINGS") != std::string::npos || line.find("HISTOGRAMS") != std::string::npos) {
            std::cout << "Parsing BROAD SETTINGS" << std::endl;
            break;
        }
        // first, a processname is read, from this, a processelement is built and for this processname a set of directories is achieved
        std::istringstream stream(line);
        
        std::string processName;
        std::string colorString;
        std::string type;
        stream >> processName >> colorString >> type;
        
        // Color_t currentColor = std::stoi(colorString);
        TString processNameAlt(processName);

        bool signal = false;
        bool data = false;
        if (type.find('S') != std::string::npos) {
            signal = true;
        } else if (type.find('D') != std::string::npos) {
            data = true;
        } else if (type.find('B') == std::string::npos) {
            std::cerr << "Error: type identifier '" << type << "' unknown" << std::endl;
            exit(1);
        }

        processes->addProcess(processNameAlt, std::stoi(colorString), inputfile, signal, data);
    }

    while (getline(infile, line)) {
        if (!considerLine(&line)) {
            continue;
        }

        if (line.find("HISTOGRAMS") != std::string::npos) {
            std::cout << "Parsing HISTOGRAMS" << std::endl;
            break;
        }

        std::pair<std::string, std::string> currSetAndVal = splitSettingAndValue(line);

        if (currSetAndVal.first == "Lumi") {
            setLumi(currSetAndVal.second);
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
    //outputfile->Close();

    delete inputfile;
    //delete outputfile;
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
    std::vector<TH1D*> histVec = processes->fillStack(histStack, histID, legend);

    TCanvas* canv = getCanvas(histID);
    canv->Draw();
    canv->cd();
    TPad* pad = getPad(histID);
    pad->Draw();
    pad->cd();

    histStack->Draw("HIST");

    histStack->GetXaxis()->SetTitle(histVec[0]->GetXaxis()->GetTitle());
    histStack->GetYaxis()->SetTitle(histVec[0]->GetYaxis()->GetTitle());
    histStack->SetMaximum(histStack->GetMaximum() * 1.2); // stack->SetMaximum(stack->GetMaximum("NOSTACK") * 1.2);

    legend->Draw();

    TLatex* info = getDatasetInfo();

    canv->Print("Output/" + histID + ".png");
}