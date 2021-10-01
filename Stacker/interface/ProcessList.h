#ifndef PROCESSLIST_H
#define PROCESSLIST_H

#include "Process.h"

#include <TString.h>
#include <TFile.h>
#include <THStack.h>

class ProcessList {
    private:
        Process* head = nullptr;
        Process* tail = nullptr;

        bool verbose = false;
    public:
        ProcessList() = default;
        ~ProcessList();

        void addProcess(TString& name, int color, TFile* inputfile, bool signal, bool data);

        Process* getHead() {return head;}
        Process* getTail() {return tail;}

        std::vector<TH1D*> fillStack(THStack* stack, TString& histogramID, TLegend* legend, TFile* outfile);

        void setVerbosity(bool verbosity) {verbose = verbosity;}

};

#endif