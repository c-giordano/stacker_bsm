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
    } else if (envelope && buildEnvelope) {
        return getEnvelope(histogram, head, histVec);
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

    //std::cout << outputName << std::endl;
    //std::cout << name << std::endl;
    //std::cout << "uncertainty " << name << " for histogram " << histogramID.Data() << " histvec size " << histVec.size() << " " << relevantProcesses.size() << std::endl;

    while (current) {
        // TODO: check if uncertainty needs this process, otherwise continue and put stuff to next one;
        //std::cout << current->getName().Data() << std::endl;        
        //std::cout << "Process " << current->getName() << " " << (current->getName() != relevantProcesses[procCount]) << (current->IsChannelIgnored(histogram->GetChannel())) << std::endl;
        //std::cout << histCount << " " << procCount << std::endl;
        
        bool isIrrelevant = (current->getName() != relevantProcesses[procCount]);
        bool isIgnored = current->IsChannelIgnored(histogram->GetChannel());

        if (! isIrrelevant && isIgnored) {
            current = current->getNext();
            procCount++;
            histCount++;
            continue;
        } else if (isIrrelevant || isIgnored) {
            current = current->getNext();
            histCount++;
            continue;
        }

        TH1D* histNominal = histVec[histCount];
        TH1D* histUp = current->getHistogramUncertainty(name, up, histogram, isEnvelope());
        TH1D* histDown = current->getHistogramUncertainty(name, down, histogram, isEnvelope());

        if (histUp == nullptr && histDown == nullptr) {
            current = current->getNext();
            histCount++;
            procCount++;
            continue;
        }

        if (histogram->HasRebin()) {
            if (histogram->GetRebinVar() == nullptr) {
                histUp->Rebin(histogram->GetRebin());
                histDown->Rebin(histogram->GetRebin());
            } else {
                TString newNameUp = histUp->GetName();
                histUp->SetName(newNameUp + TString("OLD"));
                TString newNameDown = histDown->GetName();
                histDown->SetName(newNameDown + TString("OLD"));
                
                histUp = (TH1D*) histUp->Rebin(histogram->GetRebin()-1, newNameUp, histogram->GetRebinVar());
                histDown = (TH1D*) histDown->Rebin(histogram->GetRebin()-1, newNameDown, histogram->GetRebinVar());
            }
        }

        if (histogram->hasUniWidthBins()) {
            histUp = ApplyBinWidthUnification(histUp);
            histDown = ApplyBinWidthUnification(histDown);
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

TH1D* Uncertainty::getEnvelope(Histogram* histogram, Process* head, std::vector<TH1D*>& histVec) {
    // combine stuff of different variations here
    // use normal envelopes

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
        //std::cout << current->getName().Data() << " " << relevantProcesses[procCount] << std::endl;
        bool isIrrelevant = (current->getName() != relevantProcesses[procCount]);
        bool isIgnored = current->IsChannelIgnored(histogram->GetChannel());

        if (! isIrrelevant && isIgnored) {
            current = current->getNext();
            procCount++;
            histCount++;
            continue;
        } else if (isIrrelevant || isIgnored) {
            current = current->getNext();
            histCount++;
            continue;
        }

        TH1D* histNominal = histVec[histCount];
        std::pair<TH1D*, TH1D*> histVars;

        if (name == "pdfShapeVar") {
            // alternative code is still available for building correct envelope here. Should in principle never be used anymore
            histVars = buildPDFFromSumSquaredCollections(histogram, current, histNominal);
        } else {
            histVars = buildEnvelopeForProcess(histogram, current, histNominal);
        }

        TH1D* histUp = histVars.first;
        TH1D* histDown = histVars.second;

        if (histUp == nullptr && histDown == nullptr) {
            current = current->getNext();
            histCount++;
            procCount++;
            continue;
        }


        if (histogram->getPrintToFile()) {
            for (int j=1; j < histUp->GetNbinsX() + 1; j++) {
                if (histUp->GetBinContent(j) <= 0.) histUp->SetBinContent(j, 0.00001);
                if (histDown->GetBinContent(j) <= 0.) histDown->SetBinContent(j, 0.00001);
            }

            TFile* outputFile = current->GetOutputFile();
            outputFile->cd();
            outputFile->cd((histogram->getCleanName()).c_str());

            gDirectory->cd((outputName + "Up").c_str());
            histUp->Write(current->getName());

            gDirectory->cd((outputName + "Down").c_str());
            histDown->Write(current->getName());
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

    return ret;
    
    // set return value

}

std::pair<TH1D*, TH1D*> Uncertainty::buildEnvelopeForProcess(Histogram* histogram, Process* currentProcess, TH1D* nominalHist) {
    // call this for each histogram (comes from earlier call. So only given histogram is relevant)
    // loop per process
    // get all n variations of this histogram for a given process, create each bincontent. 
    // postprocess for given process
    // should only be qcd uncertainty 
    TH1D* upVar = new TH1D(*nominalHist);
    TH1D* downVar = new TH1D(*nominalHist);

    upVar->SetName(histogram->getID() + currentProcess->getName() + TString(name + "Up"));
    upVar->SetTitle(histogram->getID() + currentProcess->getName() + TString(name + "Up"));

    downVar->SetName(histogram->getID() + currentProcess->getName() + TString(name + "Down"));
    downVar->SetTitle(histogram->getID() + currentProcess->getName() + TString(name + "Down"));

    unsigned variations = 6;

    std::vector<std::shared_ptr<TH1D>> allHistogramVariations = currentProcess->GetAllVariations(histogram, variations, name);

    for (auto currentVariation : allHistogramVariations) {
        // loop all possible up and down variations
        // get histograms for current variation
        // combine in correct way

        if (histogram->HasRebin()) {
            if (histogram->GetRebinVar() == nullptr) {
                currentVariation->Rebin(histogram->GetRebin());
            } else {
                std::string newName = currentVariation->GetName();
                currentVariation->SetName((newName + "OLD").c_str());
                currentVariation = std::make_shared<TH1D>(*(TH1D*) currentVariation->Rebin(histogram->GetRebin()-1, newName.c_str(), histogram->GetRebinVar()));
            }
        }
        if (histogram->hasUniWidthBins()) {
            currentVariation = std::make_shared<TH1D>(*(TH1D*) ApplyBinWidthUnification(currentVariation.get()));
        }

        for(int j=1; j < currentVariation->GetNbinsX()+1; j++){

            // for each up and down variation, we fix the content
            double bincontentCurrent = currentVariation->GetBinContent(j);

            if (bincontentCurrent > upVar->GetBinContent(j) ){
                upVar->SetBinContent(j, bincontentCurrent);
            } else if (bincontentCurrent < downVar->GetBinContent(j) ){
                downVar->SetBinContent(j, bincontentCurrent);
            }
        }
    }

    return {upVar, downVar};
}

std::pair<TH1D*, TH1D*> Uncertainty::buildPDFFromSumSquaredCollections(Histogram* histogram, Process* currentProcess, TH1D* nominalHist) {
    // call this for each histogram (comes from earlier call. So only given histogram is relevant)
    // loop per process
    // get all n variations of this histogram for a given process, create each bincontent. 
    // postprocess for given process

    TH1D* upVar = new TH1D(*nominalHist);
    upVar->SetName(histogram->getID() + currentProcess->getName() + TString(name + "Up"));
    upVar->SetTitle(histogram->getID() + currentProcess->getName() + TString(name + "Up"));

    TH1D* downVar = new TH1D(*nominalHist);
    downVar->SetName(histogram->getID() + currentProcess->getName() + TString(name + "Down"));
    downVar->SetTitle(histogram->getID() + currentProcess->getName() + TString(name + "Down"));

    std::string up = "Up";
    TH1D* totalVar = currentProcess->getHistogramUncertainty(name, up, histogram, isEnvelope());

    // get all histograms in up variation -> Should automatically sum in process

    for(int j=1; j < totalVar->GetNbinsX()+1; j++){
        // for each up and down variation, we fix the content
        totalVar->SetBinContent(j, sqrt(totalVar->GetBinContent(j)));
    }

    if (histogram->HasRebin()) {
        if (histogram->GetRebinVar() == nullptr) {
            totalVar->Rebin(histogram->GetRebin());
        } else {
            std::string newName = std::string(histogram->getID() + name + "Complete");
            totalVar = (TH1D*) totalVar->Rebin(histogram->GetRebin()-1, newName.c_str(), histogram->GetRebinVar());
        }
    }
    if (histogram->hasUniWidthBins()) {
        totalVar = ApplyBinWidthUnification(totalVar);
    }
    
    
    upVar->Add(totalVar);
    downVar->Add(totalVar, -1.);

    return {upVar, downVar};
}

std::pair<TH1D*, TH1D*> Uncertainty::buildSumSquaredEnvelopeForProcess(Histogram* histogram, Process* currentProcess, TH1D* nominalHist) {
    // call this for each histogram (comes from earlier call. So only given histogram is relevant)
    // loop per process
    // get all n variations of this histogram for a given process, create each bincontent. 
    // postprocess for given process

    TH1D* totalVar = new TH1D(*nominalHist);
    TH1D* upVar = new TH1D(*nominalHist);
    TH1D* downVar = new TH1D(*nominalHist);

    // make sure they are empty
    totalVar->Reset();

    unsigned variations = 100;

    std::vector<std::shared_ptr<TH1D>> allHistogramVariations = currentProcess->GetAllVariations(histogram, variations, name);

    for (auto currentVariation : allHistogramVariations) {
        // loop all possible up and down variations
        // get histograms for current variation
        // combine in correct way

        // for each contribution, take diff with nominal and square
        for(int j=1; j < currentVariation->GetNbinsX()+1; j++){
            // for each up and down variation, we fix the content
            double bincontentCurrent = currentVariation->GetBinContent(j) - nominalHist->GetBinContent(j);
            totalVar->SetBinContent(j, totalVar->GetBinContent(j) + bincontentCurrent * bincontentCurrent);
        }
    }

    for(int j=1; j < totalVar->GetNbinsX()+1; j++){
        // for each up and down variation, we fix the content
        totalVar->SetBinContent(j, sqrt(totalVar->GetBinContent(j)));
    }
    
    upVar->Add(totalVar);
    downVar->Add(totalVar, -1.);

    return {upVar, downVar};
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

        //TH1D* upVar = nullptr;
        //TH1D* downVar = nullptr;
        //TH1D* histNominal = nominalHists[histCount];
        ////current->getHistogramUncertainty(name, up, histogram, outputName, isEnvelope());
        ////current->getHistogramUncertainty(name, down, histogram, outputName, isEnvelope());
//
        //if (! envelope || (envelope && ! buildEnvelope)) {
        //    upVar = current->getHistogramUncertainty(name, up, histogram, outputName, isEnvelope());
        //    downVar = current->getHistogramUncertainty(name, down, histogram, outputName, isEnvelope());
//
        //    if (histogram->HasRebin()) {
        //        if (histogram->GetRebinVar() == nullptr) {
        //            upVar->Rebin(histogram->GetRebin());
        //            downVar->Rebin(histogram->GetRebin());
        //        } else {
        //            TString newNameUp = upVar->GetName();
        //            upVar->SetName(newNameUp + TString("OLD"));
        //            TString newNameDown = downVar->GetName();
        //            downVar->SetName(newNameDown + TString("OLD"));
        //            
        //            upVar = (TH1D*) upVar->Rebin(histogram->GetRebin(), newNameUp, histogram->GetRebinVar());
        //            downVar = (TH1D*) downVar->Rebin(histogram->GetRebin(), newNameDown, histogram->GetRebinVar());
        //        }
        //    }
        //} else if (name == "pdfShapeVar") {
        //    std::pair<TH1D*, TH1D*> histVars = buildPDFFromSumSquaredCollections(histogram, current, histNominal);
        //    upVar = histVars.first;
        //    downVar = histVars.second;
        //} else {
        //    std::pair<TH1D*, TH1D*> histVars = buildEnvelopeForProcess(histogram, current, histNominal);
        //    upVar = histVars.first;
        //    downVar = histVars.second;
        //}


        current = current->getNext();

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
    int histCount = 0;
    int procCount = 0;

    //for (unsigned histCount = 0; histCount < histVec.size(); histCount++) {
    //std::cout << "uncertainty " << name << " for histogram " << histogramID.Data() << " histvec size " << histVec.size() << " " << relevantProcesses.size() << std::endl;
    while (current) {
        //std::cout << "Process " << current->getName() << " " << (current->getName() != relevantProcesses[procCount]) << (current->IsChannelIgnored(histogram->GetChannel())) << std::endl;
        //std::cout << histCount << " " << procCount << std::endl;

        bool isIrrelevant = (current->getName() != relevantProcesses[procCount]);
        bool isIgnored = current->IsChannelIgnored(histogram->GetChannel());

        if (! isIrrelevant && isIgnored) {
            current = current->getNext();
            procCount++;
            histCount++;
            continue;
        } else if (isIrrelevant || isIgnored) {
            current = current->getNext();
            histCount++;
            continue;
        }

        //std::cout << correlatedAmongProcesses << std::endl;
        //std::cout << histVec[histCount]->GetBinContent(1) << std::endl;
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
        histCount++;
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

std::pair<TH1D*, TH1D*> Uncertainty::getUpAndDownShapeUncertainty(Histogram* histogram, Process* head, std::vector<TH1D*>& nominalHists, std::string era) {
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


    //std::cout << name << " in channel " << histogram->GetChannel() << std::endl;
//
    //std::cout << name << " rel processes: ";
    //for (auto el :relevantProcesses ) {
    //    std::cout << el.Data() << " ";
    //}
    //std::cout<< std::endl;
    
    while (current) {
        // TODO: check if uncertainty needs this process, otherwise continue and put stuff to next one;
        //std::cout << "Process " << current->getName() << " " << (current->getName() != relevantProcesses[procCount]) << (current->IsChannelIgnored(histogram->GetChannel())) << std::endl;
        if (current->getName() != relevantProcesses[procCount]) {
            histCount++;
            current = current->getNext();
            continue;
        } else if (current->IsChannelIgnored(histogram->GetChannel())) {
            histCount++;
            procCount++;
            current = current->getNext();
            continue;
        }

        TH1D* upVar = nullptr;
        TH1D* downVar = nullptr;
        TH1D* histNominal = nominalHists[histCount];
        if (! envelope || (envelope && (! buildEnvelope && ! indivudalPDFVariations))) {
            //std::cout << "process " << current->getName().Data() << " has " << current->isSet() << std::endl;
            upVar = current->getHistogramUncertainty(name, up, histogram, isEnvelope(), era);
            downVar = current->getHistogramUncertainty(name, down, histogram, isEnvelope(), era);

            if (histogram->HasRebin()) {
                if (histogram->GetRebinVar() == nullptr) {
                    upVar->Rebin(histogram->GetRebin());
                    downVar->Rebin(histogram->GetRebin());
                } else {
                    TString newNameUp = upVar->GetName();
                    upVar->SetName(newNameUp + TString("OLD"));
                    TString newNameDown = downVar->GetName();
                    downVar->SetName(newNameDown + TString("OLD"));
                    
                    upVar = (TH1D*) upVar->Rebin(histogram->GetRebin()-1, newNameUp, histogram->GetRebinVar());
                    downVar = (TH1D*) downVar->Rebin(histogram->GetRebin()-1, newNameDown, histogram->GetRebinVar());
                }
            }
            if (histogram->hasUniWidthBins()) {
                upVar = ApplyBinWidthUnification(upVar);
                downVar = ApplyBinWidthUnification(downVar);
            }
            //if (name == "ttvNJetsUnc_AddJets") {
            //    for (int j=1; j < upVar->GetNbinsX() + 1; j++) {
            //        double nom = histNominal->GetBinContent(j);
            //        downVar->SetBinContent(j, (2 * nom) - upVar->GetBinContent(j));
            //    }
            //}
        } else if (name == "pdfShapeVar") {
            if (! indivudalPDFVariations) {
                //std::cout << "building vars" << std::endl;
                std::pair<TH1D*, TH1D*> histVars = buildPDFFromSumSquaredCollections(histogram, current, histNominal);
                upVar = histVars.first;
                downVar = histVars.second;
            } else {
                writeIndividualPDFVariations(histogram, histNominal, current, 100);

                upVar = current->getHistogramUncertainty(name, up, histogram, isEnvelope(), era);
                downVar = current->getHistogramUncertainty(name, down, histogram, isEnvelope(), era);
            }
        } else if (name == "qcdScale") {
            if (! indivudalPDFVariations) {
                std::pair<TH1D*, TH1D*> histVars = buildEnvelopeForProcess(histogram, current, histNominal);
                upVar = histVars.first;
                downVar = histVars.second;
            } else {
                writeIndividualPDFVariations(histogram, histNominal, current, 6);

                upVar = current->getHistogramUncertainty(name, up, histogram, isEnvelope(), era);
                downVar = current->getHistogramUncertainty(name, down, histogram, isEnvelope(), era);
            }
        } else {
            std::pair<TH1D*, TH1D*> histVars = buildEnvelopeForProcess(histogram, current, histNominal);
            upVar = histVars.first;
            downVar = histVars.second;
        }

        if (histogram->getPrintToFile()) {
            outfile->cd();
            outfile->cd((histogram->getCleanName()).c_str());
            //std::cout << "process " << current->getName() << std::endl;
            for (int j=1; j < upVar->GetNbinsX() + 1; j++) {

                //std::cout << "bin " << j << ": " << histNominal->GetBinContent(j) << " " << upVar->GetBinContent(j) << " " << downVar->GetBinContent(j);
                if (histNominal->GetBinContent(j) <= 0.00005) {
                    upVar->SetBinContent(j, 0.00001);
                    upVar->SetBinError(j, 0.00001);
                    downVar->SetBinContent(j, 0.00001);
                    downVar->SetBinError(j, 0.00001);
                }

                if (histNominal->GetBinError(j) > histNominal->GetBinContent(j)) {
                    upVar->SetBinContent(j, histNominal->GetBinContent(j));
                    upVar->SetBinError(j, histNominal->GetBinError(j));
                    downVar->SetBinContent(j, histNominal->GetBinContent(j));
                    downVar->SetBinError(j, histNominal->GetBinError(j));
                }
                if (upVar->GetBinContent(j) <= 0.) {
                    upVar->SetBinContent(j, 0.00001);
                    upVar->SetBinError(j, 0.00001);
                }
                if (downVar->GetBinContent(j) <= 0.) {
                    downVar->SetBinContent(j, 0.00001);
                    downVar->SetBinError(j, 0.00001);
                }
                //std::cout << "\t after clean: " << upVar->GetBinContent(j) << " " << downVar->GetBinContent(j) << std::endl;
            }

            if (histogram->HasCustomAxisRange()) {
                //upVar->SetBins(hist->GetCustomNBins(), hist->GetCustomAxisRange().first, hist->GetCustomAxisRange().second);
                //downVar->SetBins(hist->GetCustomNBins(), hist->GetCustomAxisRange().first, hist->GetCustomAxisRange().second);
                upVar = rebin(upVar, histogram->GetCustomNBins(), histogram->GetCustomAxisRange().first, histogram->GetCustomAxisRange().second);
                downVar = rebin(downVar, histogram->GetCustomNBins(), histogram->GetCustomAxisRange().first, histogram->GetCustomAxisRange().second);
            }

            gDirectory->cd(outputNameUp);
            upVar->Write(current->getName());
            gDirectory->cd("..");

            gDirectory->cd(outputNameDown);
            downVar->Write(current->getName());
        }

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

void Uncertainty::writeIndividualPDFVariations(Histogram* histogram, TH1D* nominalHist, Process* current, int n) {
    std::vector<std::shared_ptr<TH1D>> allHistogramVariations = current->GetAllVariations(histogram, n, name);

    //std::cout << "writing variations for " << current->getName() << " with n = "  << allHistogramVariations.size() << std::endl; 

    for (unsigned i=0; i < allHistogramVariations.size(); i++) {
        TH1D* var = allHistogramVariations[i].get();

        if (histogram->HasRebin()) {
            if (histogram->GetRebinVar() == nullptr) {
                var->Rebin(histogram->GetRebin());
            } else {
                TString newNameUp = var->GetName();
                var->SetName(newNameUp + TString("OLD"));
                
                var = (TH1D*) var->Rebin(histogram->GetRebin()-1, newNameUp, histogram->GetRebinVar());
            }
        }
        if (histogram->hasUniWidthBins()) {
            var = ApplyBinWidthUnification(var);
        }
        if (histogram->getPrintToFile()) {
            outfile->cd();
            outfile->cd((histogram->getCleanName()).c_str());

            for (int j=1; j < var->GetNbinsX() + 1; j++) {
                if (nominalHist->GetBinError(j) > nominalHist->GetBinContent(j)) {
                    var->SetBinContent(j, nominalHist->GetBinContent(j));
                    var->SetBinError(j, nominalHist->GetBinError(j));
                }
                if (nominalHist->GetBinContent(j) <= 0.00005) {
                    var->SetBinContent(j, 0.00001);
                    var->SetBinError(j, 0.00001);
                }
                if (var->GetBinContent(j) <= 0.) {
                    var->SetBinError(j, 0.00001);
                    var->SetBinContent(j, 0.00001);
                }
            }

            TString outputNameUp = outputName + std::to_string(i) + "Up";
            TString outputNameDown = outputName + std::to_string(i) + "Down";
            if (! gDirectory->GetDirectory(outputNameUp)) {
                gDirectory->mkdir(outputNameUp);
                gDirectory->mkdir(outputNameDown);
            }
            if (histogram->HasCustomAxisRange()) {
                //upVar->SetBins(hist->GetCustomNBins(), hist->GetCustomAxisRange().first, hist->GetCustomAxisRange().second);
                //downVar->SetBins(hist->GetCustomNBins(), hist->GetCustomAxisRange().first, hist->GetCustomAxisRange().second);

                var = rebin(var, histogram->GetCustomNBins(), histogram->GetCustomAxisRange().first, histogram->GetCustomAxisRange().second);
                nominalHist = rebin(nominalHist, histogram->GetCustomNBins(), histogram->GetCustomAxisRange().first, histogram->GetCustomAxisRange().second);

            }

            gDirectory->cd(outputNameUp);
            var->Write(current->getName());
            gDirectory->cd("..");

            gDirectory->cd(outputNameDown);
            nominalHist->Write(current->getName());
        }
    }
}

bool Uncertainty::IsIgnoredChannel(std::string channel) {
    return std::find(ignoredChannels.begin(), ignoredChannels.end(), channel) != ignoredChannels.end();
}

