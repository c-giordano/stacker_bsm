#include "../interface/ProcessList.h"
#include <iomanip>

Process* ProcessList::addProcess(TString& name, int color, TFile* inputfile, TFile* outputfile, bool signal, bool data, bool oldStuff) {
    // TODO: Create new process object
    allProcessNames.push_back(name);
    Process* brandNewObj = new Process(name, color, inputfile, outputfile, signal, data, oldStuff);

    if (tail) {
        brandNewObj->setPrev(tail);
        tail->setNext(brandNewObj); // check if tail already exists
    }
    tail = brandNewObj;

    if (! head) {
        head = brandNewObj;
    }

    return brandNewObj;
}

Process* ProcessList::addProcess(TString& name, int color, std::vector<TFile*>& inputfiles, TFile* outputfile, bool signal, bool data, bool oldStuff) {
    // TODO: Create new process object
    allProcessNames.push_back(name);
    Process* brandNewObj = new Process(name, color, inputfiles, outputfile, signal, data, oldStuff);

    if (tail) {
        brandNewObj->setPrev(tail);
        tail->setNext(brandNewObj); // check if tail already exists
    }
    tail = brandNewObj;

    if (! head) {
        head = brandNewObj;
    }

    return brandNewObj;
}

Process* ProcessList::addProcess(TString& name, std::vector<TString>& procNames, int color, TFile* inputfile, TFile* outputfile, bool signal, bool data, bool oldStuff) {
    // TODO: Create new process object
    allProcessNames.push_back(name);
    Process* brandNewObj = new ProcessSet(name, procNames, color, inputfile, outputfile, signal, data, oldStuff);

    if (tail) {
        brandNewObj->setPrev(tail);
        tail->setNext(brandNewObj); // check if tail already exists
    }
    tail = brandNewObj;

    if (! head) {
        head = brandNewObj;
    }

    return brandNewObj;
}

Process* ProcessList::addProcess(TString& name, std::vector<TString>& procNames, int color, std::vector<TFile*>& inputfiles, TFile* outputfile, bool signal, bool data, bool oldStuff) {
    // TODO: Create new process object
    allProcessNames.push_back(name);
    Process* brandNewObj = new ProcessSet(name, procNames, color, inputfiles, outputfile, signal, data, oldStuff);

    if (tail) {
        brandNewObj->setPrev(tail);
        tail->setNext(brandNewObj); // check if tail already exists
    }
    tail = brandNewObj;

    if (! head) {
        head = brandNewObj;
    }
    return brandNewObj;
}

Uncertainty* ProcessList::addUncertainty(std::string& name, bool flat, bool envelope, bool corrProcess, bool eraSpec, std::vector<TString>& processes, TFile* outputfile) {
    Uncertainty* brandNewObj = new Uncertainty(name, flat, envelope, corrProcess, eraSpec, processes, outputfile);

    if (tailUnc) tailUnc->setNext(brandNewObj); // check if tail already exists
    tailUnc = brandNewObj;

    if (! headUnc) {
        headUnc = brandNewObj;
    }

    return brandNewObj;
}


ProcessList::~ProcessList() {
    Process* toDel = head;
    while (toDel->getNext()) {
        Process* nextToDel = toDel->getNext();
        delete toDel;
        toDel = nextToDel;
    }
}

std::vector<TH1D*> ProcessList::fillStack(THStack* stack, Histogram* hist, TLegend* legend, TFile* outfile, std::vector<TH1D*>* signalHistograms, TH1D** sysUnc) {
    Process* current = head;
    std::vector<TH1D*> histVec;

    TString histogramID = hist->getID();

    double signalYield = 0.;
    double bkgYield = 0.;
    if (hist->getPrintToFile()) outfile->mkdir(hist->getCleanName().c_str());

    if (verbose) std::cout << histogramID << std::endl;

    while (current) {
        TH1D* histToAdd = current->getHistogram(hist);
        if (histToAdd == nullptr) {
            current = current->getNext();
            std::cerr << "nullptr returned in ProcessList::fillStack. Quitting..." << std::endl;
            exit(1);
            //continue; // seems strange to not continue
        }
        if (current->IsChannelIgnored(hist->GetChannel())) {
            //std::cout << current->getName() << " ignored" << std::endl;
            for (int j=1; j < histToAdd->GetNbinsX() + 1; j++) {
                histToAdd->SetBinContent(j, 0.00001);
                histToAdd->SetBinError(j, 0.00001);
            }
            histVec.push_back(histToAdd);
            current = current->getNext();
            continue;
        }
        legend->AddEntry(histToAdd, current->getCleanedName());
        stack->Add(histToAdd);
        histVec.push_back(histToAdd);
        
        if (current->isSignalProcess()) {
            signalYield += histToAdd->Integral();
            TH1D* signalHist = new TH1D(*histToAdd);
            signalHistograms->push_back(signalHist);
        } else {
            bkgYield += histToAdd->Integral();
        }

        if (verbose) {
            std::cout << current->getName();
            if (veryVerbose) {
                std::cout << " & ";
                for (int i=1; i < histToAdd->GetNbinsX() + 1; i++) {
                    std::cout << std::setprecision(5) << histToAdd->GetBinContent(i) << " & ";
                }
                std::cout << std::endl;
            } else {
                std::cout << ": " << histToAdd->Integral() << " events" << std::endl;
            }
        }

        for (int j=1; j < histToAdd->GetNbinsX() + 1; j++) {
            if (histToAdd->GetBinContent(j) <= 0.) {
                histToAdd->SetBinContent(j, 0.00001);
                histToAdd->SetBinError(j, 0.00001);
            }
        }

        if (hist->getPrintToFile()) {
            outfile->cd(hist->getCleanName().c_str());
            //for (int j=1; j < histToAdd->GetNbinsX() + 1; j++) {
            //    if (histToAdd->GetBinContent(j) <= 0.) {
            //        histToAdd->SetBinContent(j, 0.00001);
            //        histToAdd->SetBinError(j, 0.00001);
            //    }
            //}
            histToAdd->Write(current->getName(), TObject::kOverwrite);
        }
        current = current->getNext();
    }
    
    if (hist->getPrintToFile()) {
        TH1D* allHistograms = sumVector(histVec);
        allHistograms->SetName("data_obs");
        allHistograms->SetTitle("data_obs");
        outfile->cd(hist->getCleanName().c_str());
        for (int j=1; j<allHistograms->GetNbinsX() + 1; j++) {
            allHistograms->SetBinError(j, sqrt(allHistograms->GetBinContent(j)));
        }
        allHistograms->Write("data_obs", TObject::kOverwrite);
    }
    
    // loop uncertainties as well if required
    Uncertainty* currUnc = headUnc;
    std::vector<TH1D*> uncVec;
    while (currUnc && hist->getDrawUncertainties() && sysUnc) {
        // getShapeUncertainty or apply flat uncertainty
        TH1D* newUncertainty = currUnc->getUncertainty(hist, head, histVec);

        uncVec.push_back(newUncertainty);

        if (*sysUnc == nullptr) {
            *sysUnc = new TH1D(*newUncertainty);
        } else {
            (*sysUnc)->Add(newUncertainty);
        }

        currUnc = currUnc->getNext();
    }
    
    if (verbose) std::cout << "S/B = " << signalYield << "/" << bkgYield << std::endl;

    if (veryVerbose) {
        std::cout << " & Signal & Bkg & S/B\\\\" << std::endl;
        for (int i=1; i < histVec[0]->GetNbinsX() + 1; i++) {
            double sig = 0.;
            double all = 0.;
            for (unsigned j=0; j < signalHistograms->size(); j++) sig += signalHistograms->at(j)->GetBinContent(i);
            for (unsigned j=0; j < histVec.size(); j++) all += histVec[j]->GetBinContent(i);
            
            std::cout << " & " << std::fixed << std::setprecision(2) << sig << " & " << all - sig << " & " << sig / (all - sig) << "\\\\" << std::endl;
        }
        //std::cout << std::endl;
    }

    return histVec;
}

std::map<TString, bool> ProcessList::printHistograms(Histogram* hist, TFile* outfile, bool isData, Process* dataProc) {
    Process* current = head;
    std::vector<TH1D*> histVec;
    std::map<TString, bool> output;

    TString histogramID = hist->getID();

    if (hist->getPrintToFile()) outfile->mkdir(hist->getCleanName().c_str());

    if (verbose) std::cout << histogramID << std::endl;
    
    //std::cout << "channel " << hist->GetChannel() << std::endl;
    while (current) {

        TH1D* histToAdd = current->getHistogram(hist);
        if (histToAdd == nullptr) {
            current = current->getNext();
        }
        histVec.push_back(histToAdd);

        output[current->getName()] = (histToAdd->Integral() > 0 && ! current->IsChannelIgnored(hist->GetChannel())); // easily add "&& !current->ignoreRegion(hist->region())"
        // std::cout << "Process " << current->getName() << " " << (histToAdd->Integral() > 0) << (! current->IsChannelIgnored(hist->GetChannel())) << std::endl;
        if (verbose) {
            std::cout << current->getName();
            if (veryVerbose) {
                std::cout << " & ";
                for (int i=1; i < histToAdd->GetNbinsX() + 1; i++) {
                    std::cout << histToAdd->GetBinContent(i) << " & ";
                }
                std::cout << std::endl;
            } else {
                std::cout << ": " << histToAdd->Integral() << " events" << std::endl;
            }
        }

        if (hist->getPrintToFile()) {
            outfile->cd(hist->getCleanName().c_str());
            for (int j=1; j < histToAdd->GetNbinsX() + 1; j++) {
                if (histToAdd->GetBinContent(j) <= 0.) {
                    histToAdd->SetBinContent(j, 0.00001);
                    histToAdd->SetBinError(j, 0.00001);
                }
                //if (histToAdd->GetBinContent(j) - histToAdd->GetBinError(j) <= 0.) {
                //    histToAdd->SetBinError(j, histToAdd->GetBinContent(j));
                //}
            }
            if (hist->HasCustomAxisRange()) {
                histToAdd = rebin(histToAdd, hist->GetCustomNBins(), hist->GetCustomAxisRange().first, hist->GetCustomAxisRange().second);
            }
            histToAdd->Write(current->getName(), TObject::kOverwrite);
        }
        current = current->getNext();
    }
    //std::cout <<" done printing channel. Isdata: " << isData << std::endl;
    if (! isData && hist->getPrintToFile()) {
        TH1D* allHistograms = sumVector(histVec);
        allHistograms->SetName("data_obs");
        allHistograms->SetTitle("data_obs");
        outfile->cd(hist->getCleanName().c_str());
        for (int j=1; j<allHistograms->GetNbinsX() + 1; j++) {
            allHistograms->SetBinError(j, sqrt(allHistograms->GetBinContent(j)));
        }
        if (hist->HasCustomAxisRange()) {
            allHistograms = rebin(allHistograms, hist->GetCustomNBins(), hist->GetCustomAxisRange().first, hist->GetCustomAxisRange().second);
        }
        allHistograms->Write("data_obs", TObject::kOverwrite);
    } else if (isData && hist->getPrintToFile()) {
        TH1D* data = dataProc->getHistogram(hist);
        data->SetName("data_obs");
        data->SetTitle("data_obs");
        outfile->cd(hist->getCleanName().c_str());
        if (hist->HasCustomAxisRange()) {
            data = rebin(data, hist->GetCustomNBins(), hist->GetCustomAxisRange().first, hist->GetCustomAxisRange().second);
        }
        data->Write("data_obs", TObject::kOverwrite);
    }

    return output;
}


std::vector<TH2D*> ProcessList::fill2DStack(THStack* stack, TString& histogramID, TLegend* legend) {
    Process* current = head;
    std::vector<TH2D*> histVec;

    double signalYield = 0.;
    double bkgYield = 0.;

    //if (hist->getPrintToFile()) outfile->mkdir(histogramID);

    if (verbose) std::cout << histogramID << std::endl;

    while (current) {
        TH2D* histToAdd = current->get2DHistogram(histogramID);
        histToAdd->Scale(1./histToAdd->Integral());
        histToAdd->RebinX(2);
        histToAdd->RebinY(2);
        if (current->getName() == "TTB") histToAdd->Scale(1.3);
        legend->AddEntry(histToAdd, current->getCleanedName());
        stack->Add(histToAdd);
        histVec.push_back(histToAdd);
        
        if (current->isSignalProcess()) {
            signalYield += histToAdd->Integral();
        } else {
            bkgYield += histToAdd->Integral();
        }

        if (verbose) {
            std::cout << current->getName() << ": " << histToAdd->Integral() << " events" << std::endl;
        }

        //if (hist->getPrintToFile()) outfile->cd(histogramID);
        //if (hist->getPrintToFile()) histToAdd->Write(current->getName(), TObject::kOverwrite);

        current = current->getNext();
    }
    
    if (verbose) std::cout << "S/B = " << signalYield << "/" << bkgYield << std::endl;

    return histVec;
}

std::map<std::string, std::pair<TH1D*, TH1D*>> ProcessList::UpAndDownHistograms(Histogram* hist, std::vector<TH1D*>& nominalHists) {
    // loop uncertainties as well if required
    Uncertainty* currUnc = headUnc;
    std::map<std::string, std::pair<TH1D*, TH1D*>> returnValue;
    while (currUnc && hist->getDrawUncertainties()) {
        if (currUnc->isFlat()) {
            currUnc = currUnc->getNext();
            continue;
        }
        // getShapeUncertainty or apply flat uncertainty
        std::pair<TH1D*, TH1D*> newUncertainty = currUnc->getUpAndDownShapeUncertainty(hist, head, nominalHists);
        //newUncertainty.first->GetBinContent(1);
        //std::cout << currUnc->getName() << std::endl;
        returnValue[currUnc->getName()] = newUncertainty;

        currUnc = currUnc->getNext();
    }

    return returnValue;
}

std::vector<TH1D*> ProcessList::CreateHistogramAllProcesses(Histogram* hist) {
    Process* current = getHead();
    std::vector<TH1D*> histVec;

    TString histogramID = hist->getID();

    while (current) {
        TH1D* histToAdd = current->getHistogram(hist);
        for (int j=1; j < histToAdd->GetNbinsX() + 1; j++) {
            if (histToAdd->GetBinContent(j) <= 0.) {
                histToAdd->SetBinContent(j, 0.00001);
                histToAdd->SetBinError(j, 0.00001);
            }
        }
        if (histToAdd == nullptr) {
            current = current->getNext();
        }
        histVec.push_back(histToAdd);

        current = current->getNext();
    }

    return histVec;
}
