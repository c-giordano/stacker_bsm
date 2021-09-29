#ifndef HISTOGRAM_MANAGER_H
#define HISTOGRAM_MANAGER_H

#include <TString.h>
#include <vector>
#include <string>

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


        static bool searchHist(Histogram*, std::string&);
};

//bool searchHist(Histogram*, std::string&);

#endif