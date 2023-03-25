#include "../interface/ProcessSet.h"


ProcessSet::ProcessSet(TString& name, std::vector<TString>& procNames, int procColor, TFile* procInputfile, TFile* outputFile, bool signal, bool data, bool OldStuff) :
    Process(name, procColor, procInputfile, outputFile, signal, data, OldStuff)
    {
    for (auto it : procNames) {
        Process* newProc = new Process(it, procColor, procInputfile, outputFile, signal, data, OldStuff);
        subProcesses.push_back(newProc);
    }
}

ProcessSet::ProcessSet(TString& name, std::vector<TString>& procNames, int procColor, std::vector<TFile*>& inputfiles, TFile* outputFile, bool signal, bool data, bool OldStuff) :
    Process(name, procColor, inputfiles, outputFile, signal, data, OldStuff)
    {
    for (auto it : procNames) {
        Process* newProc = new Process(it, procColor, inputfiles, outputFile, signal, data, OldStuff);
        subProcesses.push_back(newProc);
    }
}


TH1D* ProcessSet::getHistogram(Histogram* histogram) {
    TH1D* output = nullptr;
    TString histName = histogram->getID();

    for (auto it : subProcesses) {
        //std::cout << it->getName().Data() << std::endl;
        TH1D* tmp = it->getHistogram(histogram);
        if (tmp == nullptr) {
            continue;
        }

        if (output == nullptr) {
            output = tmp;
        } else {
            output->Add(tmp);
        }
    }

    output->SetName(histName + getName());
    output->SetTitle(histName + getName());
    
    output->SetLineColor(getColor());
    output->SetFillColor(getColor());
    output->SetMarkerColor(getColor());

    return output;
}

TH1D* ProcessSet::getHistogramUncertainty(std::string& uncName, std::string& upOrDown, Histogram* hist, bool envelope, std::string era) {
    TH1D* output = nullptr;
   // bool printToFile = hist->getPrintToFile();
   // hist->setPrintToFile(false);
    //std::cout << "in processset " << getName().Data() << std::endl;
    for (auto it : subProcesses) {
        TH1D* tmp = it->getHistogramUncertainty(uncName, upOrDown, hist, envelope, era);
        
        if (output == nullptr) {
            output = tmp;
        } else {
            output->Add(tmp);
        }
    }

    output->SetName(hist->getID() + getName() + TString(uncName + upOrDown));
    output->SetTitle(hist->getID() + getName() + TString(uncName + upOrDown));

    return output;
}

TH2D* ProcessSet::get2DHistogram(TString& histName) {
    TH2D* output = nullptr;

    for (auto it : subProcesses) {
        TH2D* tmp = it->get2DHistogram(histName);

        if (output == nullptr) {
            output = tmp;
        } else {
            output->Add(tmp);
        }
    }

    output->SetName(histName + getName());
    output->SetTitle(histName + getName());
    
    output->SetLineColor(getColor());
    output->SetFillColor(getColor());
    output->SetMarkerColor(getColor());

    return output;
}

std::vector<std::shared_ptr<TH1D>> ProcessSet::GetAllVariations(Histogram* histogram, int nVars, std::string& uncName) {
    std::vector<std::shared_ptr<TH1D>> output;

    for (auto it : subProcesses) {
        std::vector<std::shared_ptr<TH1D>> tmp = it->GetAllVariations(histogram, nVars, uncName);        
        
        if (output.size() == 0) {
            output = tmp;
        } else {
            for (int i=0; i<nVars; i++) {
                output[i]->Add(tmp[i].get());
            }
        }
    }
    return output;
}
