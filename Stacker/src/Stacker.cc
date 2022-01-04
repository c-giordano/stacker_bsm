#include "../interface/Stacker.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>

#include <TKey.h>


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

    std::string outputfilename = rootFilename;
    yearID = getYearFromRootFile(outputfilename);
    outputfilename = "combineFiles/" + yearID + ".root";
    
    outputfile = new TFile(outputfilename.c_str(), "recreate");

    inputfile = new TFile(rootFilename, "read");
    processes = new ProcessList();

    std::ifstream infile(settingFile);
    std::string line;

    if (! infile.is_open()) {
        std::cout << "ERROR: Settingsfile not found" << std::endl;
        exit(1);
    }

    if (runT2B) {
        pathToOutput = "/user/nivanden/public_html/Most_recent_plots/";
    } else {
        pathToOutput = "Output/";
    }

    while (getline(infile, line)) {
        // parse processes
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

        processes->addProcess(processNameAlt, std::stoi(colorString), inputfile, outputfile, signal, data);
    }

    while (getline(infile, line)) {
        // parse settings
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
        } else if (currSetAndVal.first == "Drawopt") {
            setDrawOpt(currSetAndVal.second);
        } else if (currSetAndVal.first == "OutFolder" && runT2B) {
            pathToOutput = "/user/nivanden/public_html/" + currSetAndVal.second;
        } else if (currSetAndVal.first == "RatioPlots" && currSetAndVal.second == "True") {
            isRatioPlot = true;
        } else if (currSetAndVal.first == "SignalYield" && currSetAndVal.second == "True") {
            isSignalLine = true;
        }
        // Set gen settings
    }

    // walk in inputfile to first process, check all histogram names   
    inputfile->cd("Nominal");
    gDirectory->cd(processes->getHead()->getName());
    gDirectory->cd(gDirectory->GetListOfKeys()->At(0)->GetName());

    TList* histogramsAvailable = gDirectory->GetListOfKeys();

    for (auto const&& obj : *histogramsAvailable) {
        if (std::string(((TKey*) obj)->GetClassName()) == "TH1D") {
            Histogram* hist = new Histogram(TString(obj->GetName()));
            histogramVec.push_back(hist);
        } else {
            Histogram2D* hist = new Histogram2D(TString(obj->GetName()));
            histogramVec2D.push_back(hist);
        }
    }

    while (getline(infile, line)) {
        // parse histogramspecific settings
        if (!considerLine(&line)) {
            continue;
        }
        // Set histograms to check

        std::string histID;
        std::istringstream stream(line);

        stream >> histID;
        
        std::vector<Histogram*>::iterator it;
        it = std::find_if(histogramVec.begin(), histogramVec.end(), std::bind(Histogram::searchHist, std::placeholders::_1, histID));

        if (it == histogramVec.end()) {
            std::vector<Histogram2D*>::iterator it2D;
            it2D = std::find_if(histogramVec2D.begin(), histogramVec2D.end(), std::bind(Histogram2D::searchHist, std::placeholders::_1, histID));

            if (it2D == histogramVec2D.end()) {
                std::cout << histID << " not found!" << std::endl;
                continue;
            }

            (*it2D)->readSettings(stream);
        } else {
            // Manage settings. Call function in histogram class maybe to parse and fix setting.
            (*it)->readSettings(stream);
        }

        std::cout << "FOUND " << histID << std::endl;
    }
}

Stacker::~Stacker() {
    inputfile->Close();
    outputfile->Close();

    delete inputfile;
    delete outputfile;
}

void Stacker::readUncertaintyFile(std::string& filename) {
    uncertainties = true;

    // read file:
    // eerst flats, dan shapes,
    // entry per uncertainty,
    // per entry: which processes (or all, or allNonData (excludes fakes)), uncorrelated or correlated across processes?, uncorrelated and correlated across years?
    // each process should keep track of uncertainties

    std::ifstream infile(filename);
    std::string line;

    if (! infile.is_open()) {
        std::cout << "ERROR: Settingsfile not found" << std::endl;
        exit(1);
    }

    std::vector<TString> allProcesses = processes->getAllProcessNames();
    std::cout << allProcesses.size() << std::endl;
    /*
    bool histSpecified = false;
    while (getline(infile, line)) {
        if (!considerLine(&line)) {
            continue;
        }

        if (line.find("UNCERTAINTIES") != std::string::npos) {
            std::cout << "Parsing HISTOGRAMS" << std::endl;
            break;
        }

        TString histIDComp = line;
        
        for (auto hist : histogramVec) {
            if (hist->getID() == histIDComp) {
                hist->setDrawUncertainties(true);
                histSpecified = true;
                break;
            }
        }
    }*/

    for (auto hist : histogramVec) {
        hist->setDrawUncertainties(true);
    }
    

    while (getline(infile, line)) {
        if (!considerLine(&line)) {
            continue;
        }

        if (line.find("HISTOGRAMS") != std::string::npos) {
            std::cout << "Parsing histograms to print" << std::endl;
            break;
        }
        
        // read name, build class
        std::string name; 
        bool flat = false;
        bool envelope = false;
        bool eraSpec = false;
        bool allEras = false;
        bool corrProcess = true;
        bool corrEra = false;
        double flatTot = 0.;
        double flatUncertaintyEra = 0.;
        double flatUncertaintyAll = 0.;
        double flatUncertainty1718 = 0.;
        std::vector<TString> relProcess = allProcesses;

        std::istringstream uncLine(line);
        uncLine >> name;
        size_t splitPoint = name.find(':');
        name = name.substr(0, splitPoint);

        std::string part;

        for (; uncLine >> part;) {
            std::pair<std::string, std::string> currSetAndVal;
            if (stringContains(part, '=')) {
                currSetAndVal = splitSettingAndValue(part);
            }
            if (currSetAndVal.first == "flat") {
                flat = true;
                flatTot = std::stod(currSetAndVal.second);
            }
            if (part == "envelope") envelope = true;
            if (part == "uncorrelated") corrProcess = false;
            if (part == "correlated") corrProcess = true;
            if (part == "corrEra") eraSpec = true;
            if (currSetAndVal.first == "process") {
                if (currSetAndVal.second == "AllMC") {
                    relProcess = allProcesses;
                    std::vector<TString>::iterator it = std::find(relProcess.begin(), relProcess.end(), TString("nonPrompt"));
                    if (it != relProcess.end()) {
                        relProcess.erase(it);
                    }
                    
                    it = std::find(relProcess.begin(), relProcess.end(), TString("ChargeMisID"));
                    if (it != relProcess.end()) {
                        relProcess.erase(it);
                    }
                    continue;
                } else if (currSetAndVal.second == "All") {
                    continue;
                } else {
                    std::vector<TString> newProcess;
                    newProcess.push_back(TString(currSetAndVal.second));
                    relProcess = newProcess;
                    continue;
                }
            }

            if (currSetAndVal.first == "Era") {
                flatUncertaintyEra = std::stod(currSetAndVal.second);
                eraSpec = true;
            }
            if (currSetAndVal.first == "AllEra") {
                allEras = true;
                flatUncertaintyAll = std::stod(currSetAndVal.second);
            }
            if (currSetAndVal.first == "1718") flatUncertainty1718 = std::stod(currSetAndVal.second);
            
        }


        Uncertainty* newUnc = processes->addUncertainty(name, flat, envelope, corrProcess, corrEra, relProcess, outputfile);
        if (flat) {
            newUnc->setFlatRate(flatTot);
            newUnc->setFlatRateAll(flatUncertaintyAll);
            newUnc->setFlatRateEra(flatUncertaintyEra);
            newUnc->setFlatRate1718(flatUncertainty1718);
            newUnc->setEraSpec(eraSpec);
            newUnc->setBoth(eraSpec && allEras);
        } else {
            newUnc->setEraSpec(eraSpec);
            newUnc->setBoth(eraSpec && allEras);
        }
    }

    std::vector<Histogram*> histForDC;

    while (getline(infile, line)) {
        if (!considerLine(&line)) {
            continue;
        }

        std::istringstream stream(line);

        std::string histogramID;
        std::string cleanName;

        stream >> histogramID >> cleanName;

        // search histogram and fix stuff
        std::vector<Histogram*>::iterator it;
        it = std::find_if(histogramVec.begin(), histogramVec.end(), std::bind(Histogram::searchHist, std::placeholders::_1, histogramID));

        if (it != histogramVec.end()) {
            (*it)->setPrintToFile(true);
            (*it)->setCleanName(cleanName);
            histForDC.push_back(*it);
        } else {
            std::cerr << "HISTOGRAM NOT FOUND FOR UNCERTAINTY" << std::endl;
            exit(1);
        }
        std::cout << "FOUND " << histogramID << std::endl;
    }
    std::cout << histForDC.size() << std::endl;
    dcwriter = new DatacardWriter(yearID, processes, histForDC, outputfile);
    dcwriter->writeUncertainties(processes->getUncHead());
}
