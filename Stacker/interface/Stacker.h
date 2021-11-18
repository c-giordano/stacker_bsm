#ifndef STACKER_H
#define STACKER_H

#include <TROOT.h>
#include <TH1.h>
#include <THStack.h>
#include <TFile.h>
#include <TCanvas.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TLine.h>
#include <TGaxis.h>
#include <TColor.h>
//#include <TRatioPlot.h>

#include <algorithm>

#include "ProcessList.h"
#include "Process.h"
#include "Histogram.h"
#include "Histogram2D.h"

#include "../../Helpers/interface/ParseTools.h"
#include "../../Helpers/interface/StringTools.h"
#include "../../Helpers/interface/thTools.h"
#include "../../Helpers/interface/Others.h"

#include "../../Styles/tdrStyle.h"
/*
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem.hpp>
*/
// Use global setting file which keeps track of colors, order, definitions of components... -> SettingParser
// Keep track of which files belong to which process with which color -> other class? or linked list with entry for each process and vector for applicable files
// 

class Stacker {
    private:
        bool verbose = false;

        ProcessList* processes;
        std::vector<Histogram*> histogramVec;
        std::vector<Histogram2D*> histogramVec2D;
        // settings
        int canvWidthX = 600; // Possibly changing directly using gStyle... or save in style object being made. 
        int canvWidthY = 600;

        // Build components of histogram based on settings
        TCanvas* getCanvas(TString& histID);
        TPad* getPad(TString& histID, int position=0);
        TLegend* getLegend();
        TLatex* getDatasetInfo(TPad* currentPad);

        // Main root file
        TFile* inputfile;
        // outputfile (structured for combine)
        TFile* outputfile = nullptr;

        // Histograms to consider (of all processes)

        // General settings
        std::string intLumi = "";
        std::string drawOpt = "HIST";
        std::string yAxisOverride = "";

        bool noStack = false;

        bool runT2B;
        std::string pathToOutput;

        bool isRatioPlot = false;
        bool isSignalLine = false;

    public:
        Stacker(const char* rootFile, std::string& settingFile, bool runT2Btrue);
        ~Stacker();

        void setVerbosity(bool verbosity) {
            verbose = verbosity;
            processes->setVerbosity(verbosity);
        }
        void printAllHistograms();
        void printHistogram(Histogram* histID);
        void drawStack(Histogram* hist, THStack* histStack, std::vector<TH1D*>& histVec);
        void drawSignalYield(TLegend* legend, std::vector<TH1D*>& signalVec);
        void drawRatioMC(Histogram* hist, std::vector<TH1D*>& histoVec, std::vector<TH1D*>& signalVec);
        std::vector<TH1D*> fillStack(THStack* stack, TString& histogramID, TLegend* legend, TFile* outfile);
        
        void printAll2DHistograms();
        void print2DHistogram(Histogram2D* histID);

        void setLumi(std::string& lumiSetting);
        void setDrawOpt(std::string& drawSetting);

        void stackSettingsPreDraw(THStack* stack, std::vector<TH1D*>& histVec);
        void stackSettingsPostDraw(TPad* pad, THStack* stack, Histogram* hist, TH1* first);

};

#endif