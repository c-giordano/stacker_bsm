#ifndef HISTOGRAM_MANAGER_H
#define HISTOGRAM_MANAGER_H

#include <TString.h>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#include "../../Helpers/interface/ParseTools.h"
#include "../../Helpers/interface/StringTools.h"

class Histogram {
    private:
        TString id;
        bool logScale; // 0 for lin, 1 for log
        std::vector<std::string>* binLabels = nullptr;
    public:
        Histogram(TString histID);
        Histogram(TString histID, bool requireLogScale);
        ~Histogram();

        TString getID() const {return id;};
        bool isLogscale() const {return logScale;};
        std::vector<std::string>* getBinLabels() const {return binLabels;};

        void readSettings(std::istringstream& settingLine);

        static bool searchHist(Histogram*, std::string&);
};

//bool searchHist(Histogram*, std::string&);

#endif