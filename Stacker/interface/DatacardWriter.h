#ifndef DATACARDWRITER_H
#define DATACARDWRITER_H

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

#include "Uncertainty.h"
#include "Histogram.h"

class DatacardWriter
{
private:
    std::ofstream datacard;
    std::string datacardName;

    std::string yearID;

    std::vector<TString> allProcNames;
    std::vector<Histogram*> allHistograms;
    /* data */
public:
    DatacardWriter(std::string yearID, std::vector<TString> allProc, std::vector<Histogram*> histVec);
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