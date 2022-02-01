#include "../interface/Uncertainty.h"

Uncertainty::Uncertainty(std::string& name, bool flat, bool envelope, bool corrProcess, bool eraSpec, std::vector<TString>& processes, TFile* outputfile) : 
    name(name), flat(flat), envelope(envelope), correlatedAmongProcesses(corrProcess), eraSpecific(eraSpec), relevantProcesses(processes), outfile(outputfile) {
    outputName = name;
    nameUp = name + "Up";
    nameDown = name + "Down";
}


TH1D* Uncertainty::getUncertainty(Histogram* histogram, Process* head, std::vector<TH1D*>& histVec) {
    if (flat) {
        return getFlatUncertainty(histogram, head, histVec);
    } else {
        return getShapeUncertainty(histogram, head, histVec);
    }
}


TH1D* Uncertainty::getShapeUncertainty(Histogram* histogram, Process* head, std::vector<TH1D*>& histVec) {
    // Loop processes, ask to add stuff

    // create one final TH1D to return
    Process* current = head;
    TH1D* ret = new TH1D(*histVec[0]);

    for (int bin = 1; bin < ret->GetNbinsX() + 1; bin++) {
        ret->SetBinContent(bin, 0.);
    }

    TString histogramID = histogram->getID();
    ret->SetName(histogramID + name);
    ret->SetTitle(name.c_str());

    //correlated case : linearly add up and down variations
    std::vector<double> varDown(histVec[0]->GetNbinsX(), 0.);
    std::vector<double> varUp(histVec[0]->GetNbinsX(), 0.);

    //uncorrelated case : quadratically add the maximum of the up and down variations
    std::vector<double> var(histVec[0]->GetNbinsX(), 0.);

    int histCount = 0;
    int procCount = 0;

    if (histogram->getPrintToFile()) {
        outfile->cd(histogram->getCleanName().c_str());
        gDirectory->mkdir(nameUp);
        gDirectory->mkdir(nameDown);
    }

    std::string up = "Up";
    std::string down = "Down";


    while (current) {
        // TODO: check if uncertainty needs this process, otherwise continue and put stuff to next one;
        if (current->getName() != relevantProcesses[procCount]) {
            histCount++;
            current = current->getNext();
        }

        TH1D* histNominal = histVec[histCount];
        TH1D* histUp = current->getHistogramUncertainty(name, up, histogram, outputName, isEnvelope());
        TH1D* histDown = current->getHistogramUncertainty(name, down, histogram, outputName, isEnvelope());

        if (histUp == nullptr && histDown == nullptr) {
            current = current->getNext();
            histCount++;
            procCount++;
            continue;
        }
        // do stuff
        // anyway
        for (int bin = 1; bin < histNominal->GetNbinsX() + 1; bin++) {

            double nominalContent = histNominal->GetBinContent( bin );
            double upVariedContent = histUp->GetBinContent( bin );
            double downVariedContent = histDown->GetBinContent( bin );
            double down = fabs( downVariedContent - nominalContent );
            double up = fabs( upVariedContent - nominalContent );

            //uncorrelated case : 
            if(! correlatedAmongProcesses ){
                double variation = std::max( down, up );
                var[bin - 1] += variation*variation;
            
            //correlated case :     
            } else {
                varDown[bin - 1] += down;
                varUp[bin - 1] += up;
            }

        }

        current = current->getNext();
        histCount++;
        procCount++;
    } 

    //std::cout << "bin content in Uncertainty:\t";

    for (int bin = 1; bin < histVec[0]->GetNbinsX() + 1; bin++) {
        //correlated case :
        if(correlatedAmongProcesses ){
            double varLocal = std::max( varDown[bin - 1], varUp[bin - 1] );
            var[bin - 1] = varLocal * varLocal;
        }

        ret->SetBinContent(bin, var[bin - 1]);
        //std::cout << ret->GetBinContent(bin) << "\t";
    }
    //std::cout << std::endl;
    // writeout uncertainty

    return ret;
    
    // set return value
}

void Uncertainty::printOutShapeUncertainty(Histogram* histogram, Process* head) {
    // Loop processes, ask to add stuff
    Process* current = head;
    TString histogramID = histogram->getID();

    int histCount = 0;
    int procCount = 0;

    TString outputNameUp = outputName + "Up";
    TString outputNameDown = outputName + "Down";

    if (histogram->getPrintToFile()) {
        outfile->cd(histogram->getCleanName().c_str());
        gDirectory->mkdir(outputNameUp);
        gDirectory->mkdir(outputNameDown);
    }

    std::string up = "Up";
    std::string down = "Down";

    while (current) {
        // TODO: check if uncertainty needs this process, otherwise continue and put stuff to next one;
        if (relevantProcesses.size() == 0) return;
        if (current->getName() != relevantProcesses[procCount]) {
            histCount++;
            current = current->getNext();
        }
        current->getHistogramUncertainty(name, up, histogram, outputName, isEnvelope());
        current->getHistogramUncertainty(name, down, histogram, outputName, isEnvelope());

        current = current->getNext();

        //if (! correlatedAmongProcesses) {
        //    relevantProcesses.erase(relevantProcesses.begin() + procCount);
        //    return;
        //}

        histCount++;
        procCount++;
    } 
}

TH1D* Uncertainty::getFlatUncertainty(Histogram* histogram, Process* head, std::vector<TH1D*>& histVec) {
    Process* current = head;
    //int histCount = 0;

    TH1D* ret = new TH1D(*histVec[0]);
    TString histogramID = histogram->getID();
    ret->SetName(histogramID + name);
    ret->SetTitle(histogramID + name);

    std::vector<double> var(histVec[0]->GetNbinsX(), 0.);
    int procCount = 0;

    for (unsigned histCount = 0; histCount < histVec.size(); histCount++) {
        if (current->getName() != relevantProcesses[procCount]) {
            current = current->getNext();
            continue;
        }

        for (int bin = 1; bin < histVec[0]->GetNbinsX() + 1; bin++) {
            double variation = 0.;
            if (correlatedAmongProcesses) {
                double binContent = histVec[histCount]->GetBinContent( bin );
                variation = binContent * (flatUncertainty - 1.);
                var[bin - 1] += variation;
            } else {
                variation = histVec[histCount]->GetBinContent( bin )*( flatUncertainty - 1. );
                var[bin - 1] = variation * variation;
            }
        }

        current = current->getNext();
        procCount++;
    }

    for (int bin = 1; bin < histVec[0]->GetNbinsX() + 1; bin++) {
        //correlated case :
        
        if (correlatedAmongProcesses) {
            ret->SetBinContent(bin, var[bin - 1] * var[bin - 1]);
            //std::cout << "bin: " << bin << " error " << var[bin - 1] << std::endl;
        } else {
            ret->SetBinContent(bin, var[bin - 1]);
            //std::cout << "bin: " << bin << " error " << sqrt(var[bin - 1]) << std::endl;
        }
    }

    return ret;
}

std::pair<TH1D*, TH1D*> Uncertainty::getUpAndDownShapeUncertainty(Histogram* histogram, Process* head) {
    // Loop processes, ask to add stuff
    Process* current = head;
    TString histogramID = histogram->getID();

    int histCount = 0;
    int procCount = 0;

    TString outputNameUp = outputName + "Up";
    TString outputNameDown = outputName + "Down";

    if (histogram->getPrintToFile()) {
        outfile->cd(histogram->getCleanName().c_str());
        gDirectory->mkdir(outputNameUp);
        gDirectory->mkdir(outputNameDown);
    }

    std::string up = "Up";
    std::string down = "Down";

    TH1D* upVarReturn = nullptr;
    TH1D* downVarReturn = nullptr;


    while (current) {
        // TODO: check if uncertainty needs this process, otherwise continue and put stuff to next one;
        if (current->getName() != relevantProcesses[procCount]) {
            histCount++;
            current = current->getNext();
        }
        TH1D* upVar = current->getHistogramUncertainty(name, up, histogram, outputName, isEnvelope());
        TH1D* downVar = current->getHistogramUncertainty(name, down, histogram, outputName, isEnvelope());

        if (upVarReturn == nullptr) {
            upVarReturn = new TH1D(*upVar);
            downVarReturn = new TH1D(*downVar);

            if(! correlatedAmongProcesses ){
                upVarReturn->Multiply(upVarReturn);
                downVarReturn->Multiply(downVarReturn);
            }
        } else {
            if(! correlatedAmongProcesses ){
                upVar->Multiply(upVar);
                downVar->Multiply(downVar);

                upVarReturn->Add(upVar);
                downVarReturn->Add(downVar);
            } else {
                upVarReturn->Add(upVar);
                downVarReturn->Add(downVar);
            }
        }

        current = current->getNext();

        histCount++;
        procCount++;
    }

    return {upVarReturn, downVarReturn};
}