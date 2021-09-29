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

        std::stringstream stream;
        stream << std::fixed << std::setprecision(2) << lumiHist->GetBinContent(1);
        intLumi = stream.str();
    } else {
        std::cout << "Luminosity not found. Are you sure it is supplied?" << std::endl;
        exit(1);
    }
}
