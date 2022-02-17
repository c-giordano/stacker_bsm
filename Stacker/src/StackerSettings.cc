#include "../interface/Stacker.h"
#include <iomanip>
#include <sstream>

void Stacker::setLumi(std::string& lumiSetting) {
    try {
        double intLumiDouble = std::stod(lumiSetting);
        intLumi = lumiSetting;
        std::cout << "Luminosity is " << intLumiDouble << " fb^(-1)" << std::endl;
        return;
    } catch(const std::exception& e) {
        std::cout << "Getting luminosity from histogram" << std::endl;
    }
    if(inputfile->GetListOfKeys()->Contains("IntLumi")) {
        TH1F* lumiHist;
        inputfile->GetObject("IntLumi", lumiHist);

        for (unsigned i = 1; i < inputfiles.size(); i++) {
            TFile* it = inputfiles[i];
            it->cd();
            TH1F* inter;
            it->GetObject("IntLumi", inter);
            lumiHist->Add(inter);
        }

        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << lumiHist->GetBinContent(1);
        intLumi = stream.str();
    } else {
        std::cout << "Luminosity not found. Are you sure it is supplied?" << std::endl;
        exit(1);
    }
}

void Stacker::setDrawOpt(std::string& drawSetting) {
    std::stringstream stream(drawSetting);
    std::string part;
    drawOpt = "";
    while (getline(stream, part, ',')) {
        cleanWhitespace(part);

        // Todo: continue this stuff
        drawOpt += part + " ";

        if (part == "NOSTACK") {
            noStack = true;
            // Fillcolor of all histograms must be set to 0 
            // Yaxis title must change
            yAxisOverride += "a.u.";
            gStyle->SetHistLineWidth(3);
            gStyle->SetHistFillStyle(0);

            if (runT2B) {
                pathToOutput += "Shapes/";
            }
            
            gROOT->ForceStyle();
        }
    }
}