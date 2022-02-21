#include "../interface/Process.h"

Process::Process(TString& procName, int procColor, TFile* procInputfile, TFile* outputFile, bool signal, bool data, bool oldStuff) : name(procName), rootFile(procInputfile),
    outputFile(outputFile), isSignal(signal), isData(data) {
    color = procColor;
    cleanedName = cleanTString(name);

    inputfiles.push_back(rootFile);

    for (auto it : inputfiles) {
        it->cd("Nominal");

        if (! gDirectory->GetDirectory(procName)) {
            std::cout << "ERROR: Process " << procName << " not found." << std::endl;
            std::cout << "Trying rootfile itself..." << std::endl;
            it->cd();

            if (! gDirectory->GetDirectory(procName)) {
                std::cout << "ERROR: Process still " << procName << " not found." << std::endl;
                exit(2);
            }
        }

        std::vector<const char*>* subdirectories = new std::vector<const char*>;

        if (procName == "nonPrompt" && oldStuff) {
            subdirectories->push_back("../nonPrompt");
        } else {

            gDirectory->cd(procName);

            TList* folders = gDirectory->GetListOfKeys();

            for(const auto&& obj: *folders) {
                subdirectories->push_back(obj->GetName());
            }
        }
        subdirectoriesPerFile.push_back(subdirectories);
    }
}

Process::Process(TString& procName, int procColor, std::vector<TFile*>& inputfiles, TFile* outputFile, bool signal, bool data, bool oldStuff) : name(procName), rootFile(inputfiles[0]),
    inputfiles(inputfiles), outputFile(outputFile), isSignal(signal), isData(data) {
    color = procColor;
    cleanedName = cleanTString(name);

    for (auto it : inputfiles) {
        it->cd("Nominal");

        if (! gDirectory->GetDirectory(procName)) {
            std::cout << "ERROR: Process " << procName << " not found." << std::endl;
            std::cout << "Trying rootfile itself..." << std::endl;
            it->cd();

            if (! gDirectory->GetDirectory(procName)) {
                std::cout << "ERROR: Process still " << procName << " not found." << std::endl;
                exit(2);
            }
        }
        std::vector<const char*>* subdirectories = new std::vector<const char*>;

        if (procName == "nonPrompt" && oldStuff) {
            subdirectories->push_back("../nonPrompt");
        } else {
            gDirectory->cd(procName);

            TList* folders = gDirectory->GetListOfKeys();

            for(const auto&& obj: *folders) {
                subdirectories->push_back(obj->GetName());
            }
        }
        subdirectoriesPerFile.push_back(subdirectories);
    }
}

TH1D* Process::getHistogram(TString& histName) {
    TH1D* output = nullptr;// = new TH1D();// = new TH1D(histName + "_" + name, name)

    for (unsigned i = 0; i < inputfiles.size(); i++) {
        TFile* currFile = inputfiles[i];
        std::vector<const char*>* subdirectories = subdirectoriesPerFile[i];

        currFile->cd("Nominal");
        if (! gDirectory->GetDirectory(name)) {
            std::cout << "ERROR: Process " << name << " not found." << std::endl;
            std::cout << "Trying rootfile itself..." << std::endl;
            currFile->cd();

            if (! gDirectory->GetDirectory(name)) {
                std::cout << "ERROR: Process still " << name << " not found." << std::endl;
                exit(2);
            }
        }
        gDirectory->cd(name);

        for(auto subdir : *subdirectories) {
            gDirectory->cd(subdir);
            
            TH1D* inter;
            gDirectory->GetObject(histName, inter);

            if (output == nullptr) {
                output = new TH1D(*inter);
            } else {
                output->Add(inter);
            }

            // Read stuff, add to outputhistogram (maybe first output is stack but then print it to th?)

            gDirectory->cd("..");
        }
    }

    output->SetName(histName + name);
    output->SetTitle(histName + name);
    
    output->SetLineColor(color);
    output->SetFillColor(color);
    output->SetMarkerColor(color);
    
    return output;
}

TH1D* Process::getHistogramUncertainty(std::string& uncName, std::string& upOrDown, Histogram* hist, std::string& outputFolder, bool envelope) {
    TString histName = hist->getID(); // + "_" + uncName + "_" + upOrDown;
    TH1::AddDirectory(false);
    // std::cout << histName << std::endl;
    TH1D* output = nullptr;

    for (unsigned i = 0; i < inputfiles.size(); i++) {
        TFile* currFile = inputfiles[i];
        std::vector<const char*>* subdirectories = subdirectoriesPerFile[i];
        
        currFile->cd("Uncertainties");
        gDirectory->cd(name);

        //std::cout << name << " UNCERTAINTY: " << histName.Data() << "\t";

        for (auto subdir : *subdirectories) {
            if (! envelope) gDirectory->cd(subdir);
            
            // std::cout << subdir << std::endl;
            if (! gDirectory->GetDirectory(uncName.c_str())) {
                std::cout << "ERROR: Uncertainty " << uncName << " in process " << subdir << " not found. Should it?" << std::endl;
                exit(2);
            }

            gDirectory->cd(uncName.c_str());
            gDirectory->cd(upOrDown.c_str());
            
            TH1D* inter;
            gDirectory->GetObject(histName, inter);
            if (inter != nullptr) {
                if (output == nullptr) {
                    output = new TH1D(*inter);
                } else {
                    output->Add(inter);
                }
            } else {
                std::cout << "Histogram " << histName << " not found for uncertainty " << uncName << " in " << subdir << " for process" << getName().Data() << ". Should it exist?" << std::endl;
            }

            //std::cout << output->Integral() << "\t";

            gDirectory->cd("..");
            gDirectory->cd("..");
            gDirectory->cd("..");
            
            if (envelope) break;
        }
    }

    if (output == nullptr) {
        //std::cout << "Histogram " << histName << " has no uncertainty " << uncName << std::endl;
        return nullptr;
    }
    
    for (int j=1; j < output->GetNbinsX() + 1; j++) {
        if (output->GetBinContent(j) <= 0.) output->SetBinContent(j, 0.00001);
    }

    if (hist->getPrintToFile()) {
        outputFile->cd();
        outputFile->cd((hist->getCleanName()).c_str());

        gDirectory->cd((outputFolder + upOrDown).c_str());

        output->Write(name);
    }

    output->SetName(histName + name + TString(uncName + upOrDown));
    output->SetTitle(histName + name + TString(uncName + upOrDown));

    // fix writingOutput
    
    return output;
}

TH2D* Process::get2DHistogram(TString& histName, TLegend* legend) {
    TH2D* output = nullptr;// = new TH1D();// = new TH1D(histName + "_" + name, name)

    for (unsigned i = 0; i < inputfiles.size(); i++) {
        TFile* currFile = inputfiles[i];
        std::vector<const char*>* subdirectories = subdirectoriesPerFile[i];

        currFile->cd("Nominal");
        if (! gDirectory->GetDirectory(name)) {
            std::cout << "ERROR: Process " << name << " not found." << std::endl;
            std::cout << "Trying rootfile itself..." << std::endl;
            currFile->cd();

            if (! gDirectory->GetDirectory(name)) {
                std::cout << "ERROR: Process still " << name << " not found." << std::endl;
                exit(2);
            }
        }
        gDirectory->cd(name);

        for(auto subdir : *subdirectories) {
            gDirectory->cd(subdir);
            
            TH2D* inter;
            gDirectory->GetObject(histName, inter);

            if (output == nullptr) {
                output = new TH2D(*inter);
            } else {
                output->Add(inter);
            }

            // Read stuff, add to outputhistogram (maybe first output is stack but then print it to th?)

            gDirectory->cd("..");
        }

    }

    output->SetName(histName + name);
    output->SetTitle(histName + name);
    
    output->SetLineColor(color);
    output->SetFillColor(color);
    output->SetMarkerColor(color);
    
    legend->AddEntry(output, cleanedName);

    return output;
}
