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

    bool verbose = false;
    if (argc == 4 && strcmp(argv[3], "-v") == 0) {
        verbose = true;
    }

    Stacker stacker(rootFile, settingsFile);
    stacker.setVerbosity(verbose);
    stacker.printAllHistograms();
    stacker.printAll2DHistograms();

    return 0;
}
