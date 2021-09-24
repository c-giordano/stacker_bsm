#include "../interface/Stacker.h"

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
        intLumi = std::to_string(lumiHist->GetBinContent(1));
    } else {
        std::cout << "Luminosity not found. Are you sure it is supplied?" << std::endl;
        exit(1);
    }
}
