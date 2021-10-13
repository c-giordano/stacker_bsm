#ifndef HISTOGRAM_2D_H
#define HISTOGRAM_2D_H

#include <TString.h>

#include <vector>
#include <string>
#include <sstream>
#include <iostream>

#include "../../Helpers/interface/ParseTools.h"
#include "../../Helpers/interface/StringTools.h"

#include "Histogram.h"

class Histogram2D : public Histogram {
    private:
        std::vector<std::string>* yBinLabels = nullptr;

    public:
        Histogram2D(TString histID);
        ~Histogram2D();

        std::vector<std::string>* getYBinLabels() const {return yBinLabels;};

        virtual void readSetting(std::string& setting, std::pair<std::string, std::string>& currSetAndVal);

        virtual bool is2D() {return true;}

        static bool search2DHist(Histogram2D*, std::string&);

};

#endif