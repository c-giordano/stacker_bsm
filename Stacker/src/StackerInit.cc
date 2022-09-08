#include "../interface/Stacker.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <TObjString.h>

#include <TKey.h>

Stacker::Stacker(std::vector<std::string>& cmdArgs) {
    // split between root files and settings file

    TH1::AddDirectory(false);

    unsigned settingFileNb = 0;
    bool era_16Pre = false;
    bool era_16Post = false;
    bool era_17 = false;
    bool era_18 = false;

    if (cmdArgs.back() == "OLD") oldStuff = true;

    for (auto it : cmdArgs) {
        // if string does not contain .root: break
        if (! stringContainsSubstr(it, ".root")) break;
        
        TFile* newInputFile = new TFile(it.c_str(), "read");
        inputfiles.push_back(newInputFile);

        if (stringContainsSubstr(it, "2016Pre")) era_16Pre = true;
        if (stringContainsSubstr(it, "2016Post")) era_16Post = true;
        if (stringContainsSubstr(it, "2017")) era_17 = true;
        if (stringContainsSubstr(it, "2018")) era_18 = true;

        settingFileNb++;
    }

    unsigned sumYears = unsigned(era_16Pre) + unsigned(era_16Post) + unsigned(era_17) + unsigned(era_18);
    if (sumYears > 1) {
        yearID = "AllEras";
    } else  if (era_16Pre && era_16Post && !era_17 && !era_18) {
        yearID = "2016";
    } else {
        if (era_16Pre) yearID = "2016PreVFP";
        else if (era_16Post) yearID = "2016PostVFP";
        else if (era_17) yearID = "2017";
        else if (era_18) yearID = "2018";
        else yearID = getYearFromRootFile(cmdArgs[0]);
    }
    
    std::string outputfilename = "combineFiles/Tmp/" + yearID + ".root";
    
    outputfile = new TFile(outputfilename.c_str(), "recreate");

    inputfile = new TFile(cmdArgs[0].c_str(), "read");
    processes = new ProcessList();

    if (runT2B) {
        pathToOutput = "/user/nivanden/public_html/Most_recent_plots/";
    } else {
        pathToOutput = "Output/";
    }

    ReadSettingFile(cmdArgs[settingFileNb]);
}

void Stacker::ReadSettingFile(std::string& settingFile) {
    std::ifstream infile(settingFile);
    std::string line;

    if (! infile.is_open()) {
        std::cout << "ERROR: Settingsfile " << settingFile << " not found" << std::endl;
        exit(1);
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

        // check for special stuff
        std::string part;
        if (stream >> part) {
            // check if it contains a plus. If so, the current process must be modified to take into account multiple subdirectories
            // maybe make a process object containing multiple processes
            std::vector<std::string> processNamesStrings = split(part, "+");
            std::vector<TString> processNamesSet;
            for (auto it : processNamesStrings) {
                processNamesSet.push_back(TString(it));
            }

            processes->addProcess(processNameAlt, processNamesSet, std::stoi(colorString), inputfiles, outputfile, signal, data, oldStuff);

        } else {
            processes->addProcess(processNameAlt, std::stoi(colorString), inputfiles, outputfile, signal, data, oldStuff);
        }
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
            altOutput = currSetAndVal.second;

            std::string filename = inputfile->GetName();
            filename = getFilename(filename);
            std::string datestring;

            if (inputfile->GetListOfKeys()->Contains("Timestamp")) {
                TObjString* ts, *br, *an, *st;
                std::string brStr = "NA";
                std::string anStr = "NA";
                std::string stStr = "NA";
                std::string extras = "";

                inputfile->GetObject( "Timestamp" , ts);
                if (inputfile->GetListOfKeys()->Contains("Branch")) {
                    inputfile->GetObject( "Branch" , br);
                    brStr = br->GetString().Data();
                }
                if (inputfile->GetListOfKeys()->Contains("AN_Type")) {
                    inputfile->GetObject( "AN_Type" , an);
                    anStr = an->GetString().Data();
                }
                if (inputfile->GetListOfKeys()->Contains("EventSelectionType")) {
                    inputfile->GetObject( "EventSelectionType" , st);
                    stStr = st->GetString().Data();
                }
                if (stringContainsSubstr(std::string(inputfile->GetName()), "_CR_")) {
                    extras += "_CR";
                }
                datestring = std::string(ts->GetString().Data()) + "_" + anStr + "_" + brStr + "_" + stStr + "_" + yearID + extras;
            } else {
                size_t firstPos = filename.find_first_of('_');
                size_t lastPos = filename.find_last_of('_');

                datestring = filename.substr(firstPos+1, lastPos-firstPos-1);
            }

            std::string baseDir = "/user/nivanden/public_html/PreviousVersions/";
            std::string subDir = removeOccurencesOf(altOutput, "Most_recent_plots/");

            int response = std::system( ("mkdir " + baseDir + subDir + datestring).c_str());
            if (response < 0) std::cout << "mkdir failed" << std::endl;

            response = std::system( ("cp /user/nivanden/public_html/index.php " + baseDir + subDir + datestring).c_str());
            if (response < 0) std::cout << "cp failed" << std::endl;


            pathToOutput = baseDir + subDir + datestring + "/";

            std::vector<std::string> subdirs = {"CRWZ", "CRZZ", "CR-Conversion","CR-3L-Z", "CR-4L-Z", "CR-2L-23J1B", "CR-2L-23J1B++", "CR-2L-23J1B--", "CR-2L-23J1Bee", "CR-2L-23J1Bem", "CR-2L-23J1Bmm", "CR-3L-2J1B", "CR-2L-45J2B", "SR-2L", "SR-2Lee", "SR-2Lem", "SR-2Lmm", "SR-2L++", "SR-2L--", "SR-3L", "SR-3LnoOSSF", "SR-3LOSSF", "SR-4L"};
            // {"DL", "3L", "4L", "3LnoOSSF", "3LOSSF", "DLee", "DLem", "DLmm", "DL++", "DL--", "CR", "CRO", "CRZ", "CRO-3L", "CRZ-4L", "CRWZ"};
            //std::vector<std::string> subdirs = {"DL", "3L", "4L"};
            
            for (auto currSub : subdirs) {
                response = std::system( ("mkdir " + baseDir + subDir + datestring + "/" + currSub).c_str());
                if (response < 0) std::cout << "mkdir failed for " << currSub << std::endl;

                response = std::system( ("cp /user/nivanden/public_html/index.php " + baseDir + subDir + datestring + "/" + currSub).c_str());
                if (response < 0) std::cout << "cp failed" << std::endl;
            }
        } else if (currSetAndVal.first == "RatioPlots" && currSetAndVal.second == "True") {
            isRatioPlot = true;
        } else if (currSetAndVal.first == "SignalYield" && currSetAndVal.second == "True") {
            isSignalLine = true;
        }
        // Set gen settings
    }

    // walk in inputfile to first process, check all histogram names   
    inputfile->cd("Nominal");
    Process* curr = processes->getHead();
    //while (curr != nullptr && ! gDirectory->GetDirectory(curr->getName())) {
    //    for (auto it : inputfiles) {
    //        it->cd("Nominal");>
    //        if (gDirectory->GetDirectory(curr->getName())) {
    //            break;
    //        }
    //    }
    //    curr = curr->getNext();
    //}

    for (auto it : inputfiles) {
        curr = processes->getHead();
        it->cd("Nominal");
        while (curr != nullptr && ! gDirectory->GetDirectory(curr->getName())) {
            curr = curr->getNext();
        }
        if (curr != nullptr && gDirectory->GetDirectory(curr->getName())) {
            break;
        }
        
    }
    //if (curr == nullptr) {
    //    curr = processes->getHead();
    //
    //    for (auto it : inputfiles) {
    //        it->cd("Nominal");
    //        if (gDirectory->GetDirectory(curr->getName())) {
    //            break;
    //        }
    //    }
    //}
    gDirectory->cd(curr->getName());
    gDirectory->cd(gDirectory->GetListOfKeys()->At(0)->GetName());

    TList* histogramsAvailable = gDirectory->GetListOfKeys();

    for (auto const&& obj : *histogramsAvailable) {
        if (std::string(((TKey*) obj)->GetClassName()) == "TH1D") {
            Histogram* hist = new Histogram(TString(obj->GetName()));
            histogramVec.push_back(hist);
        } else if (std::string(((TKey*) obj)->GetClassName()) == "TH2D") {
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