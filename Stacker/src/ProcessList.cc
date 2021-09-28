#include "../interface/ProcessList.h"

void ProcessList::addProcess(TString& name, int color, TFile* inputfile, bool signal, bool data) {
    // TODO: Create new process object
    Process* brandNewObj = new Process(name, color, inputfile, signal, data);

    if (tail) tail->setNext(brandNewObj); // check if tail already exists
    tail = brandNewObj;

    if (! head) {
        head = brandNewObj;
    }
}

ProcessList::~ProcessList() {
    Process* toDel = head;
    while (toDel->getNext()) {
        Process* nextToDel = toDel->getNext();
        delete toDel;
        toDel = nextToDel;
    }
}

std::vector<TH1D*> ProcessList::fillStack(THStack* stack, TString& histogramID, TLegend* legend, TFile* outfile) {
    Process* current = head;
    std::vector<TH1D*> histVec;

    double signalYield = 0.;
    double bkgYield = 0.;

    outfile->mkdir(histogramID);

    while (current) {
        TH1D* histToAdd = current->getHistogram(histogramID, legend);
        stack->Add(histToAdd);
        histVec.push_back(histToAdd);
        
        if (current->isSignalProcess()) {
            signalYield += histToAdd->Integral();
        } else {
            bkgYield += histToAdd->Integral();
        }

        outfile->cd(histogramID);
        histToAdd->Write(current->getName(), TObject::kOverwrite);

        current = current->getNext();
    }

    std::cout << "S/B = " << signalYield << "/" << bkgYield << std::endl;

    return histVec;
}
