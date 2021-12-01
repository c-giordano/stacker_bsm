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
        bool drawUncertainties = false;
        std::vector<std::string>* xBinLabels = nullptr;
    public:
        Histogram(TString histID);
        Histogram(TString histID, bool requireLogScale);
        ~Histogram();

        TString getID() const {return id;};
        bool isLogscale() const {return logScale;};

        bool getDrawUncertainties() {return drawUncertainties;}
        void setDrawUncertainties(bool draw) {drawUncertainties = draw;}
        
        std::vector<std::string>* getXBinLabels() const {return xBinLabels;};

        void setXBinLabels(std::vector<std::string>& binlabels) {
            xBinLabels = new std::vector<std::string>(binlabels);
        }

        void readSettings(std::istringstream& settingLine);
        virtual void readSetting(std::string& setting, std::pair<std::string, std::string>& currSetAndVal);

        virtual bool is2D() {return false;}

        static bool searchHist(Histogram*, std::string&);
};

//bool searchHist(Histogram*, std::string&);

#endif