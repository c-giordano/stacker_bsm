#ifndef UNCERTAINTY_H
#define UNCERTAINTY_H

#include <string>
#include <vector>

#include <TString.h>
#include <TH1.h>

#include "Process.h"

class Uncertainty {
    private:
        std::string name;
        TString nameUp;
        TString nameDown;
        bool flat;
        bool correlated;
        bool eraSpecific;

        double flatUncertainty; // value used in histograms
        double flatUncertaintyEra=0.0; // for datacards: this era of data
        double flatUncertaintyAll=0.0; // for datacards: all eras
        double flatUncertainty1718=0.0; // for datacards: all eras

        std::vector<TString> relevantProcesses;

        Uncertainty* next = nullptr;

    public:
        Uncertainty(std::string& name, bool flat, bool corrProcess, bool eraSpec, std::vector<TString>& processes);
        ~Uncertainty();

        void setFlatRate(double rate) {flatUncertainty = rate;}
        
        void setFlatRateEra(double rate) {flatUncertaintyEra = rate;}
        void setFlatRateAll(double rate) {flatUncertaintyAll = rate;}
        void setFlatRate1718(double rate) {flatUncertainty1718 = rate;}

        bool isFlat() {return flat;}

        Uncertainty* getNext() {return next;}
        void setNext(Uncertainty* newNext) {next = newNext;}

        TH1D* getShapeUncertainty(TString& histogramID, Process* head, std::vector<TH1D*>& histVec);
        TH1D* getFlatUncertainty(TString& histogramID, Process* head, std::vector<TH1D*>& histVec);
        TH1D* getUncertainty(TString& histogramID, Process* head, std::vector<TH1D*>& histVec);
};


#endif