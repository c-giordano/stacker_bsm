#ifndef PROCESS_H
#define PROCESS_H

#include <TString.h>
#include <TFile.h>
#include <TH1.h>
#include <TH2.h>
#include <TLegend.h>

#include <iostream>

#include "../../Helpers/interface/StringTools.h"

// Processes as linkedlist: first element of list is pointed to by ProcessList
// Should make it relatively easy to work in a specific order?
// 

class Process {
    private:
        Process* next = nullptr;

        TString name;
        TString cleanedName;
        std::vector<const char*>* subdirectories;
        Color_t color;
        TFile* rootFile;

        bool isSignal;
        bool isData;
    public:
        Process(TString& procName, int procColor, TFile* procInputfile, bool signal, bool data);
        ~Process() {};

        void setNext(Process* newNext) {next = newNext;}
        Process* getNext() const {return next;}
        TString const getName() {return name;}
        TString const getCleanedName()  {return cleanedName;}
        bool isSignalProcess() {return isSignal;}

        TH1D* getHistogram(TString& histName);
        TH1D* getHistogramUncertainty(TString& uncName, TString& histName);

        TH2D* get2DHistogram(TString& histName, TLegend* legend);

};

#endif