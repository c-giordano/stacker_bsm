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
        std::map<std::string, double> takenEras; // era and lumi

        std::string initEra = inputfile->GetName();
        initEra = getFilename(initEra);

        if (stringContainsSubstr(initEra, "_201")) {
            std::vector<std::string> eraSplit = split(initEra, "_");
            for (auto it : eraSplit) {
                if (stringContainsSubstr(it, "201")) {
                    initEra = it;
                    break;
                }
            }
            takenEras[initEra] = lumiHist->GetBinContent(1);
        }


        for (unsigned i = 1; i < inputfiles.size(); i++) {
            // get filename
            // get year from filename
            // put in map together with lumi, only add if not present

            TFile* it = inputfiles[i];
            it->cd();
            TH1F* inter;
            it->GetObject("IntLumi", inter);

            std::string currentEra = it->GetName();
            currentEra = getFilename(currentEra);
            if (! stringContainsSubstr(currentEra, "_201")) {
                lumiHist->Add(inter);
                continue;
            }

            std::vector<std::string> eraSplit = split(currentEra, "_");
            for (auto it : eraSplit) {
                if (stringContainsSubstr(it, "201")) {
                    currentEra = it;
                    break;
                }
            }
            
            if ( takenEras.find(currentEra) == takenEras.end() ) {
                lumiHist->Add(inter);
                takenEras[currentEra] = inter->GetBinContent(1);
                std::cout << "New era: " << currentEra << " with Lumi " << inter->GetBinContent(1) << std::endl;
                // not found
            } else {
                double comp = takenEras[currentEra];
                std::cout << "existing era " << currentEra << " with Lumi " << inter->GetBinContent(1) << std::endl;
                if (inter->GetBinContent(1) != comp) {
                    std::cerr << "Attention! Lumi in file " << it->GetName() << " does not match lumi in first file from this era" << std::endl;
                }
            }
            // lumiHist->Add(inter);
        }

        std::stringstream stream;
        int precision = 1;
        if (lumiHist->GetBinContent(1) >= 100.) {
            precision = 0;
        }
        stream << std::fixed << std::setprecision(precision) << lumiHist->GetBinContent(1);
        intLumi = stream.str();
        std::cout << "Lumi is " << intLumi << " fb^{-1}" << std::endl;
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