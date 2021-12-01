#include "../interface/ProcessList.h"

void ProcessList::addProcess(TString& name, int color, TFile* inputfile, bool signal, bool data) {
    // TODO: Create new process object
    allProcessNames.push_back(name);

    Process* brandNewObj = new Process(name, color, inputfile, signal, data);

    if (tail) tail->setNext(brandNewObj); // check if tail already exists
    tail = brandNewObj;

    if (! head) {
        head = brandNewObj;
    }
}

Uncertainty* ProcessList::addUncertainty(std::string& name, bool flat, bool corrProcess, bool eraSpec, std::vector<TString>& processes) {
    Uncertainty* brandNewObj = new Uncertainty(name, flat, corrProcess, eraSpec, processes);

    if (tailUnc) tailUnc->setNext(brandNewObj); // check if tail already exists
    tailUnc = brandNewObj;

    if (! headUnc) {
        headUnc = brandNewObj;
    }

    return brandNewObj;
}


ProcessList::~ProcessList() {
    Process* toDel = head;
    while (toDel->getNext()) {
        Process* nextToDel = toDel->getNext();
        delete toDel;
        toDel = nextToDel;
    }
}

std::vector<TH1D*> ProcessList::fillStack(THStack* stack, Histogram* hist, TLegend* legend, TFile* outfile, std::vector<TH1D*>* signalHistograms, TH1D** sysUnc) {
    Process* current = head;
    std::vector<TH1D*> histVec;

    TString histogramID = hist->getID();

    double signalYield = 0.;
    double bkgYield = 0.;

    outfile->mkdir(histogramID);

    if (verbose) std::cout << histogramID << std::endl;

    while (current) {
        TH1D* histToAdd = current->getHistogram(histogramID);
        legend->AddEntry(histToAdd, current->getCleanedName());
        stack->Add(histToAdd);
        histVec.push_back(histToAdd);
        
        if (current->isSignalProcess()) {
            signalYield += histToAdd->Integral();
            TH1D* signalHist = new TH1D(*histToAdd);
            signalHistograms->push_back(signalHist);
        } else {
            bkgYield += histToAdd->Integral();
        }

        if (verbose) {
            std::cout << current->getName() << ": " << histToAdd->Integral() << " events" << std::endl;
        }

        outfile->cd(histogramID);
        histToAdd->Write(current->getName(), TObject::kOverwrite);

        current = current->getNext();
    }
    

    // loop uncertainties as well if required
    Uncertainty* currUnc = headUnc;
    //TH1D* totalUncSq = nullptr;
    std::vector<TH1D*> uncVec;
    while (currUnc && hist->getDrawUncertainties()) {
        // getShapeUncertainty or apply flat uncertainty
        TH1D* newUncertainty = currUnc->getUncertainty(histogramID, head, histVec);
        uncVec.push_back(newUncertainty);
        if (*sysUnc == nullptr) {
            *sysUnc = new TH1D(*newUncertainty);
        } else {
            (*sysUnc)->Add(newUncertainty);
        }

        currUnc = currUnc->getNext();
    }
    
    
    if (verbose) std::cout << "S/B = " << signalYield << "/" << bkgYield << std::endl;

    return histVec;
}

std::vector<TH2D*> ProcessList::fill2DStack(THStack* stack, TString& histogramID, TLegend* legend, TFile* outfile) {
    Process* current = head;
    std::vector<TH2D*> histVec;

    double signalYield = 0.;
    double bkgYield = 0.;

    outfile->mkdir(histogramID);

    if (verbose) std::cout << histogramID << std::endl;

    while (current) {
        TH2D* histToAdd = current->get2DHistogram(histogramID, legend);
        stack->Add(histToAdd);
        histVec.push_back(histToAdd);
        
        if (current->isSignalProcess()) {
            signalYield += histToAdd->Integral();
        } else {
            bkgYield += histToAdd->Integral();
        }

        if (verbose) {
            std::cout << current->getName() << ": " << histToAdd->Integral() << " events" << std::endl;
        }

        outfile->cd(histogramID);
        histToAdd->Write(current->getName(), TObject::kOverwrite);

        current = current->getNext();
    }
    
    if (verbose) std::cout << "S/B = " << signalYield << "/" << bkgYield << std::endl;

    return histVec;
}