#ifndef UNCERTAINTY_H
#define UNCERTAINTY_H

#include <string>
#include <vector>

#include <TString.h>
#include <TH1.h>

#include "Process.h"
#include "Histogram.h"

class Uncertainty {
    private:
        std::string name;
        TString nameUp;
        TString nameDown;
        bool flat;
        bool correlatedAmongProcesses;
        bool eraSpecific;
        bool isBoth;

        double flatUncertainty; // value used in histograms
        double flatUncertaintyEra=0.0; // for datacards: this era of data
        double flatUncertaintyAll=0.0; // for datacards: all eras
        double flatUncertainty1718=0.0; // for datacards: all eras

        std::vector<TString> relevantProcesses;

        TFile* outfile;

        Uncertainty* next = nullptr;

    public:
        Uncertainty(std::string& name, bool flat, bool corrProcess, bool eraSpec, std::vector<TString>& processes, TFile* outputfile);
        ~Uncertainty();

        void setFlatRate(double rate) {flatUncertainty = rate;}
        
        void setFlatRateEra(double rate) {flatUncertaintyEra = rate;}
        void setFlatRateAll(double rate) {flatUncertaintyAll = rate;}
        void setFlatRate1718(double rate) {flatUncertainty1718 = rate;}
        void setEraSpec(bool newEraSpec) {eraSpecific = newEraSpec;}
        void setBoth(bool newBoth) {isBoth = newBoth;}

        void setCorrProcess(bool newCorr) {correlatedAmongProcesses = newCorr;}


        bool isFlat() {return flat;}

        Uncertainty* getNext() {return next;}
        void setNext(Uncertainty* newNext) {next = newNext;}

        TH1D* getShapeUncertainty(Histogram* histogram, Process* head, std::vector<TH1D*>& histVec);
        TH1D* getFlatUncertainty(Histogram* histogram, Process* head, std::vector<TH1D*>& histVec);
        TH1D* getUncertainty(Histogram* histogram, Process* head, std::vector<TH1D*>& histVec);

        std::string& getName() {return name;}
        std::vector<TString>& getRelevantProcesses() {return relevantProcesses;}
        
        bool isEraSpecific() {return eraSpecific;}
        bool isBothEraAndFull() {return isBoth;}
        bool is1718Specific() {return (flatUncertainty1718 != 0.);}
        double getFlatRateAll() const {return flatUncertaintyAll;}
        double getFlatRateEra() const {return flatUncertaintyEra;}
        double getFlatRate1718() const {return flatUncertainty1718;}

        bool getCorrelatedAmongProcesses() const {return correlatedAmongProcesses;}

};


#endif