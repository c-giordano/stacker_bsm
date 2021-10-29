#include "Stacker/interface/Stacker.h"
#include <iostream>


int main(int argc, char const *argv[]) {
    // parse settings to create stacker
    if (argc < 3) {
        std::cout << "Stacker works by supplying 2 arguments: ./Stacker <root-file> <settingfile>" << std::endl;
        exit(1);
    }

    // if required input is met, pass stuff to other stuff
    const char* rootFile = argv[1];
    std::string settingsFile = argv[2];

    bool runT2B = true;

    bool verbose = false;
/*
    if (argc == 4) {
        std::string argFour = argv[3];
        if (argFour == "-v") {

        }
    }*/
    if (argc == 4 && strcmp(argv[3], "-v") == 0) {
        verbose = true;
    } else if ((argc == 4 && strcmp(argv[3], "-local") == 0) || (argc == 5 && strcmp(argv[4], "-local") == 0)) {
        runT2B = false;
    }

    Stacker stacker(rootFile, settingsFile, runT2B);
    stacker.setVerbosity(verbose);
    stacker.printAllHistograms();
    stacker.printAll2DHistograms();

    return 0;
}
