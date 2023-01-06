#include "Stacker/interface/Stacker.h"
#include <iostream>
#include "Styles/tdrStyle.h"

int main(int argc, char const *argv[])
{
    if (argc < 3) {
        std::cerr << "Stacker expects at least 2 arguments: ./stacker_exec <root-file> <settingfile>" << std::endl;
        exit(1);
    }
    std::vector< std::string > argvStr( &argv[1], &argv[0] + argc );

    setTDRStyle();
    gROOT->ForceStyle();

    Stacker stacker(argvStr);
    bool savePrev = true;
    for (int i = 0; i < argc-1; i++) {
        std::string currentElement = argvStr[i];
        std::cout << "checking elements. Current one: " << currentElement << std::endl;
        if (currentElement == "-v") stacker.setVerbosity(true);
        if (currentElement == "-vv") {
            stacker.setVerbosity(true);
            stacker.setVeryVerbosity(true);
        }
        if (currentElement == "-local") stacker.useT2B(false);
        if (currentElement == "-unc") stacker.readUncertaintyFile(argvStr[i+1]);
        if (currentElement == "-FD") {
            stacker.setFakeData(true);
            stacker.setData(true);
        }
        if (currentElement == "-RD") stacker.readData(argvStr, i+1);
        
        if (currentElement == "-DC") {
            stacker.setOnlyDC(true);
            stacker.initDatacard();
            stacker.GetDCWriter()->WriteDatacardVariations();
            savePrev = false;
        }
        
        if (currentElement == "-IP") {
            std::cout << "Drawing impact of uncertainties" << std::endl;
            stacker.drawAllUncertaintyImpacts();
            stacker.SaveToVault();
            exit(0);
        }

        if (currentElement == "-COMP") {
            stacker.plotDifference(argvStr);

            stacker.SaveToVault();
        }

        if (currentElement == "-stats") {
            gStyle->SetOptStat(11);
        }

        if (currentElement == "-SF") {
            stacker.GenerateSFs(argvStr[i+1]);
            exit(0);
        }
    }

    stacker.printAllHistograms();
    stacker.printAll2DHistograms();

    if (savePrev) stacker.SaveToVault();
    return 0;
}
