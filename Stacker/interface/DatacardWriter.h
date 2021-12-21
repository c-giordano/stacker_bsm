#ifndef DATACARDWRITER_H
#define DATACARDWRITER_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <map>

#include "Uncertainty.h"
#include "Histogram.h"
#include "ProcessList.h"

class DatacardWriter
{
private:
    std::ofstream datacard;
    std::string datacardName;
    std::string yearID;

    TFile* outfile;
    ProcessList* allProc;
    //std::vector<Process*> allProc;
    std::vector<Histogram*> allHistograms;

    Process* head;
    /* data */
public:
    DatacardWriter(std::string yearID, ProcessList* allProc, std::vector<Histogram*> histVec, TFile* outfile);
    ~DatacardWriter();

    void initDatacard();

    void writeEmptyLine(unsigned length);

    void writeInit();
    void writeShapeSource();
    void writeDataObs();
    void writeProcessHeader();
    void writeUncertainties(Uncertainty* uncertainty, bool eraSpecific = false);
    void write1718Uncertainty(Uncertainty* uncertainty);
    void writeMCStats();

    bool containsProcess(std::vector<TString>& vector, TString& process);
};

#endif