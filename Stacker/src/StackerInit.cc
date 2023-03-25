#include "../interface/Stacker.h"

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <TObjString.h>
#include <set>
#include <boost/filesystem.hpp>

#include <TKey.h>

Stacker::Stacker(std::vector<std::string>& cmdArgs) {
    // split between root files and settings file

    TH1::AddDirectory(false);
    std::cout << "Building stacker" << std::endl;

    unsigned settingFileNb = 0;
    bool era_16Pre = false;
    bool era_16Post = false;
    bool era_17 = false;
    bool era_18 = false;

    if (cmdArgs.back() == "OLD") oldStuff = true;

    for (auto it : cmdArgs) {
        // if string does not contain .root: break
        if (! stringContainsSubstr(it, ".root")) break;
        std::cout << it << std::endl;
        //std::cout << "Loading input file" << std::endl;
        
        TFile* newInputFile = new TFile(it.c_str(), "read");
        //std::cout << "Loaded input file" << std::endl;

        inputfiles.push_back(newInputFile);

        if (stringContainsSubstr(it, "2016Pre")) era_16Pre = true;
        if (stringContainsSubstr(it, "2016Post")) era_16Post = true;
        if (stringContainsSubstr(it, "2017")) era_17 = true;
        if (stringContainsSubstr(it, "2018")) era_18 = true;
        //std::cout << "era chosen" << std::endl;

        settingFileNb++;
    }
    //std::cout << "ran over files" << std::endl;


    unsigned sumYears = unsigned(era_16Pre) + unsigned(era_16Post) + unsigned(era_17) + unsigned(era_18);
    
    if (era_16Pre && era_16Post && !era_17 && !era_18) {
        yearID = "2016";
    } else if (sumYears > 1) {
        yearID = "AllEras";
    } else {
        if (era_16Pre) yearID = "2016PreVFP";
        else if (era_16Post) yearID = "2016PostVFP";
        else if (era_17) yearID = "2017";
        else if (era_18) yearID = "2018";
        else yearID = getYearFromRootFile(cmdArgs[0]);
    }

    //std::cout << "got year " << yearID << std::endl;

    
    std::string outputfilename = "combineFiles/Tmp/" + yearID + ".root";
    
    outputfile = new TFile(outputfilename.c_str(), "recreate");

    inputfile = new TFile(cmdArgs[0].c_str(), "read");
    processes = new ProcessList();

    if (runT2B) {
        pathToOutput = "/user/nivanden/public_html/Most_recent_plots/";
    } else {
        pathToOutput = "Output/";
    }
    //std::cout << "going to read settingfile" << yearID << std::endl;

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
        Process* newProcess;
        bool read = bool(stream >> part);
        if (read && stringContainsSubstr(part, "Set=")) {
            std::pair<std::string, std::string> currSetAndVal = splitSettingAndValue(part);
            // check if it contains a plus. If so, the current process must be modified to take into account multiple subdirectories
            // maybe make a process object containing multiple processes
            std::vector<std::string> processNamesStrings = split(currSetAndVal.second, "+");
            std::vector<TString> processNamesSet;
            for (auto it : processNamesStrings) {
                processNamesSet.push_back(TString(it));
            }

            newProcess = processes->addProcess(processNameAlt, processNamesSet, std::stoi(colorString), inputfiles, outputfile, signal, data, oldStuff);
        } else {
            newProcess = processes->addProcess(processNameAlt, std::stoi(colorString), inputfiles, outputfile, signal, data, oldStuff);
        }

        if (read && stringContainsSubstr(part, "Set=")) {
            read = bool(stream >> part);
        }

        if (read) {
            std::pair<std::string, std::string> currSetAndVal = splitSettingAndValue(part);

            if (currSetAndVal.first == "IgnoreChannels") {
                std::vector<std::string> ignoredChannels = split(currSetAndVal.second, ",");

                for (auto& channel : ignoredChannels) {
                    newProcess->AddIgnoredChannel(channel);
                }
            }
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

    for (auto it : inputfiles) {
        curr = processes->getHead();
        it->cd("Nominal");
        std::cout << gDirectory->GetListOfKeys()->At(0)->GetName() << std::endl;
        
        while (curr != nullptr && ! gDirectory->GetDirectory(curr->getName())) {
            std::cout << curr->getName() << std::endl;
            if (curr != nullptr && curr->isSet()) {
                std::vector<Process*> vec = ((ProcessSet*) curr)->getSubProcesses();
                Process* tmp = nullptr;
                for (auto& el : vec) {
                    if (gDirectory->GetDirectory(el->getName())) {
                        tmp = el;
                        break;
                    }
                }
                if (tmp) curr = tmp;
                break;
            }

            curr = curr->getNext();
        }
        if (curr != nullptr && gDirectory->GetDirectory(curr->getName())) {
            break;
        }
    }

    gDirectory->cd(curr->getName());
    gDirectory->cd(gDirectory->GetListOfKeys()->At(0)->GetName());

    TList* histogramsAvailable = gDirectory->GetListOfKeys();
    std::set<std::string> channels;

    for (auto const&& obj : *histogramsAvailable) {
        if (std::string(((TKey*) obj)->GetClassName()) == "TH1D") {
            Histogram* hist = new Histogram(TString(obj->GetName()));
            channels.insert(hist->GetChannel());
            histogramVec.push_back(hist);
        } else if (std::string(((TKey*) obj)->GetClassName()) == "TH2D") {
            Histogram2D* hist = new Histogram2D(TString(obj->GetName()));
            histogramVec2D.push_back(hist);
        }
    }

    int response;
    for (auto& currSub : channels) {
        if (stringContainsSubstr(currSub, "-CH-")) {
            std::vector< std::string > channelAndSubchannel = split(currSub, "-CH-");
            if (! boost::filesystem::exists((pathToOutput + channelAndSubchannel[0]).c_str())) {
                response = std::system( ("mkdir " + pathToOutput + channelAndSubchannel[0]).c_str());
                if (response < 0) std::cout << "mkdir failed for " << channelAndSubchannel[0] << std::endl;

                response = std::system( ("cp /user/nivanden/public_html/index.php " + pathToOutput + channelAndSubchannel[0]).c_str());
                if (response < 0) std::cout << "cp failed" << std::endl;
            }

            response = std::system( ("mkdir " + pathToOutput + channelAndSubchannel[1]).c_str());
            if (response < 0) std::cout << "mkdir failed for " << channelAndSubchannel[1] << std::endl;

            response = std::system( ("cp /user/nivanden/public_html/index.php " + pathToOutput + channelAndSubchannel[1]).c_str());
            if (response < 0) std::cout << "cp failed" << std::endl;

        } else {
            // if region is region-CH-subregion, split in two, check if first exists and then build second
            response = std::system( ("mkdir " + pathToOutput + currSub).c_str());
            if (response < 0) std::cout << "mkdir failed for " << currSub << std::endl;

            response = std::system( ("cp /user/nivanden/public_html/index.php " + pathToOutput + currSub).c_str());
            if (response < 0) std::cout << "cp failed" << std::endl;
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