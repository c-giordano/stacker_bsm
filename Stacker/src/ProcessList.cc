#include "../interface/ProcessList.h"

void ProcessList::addProcess(TString& name, int color, TFile* inputfile) {
    // TODO: Create new process object
    Process* brandNewObj = new Process(name, color, inputfile);

    tail->setNext(brandNewObj);
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

void ProcessList::fillStack(THStack* stack, TString& histogramID) {
    Process* current = head;

    while (current) {
        stack->Add(current->getHistogram(histogramID));
    }
}
