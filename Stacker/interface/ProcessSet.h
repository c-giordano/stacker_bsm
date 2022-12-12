#ifndef PROCESSSET_H
#define PROCESSSET_H

#include "Process.h"



class ProcessSet : public Process {
    private:
        /* data */
        std::vector<Process*> subProcesses;

    public:
        // constructor should basically be process constructor but with vector for names and looping for each subprocess
        ProcessSet(TString& name, std::vector<TString>& procNames, int procColor, TFile* procInputfile, TFile* outputFile, bool signal, bool data, bool OldStuff);
        ProcessSet(TString& name, std::vector<TString>& procNames, int procColor, std::vector<TFile*>& inputfiles, TFile* outputFile, bool signal, bool data, bool OldStuff);

        ~ProcessSet() {};

        virtual bool isSet() {return true;};

        // overloading of getHistogram functions which basically does the same but sums over all the thingies?
        virtual TH1D* getHistogram(Histogram* histogram);
        virtual TH1D* getHistogramUncertainty(std::string& uncName, std::string& upOrDown, Histogram* hist, std::string& outputFolder, bool envelope, std::string era="");

        virtual TH2D* get2DHistogram(TString& histName, TLegend* legend);
        virtual std::vector<std::shared_ptr<TH1D>> GetAllVariations(Histogram* histogram, int nVars, std::string& uncName);
};

#endif