#include "../interface/DatacardWriter.h"

DatacardWriter::DatacardWriter(std::string yearID, ProcessList* allProc, std::vector<Histogram*> histVec, TFile* outfile, Process* dataProcNew, std::string nameOverwrite) :
    yearID(yearID), allProc(allProc), allHistograms(histVec), outfile(outfile)
{   
    datacardName = "DC_" + yearID;
    std::cout << "outputfile name is " << outfile->GetName() << std::endl;
    if (nameOverwrite != "") {
        datacardName = "DC_" + nameOverwrite;
    }
    std::cout << datacardName << std::endl;
    if (dataProcNew != nullptr) {
        dataProc = dataProcNew;
        isData = true;
    }
    //initDatacard();
    
    bool era_16Pre = false;
    bool era_16Post = false;
    bool era_17 = false;
    bool era_18 = false;
    for (auto file : allProc->getHead()->GetInputfiles()) {
        std::string filename = file->GetName();

        if (stringContainsSubstr(filename, "2016Pre")) era_16Pre = true;
        if (stringContainsSubstr(filename, "2016Post")) era_16Post = true;
        if (stringContainsSubstr(filename, "2017")) era_17 = true;
        if (stringContainsSubstr(filename, "2018")) era_18 = true;
    }
    if (era_16Pre) relevantEras.push_back("2016PreVFP");
    if (era_16Post) relevantEras.push_back("2016PostVFP");
    if (era_17) relevantEras.push_back("2017");
    if (era_18) relevantEras.push_back("2018");
}


void DatacardWriter::initDatacard() {
    datacard.open("combineFiles/" + datacardName + ".txt");

    writeInit();
    writeShapeSource();
    writeDataObs();
    writeProcessHeader();

    for (auto it : allHistograms) {
        allHistogramTH1Ds.push_back(allProc->CreateHistogramAllProcesses(it));
    }
}

void DatacardWriter::writeEmptyLine(unsigned length) {
    for (unsigned i=0; i < length; i++) {
        datacard << "-";
    }

    datacard << std::endl;
}

void DatacardWriter::writeInit() {
    datacard << "imax " << allHistograms.size() << std::endl;
    datacard << "jmax *" << std::endl;
    datacard << "kmax *" << std::endl;

    writeEmptyLine(500);
}

void DatacardWriter::writeShapeSource() {
    // get shapes that are printed 
    // add option to histogram mss for which are printed?
    datacard << "shapes * * " << yearID << ".root";
    // combinefile name
    // folder structure
    datacard << " $CHANNEL/$PROCESS $CHANNEL/$SYSTEMATIC/$PROCESS" << std::endl;
    writeEmptyLine(500);

}

void DatacardWriter::writeDataObs() {
    datacard << std::setw(20) << "bin\t";
    std::stringstream rates;
    rates << std::setw(20) << "observation\t";
    for (unsigned i=0; i < allHistograms.size(); i++) {
        // loop histograms
        datacard << std::setw(15) << allHistograms[i]->getCleanName() << "\t";
        rates << std::setw(15) << "-1" << "\t";
    }
    datacard << std::endl;
    datacard << rates.str() << std::endl;
    // write out the histogram names again
    writeEmptyLine(500);
}

void DatacardWriter::writeProcessHeader() {
    datacard << std::setw(30) << "bin"<< "\t" << std::setw(15) << "\t";;

    // loop histograms, 
    std::stringstream processline;
    std::stringstream processNumbers;
    std::stringstream rates;

    processline << std::setw(30) << "process"<< "\t" << std::setw(15) << "\t";
    processNumbers << std::setw(30) << "process"<< "\t" << std::setw(15) << "\t";
    rates << std::setw(30) << "rate"<< "\t" << std::setw(15) << "\t";

    for (unsigned i=0; i < allHistograms.size(); i++) {
        // loop histograms
        std::map<TString, bool> relevance = allProc->printHistograms(allHistograms[i], outfile, isData, dataProc);
        allHistograms[i]->setRelevance(relevance);
        Process* proc = allProc->getTail();
        int j = -1;
        while (proc) {
            j++;

            if (! relevance[proc->getName()]) {
                proc = proc->getPrev();
                continue;
            }

            datacard << std::setw(15) << allHistograms[i]->getCleanName() << "\t";
            processline << std::setw(15) << proc->getName().Data() << "\t";
            processNumbers << std::setw(15) << j << "\t";
            rates << std::setw(15) << "-1" << "\t";

            proc = proc->getPrev();
        }
    }

    datacard << std::endl;
    datacard << processline.str() << std::endl;
    datacard << processNumbers.str() << std::endl;
    datacard << rates.str() << std::endl;
    writeEmptyLine(500);
}

bool DatacardWriter::containsProcess(std::vector<TString>& vector, TString& process) {
    for (unsigned i=0; i < vector.size(); i++) {
        if (vector[i] == process) return true;
    }
    return false;
}


void DatacardWriter::writeUncertainties(Uncertainty* uncertainty, bool eraSpecific) {
    // consider switching to just 4 bools for era or just naming it erawise
    std::cout << uncertainty->getName() << " " <<uncertainty->isEraSpecific() << eraSpecific << std::endl;
    if (uncertainty->isEraSpecific() && ! eraSpecific && uncertainty->isBothEraAndFull()) {
        writeUncertainties(uncertainty, true);
    } else if (! uncertainty->isBothEraAndFull() && uncertainty->isEraSpecific() && ! eraSpecific) {
        writeUncertainties(uncertainty, true);
        if (uncertainty->getNext() != nullptr) writeUncertainties(uncertainty->getNext());
        return;
    }
    if (uncertainty->is1718Specific() && ! eraSpecific) write1718Uncertainty(uncertainty);

    // write name
    std::string name = uncertainty->getName();

    std::vector<TString> relevantProcesses = uncertainty->getRelevantProcesses();

    unsigned k = 1;
    if (! uncertainty->getCorrelatedAmongProcesses()) k = relevantProcesses.size();

    bool isUnspecifiedEraSpecific = stringContainsSubstr(uncertainty->getName(), "201");
    for (unsigned j=0; j<k; j++) {        
        for (auto era : relevantEras) {
            if (isUnspecifiedEraSpecific) {
                if (stringContainsSubstr(uncertainty->getName(), "2016Pre")) era = "2016PreVFP";
                else if (stringContainsSubstr(uncertainty->getName(), "2016Post")) era = "2016PostVFP";
                else if (stringContainsSubstr(uncertainty->getName(), "2016")) era = "2016";
                else if (stringContainsSubstr(uncertainty->getName(), "2017")) era = "2017";
                else if (stringContainsSubstr(uncertainty->getName(), "2018")) era = "2018";
            }
            std::string tempName = name;

            if (k > 1) tempName += relevantProcesses[j].Data();
            if (eraSpecific) tempName += era;
            if (uncertainty->isIndivudalPDFVariations()) tempName += "0";

            datacard << std::setw(30) << tempName << "\t";
            double errorValue = 1.0;

            if (uncertainty->isFlat()) {
                datacard << std::setw(15) << "lnN" << "\t";
                if (eraSpecific) errorValue = uncertainty->getFlatRateEra();
                else errorValue = uncertainty->getFlatRateAll();
            } else {
                datacard << std::setw(15) << "shape" << "\t";
                if (! uncertainty->isIndivudalPDFVariations()) uncertainty->setOutputName(tempName);
                else uncertainty->setOutputName(name);
            }

            for (unsigned i=0; i < allHistograms.size(); i++) {
                if (! uncertainty->isFlat() && (eraSpecific || isUnspecifiedEraSpecific)) uncertainty->getUpAndDownShapeUncertainty(allHistograms[i], allProc->getHead(), allHistogramTH1Ds[i], era);
                else if (! uncertainty->isFlat()) uncertainty->getUpAndDownShapeUncertainty(allHistograms[i], allProc->getHead(), allHistogramTH1Ds[i]);
                
                std::stringstream interString;

                Process* proc = allProc->getTail();
                while (proc) {
                    TString currentName = proc->getName();
                    if (! allHistograms[i]->isRelevant(currentName)) {
                        proc = proc->getPrev();
                        continue;
                    }
                    if (! containsProcess(relevantProcesses, currentName)
                        || (k != 1 && currentName != relevantProcesses[j])) {
                        interString << std::setw(15) << "-" << "\t";
                        proc = proc->getPrev();
                        continue;
                    }
                    interString << std::setw(15) << std::setprecision(5) << errorValue << "\t";

                    proc = proc->getPrev();
                }

                // loop over number of histograms we consider
                datacard << interString.str();
            }

            if (uncertainty->isIndivudalPDFVariations()) {
                unsigned nVariations = 100;
                if (uncertainty->getName() == "qcdScale") nVariations = 6;
                for (unsigned countPDFs = 1; countPDFs < nVariations; countPDFs++) {
                    datacard << std::endl;

                    std::string tempNamePDF = name + std::to_string(countPDFs);
                    datacard << std::setw(30) << tempNamePDF << "\t";
                    datacard << std::setw(15) << "shape" << "\t";
                    
                    for (unsigned i=0; i < allHistograms.size(); i++) {
                        std::stringstream interString;

                        Process* proc = allProc->getTail();
                        while (proc) {
                            TString currentName = proc->getName();
                            if (! allHistograms[i]->isRelevant(currentName)) {
                                proc = proc->getPrev();
                                continue;
                            }
                            if (! containsProcess(relevantProcesses, currentName)
                                || (k != 1 && currentName != relevantProcesses[j])) {
                                interString << std::setw(15) << "-" << "\t";
                                proc = proc->getPrev();
                                continue;
                            }
                            interString << std::setw(15) << std::setprecision(5) << errorValue << "\t";

                            proc = proc->getPrev();
                        }
                        datacard << interString.str();
                    }
                    // loop over number of histograms we consider
                }
            }

            datacard << std::endl;
            if (! eraSpecific) break;
        }
    }
    
    if (uncertainty->getNext() == nullptr) {
        writeMCStats();
        return;
    } else if (eraSpecific) return;
    else if (uncertainty->getNext() != nullptr) writeUncertainties(uncertainty->getNext());
    else writeMCStats();
}

void DatacardWriter::write1718Uncertainty(Uncertainty* uncertainty) {
    // consider switching to just 4 bools for era or just naming it erawise
    // write name
    std::string name = uncertainty->getName() + "1718";

    std::vector<TString> relevantProcesses = uncertainty->getRelevantProcesses();

    unsigned k = 1;
    if (! uncertainty->getCorrelatedAmongProcesses()) k = relevantProcesses.size();

    for (unsigned j=0; j<k; j++) {
        std::string tempName = name;
        if (k > 1) tempName += relevantProcesses[j].Data();
        datacard << std::setw(30) << tempName << "\t";

        double errorValue = 1.0;

        if (uncertainty->isFlat()) {
        datacard << std::setw(15) << "lnN" << "\t";
        errorValue = uncertainty->getFlatRate1718();
        } else {
            datacard << std::setw(15) << "shape" << "\t";
        }

        for (unsigned i=0; i < allHistograms.size(); i++) {
            if (! uncertainty->isFlat()) uncertainty->printOutShapeUncertainty(allHistograms[i], allProc->getHead());

            std::stringstream interString;

            Process* proc = allProc->getTail();
            while (proc) {
                TString currentName = proc->getName();
                if (! allHistograms[i]->isRelevant(currentName)) {
                    proc = proc->getPrev();
                    continue;
                }
                if (! containsProcess(relevantProcesses, currentName)
                    || (k != 1 && currentName != relevantProcesses[j])) {
                    interString << std::setw(15) << "-" << "\t";
                    proc = proc->getPrev();
                    continue;
                }
                interString << std::setw(15) << std::setprecision(5) << errorValue << "\t";

                proc = proc->getPrev();
            }

        // loop over number of histograms we consider
            datacard << interString.str();
        }
        datacard << std::endl;
    }

}


void DatacardWriter::writeMCStats() {
    writeEmptyLine(500);
    datacard << "* autoMCStats 0 1 1" << std::endl;
    // double check these numbers
}
