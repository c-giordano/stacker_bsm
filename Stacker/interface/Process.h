#ifndef PROCESS_H
#define PROCESS_H

#include <TString.h>
#include <TFile.h>
#include <TH1.h>

#include <iostream>

// Processes as linkedlist: first element of list is pointed to by ProcessList
// Should make it relatively easy to work in a specific order?
// 

class Process {
    private:
        Process* next = nullptr;

        TString name;
        std::vector<const char*>* subdirectories;
        Color_t color;
        TFile* rootFile;
    public:
        Process(TString& procName, int procColor, TFile* procInputfile);
        ~Process() {};

        void setNext(Process* newNext) {next = newNext;}
        Process* const getNext() {return next;}

        TString const getName() {return name;}

        TH1D* getHistogram(TString& histName);
};

#endif