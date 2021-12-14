#include "../interface/DatacardWriter.h"

DatacardWriter::DatacardWriter(std::string yearID, std::vector<TString> allProc, std::vector<Histogram*> histVec) :
    yearID(yearID), allProcNames(allProc), allHistograms(histVec)
{   
    std::reverse(allProcNames.begin(), allProcNames.end());
    datacardName = "DC_" + yearID;
    initDatacard();
}


void DatacardWriter::initDatacard() {
    datacard.open("combineFiles/" + datacardName + ".txt");

    writeInit();
    writeShapeSource();
    writeDataObs();
    writeProcessHeader();
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
        for (unsigned j=0; j < allProcNames.size(); j++) {
            datacard << std::setw(15) << allHistograms[i]->getCleanName() << "\t";
            processline << std::setw(15) << allProcNames[j].Data() << "\t";
            processNumbers << std::setw(15) << j << "\t";
            rates << std::setw(15) << "-1" << "\t";
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
    if (uncertainty->isEraSpecific() && ! eraSpecific && uncertainty->isBothEraAndFull()) {
        writeUncertainties(uncertainty, true);
    } else if (! uncertainty->isBothEraAndFull() && uncertainty->isEraSpecific()) {
        writeUncertainties(uncertainty, true);
        if (uncertainty->getNext() != nullptr) writeUncertainties(uncertainty->getNext());
        return;
    }
    if (uncertainty->is1718Specific() && ! eraSpecific) write1718Uncertainty(uncertainty);

    std::cout << "past initial unc stuff " << std::endl;
    // write name
    std::string name = uncertainty->getName();
    if (eraSpecific) name += yearID;
    datacard << std::setw(30) << name << "\t";

    double errorValue = 1.0;

    if (uncertainty->isFlat()) {
        datacard << std::setw(15) << "lnN" << "\t";
        if (eraSpecific) errorValue = uncertainty->getFlatRateEra();
        else errorValue = uncertainty->getFlatRateAll();
    } else {
        datacard << std::setw(15) << "shape" << "\t";
    }

    std::stringstream interString;
    std::vector<TString> relevantProcesses = uncertainty->getRelevantProcesses();

    for (unsigned i = 0; i < allProcNames.size(); i++) {
        if (! containsProcess(relevantProcesses, allProcNames[i])) {
            interString << std::setw(15) << "-" << "\t";
            continue;
        }
        interString << std::setw(15) << std::setprecision(5) << errorValue << "\t";
    }

    // loop over number of histograms we consider
    for (unsigned i=0; i < allHistograms.size(); i++) {
        datacard << interString.str();
    }
    datacard << std::endl;

    if (eraSpecific) return;
    else if (uncertainty->getNext() != nullptr) writeUncertainties(uncertainty->getNext());
    else writeMCStats();

}

void DatacardWriter::write1718Uncertainty(Uncertainty* uncertainty) {
    // consider switching to just 4 bools for era or just naming it erawise
    // write name
    std::string name = uncertainty->getName() + "1718";

    datacard << std::setw(30) << name << "\t";
    double errorValue = 1.0;

    if (uncertainty->isFlat()) {
        datacard << std::setw(15) << "lnN" << "\t";
        errorValue = uncertainty->getFlatRate1718();
    } else {
        datacard << std::setw(15) << "shape" << "\t";
    }

    std::stringstream interString;
    std::vector<TString> relevantProcesses = uncertainty->getRelevantProcesses();

    for (unsigned i = 0; i < allProcNames.size(); i++) {
        if (! containsProcess(relevantProcesses, allProcNames[i])) {
            interString << std::setw(15) << "-" << "\t";
            continue;
        }
        interString << std::setw(15) << std::setprecision(5) << errorValue << "\t";
    }

    // loop over number of histograms we consider
    for (unsigned i=0; i < allHistograms.size(); i++) {
        datacard << interString.str();
    }
    datacard << std::endl;

}


void DatacardWriter::writeMCStats() {
    writeEmptyLine(50);
    datacard << "* autoMCStats 0 1 1" << std::endl;
    // double check these numbers
}
