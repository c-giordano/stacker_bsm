#include "../interface/Process.h"

Process::Process(TString& procName, int procColor, TFile* procInputfile, bool signal, bool data) : name(procName), rootFile(procInputfile),
    isSignal(signal), isData(data) {
    color = procColor;
    cleanedName = cleanTString(name);

    rootFile->cd("Nominal");

    if (! gDirectory->GetDirectory(procName)) {
        std::cout << "ERROR: Process " << procName << " not found." << std::endl;
        std::cout << "Trying rootfile itself..." << std::endl;
        rootFile->cd();

        if (! gDirectory->GetDirectory(procName)) {
            std::cout << "ERROR: Process still " << procName << " not found." << std::endl;
            exit(2);
        }

        //exit(2);
    }

    if (procName == "nonPrompt") {
        subdirectories = new std::vector<const char*>;
        subdirectories->push_back("../nonPrompt");
        return;
    }

    gDirectory->cd(procName);
    subdirectories = new std::vector<const char*>;

    TList* folders = gDirectory->GetListOfKeys();

    for(const auto&& obj: *folders) {
        subdirectories->push_back(obj->GetName());
    }

}

TH1D* Process::getHistogram(TString& histName, TLegend* legend) {
    TH1D* output = nullptr;// = new TH1D();// = new TH1D(histName + "_" + name, name)

    rootFile->cd("Nominal");
    if (! gDirectory->GetDirectory(name)) {
        std::cout << "ERROR: Process " << name << " not found." << std::endl;
        std::cout << "Trying rootfile itself..." << std::endl;
        rootFile->cd();

        if (! gDirectory->GetDirectory(name)) {
            std::cout << "ERROR: Process still " << name << " not found." << std::endl;
            exit(2);
        }

        //exit(2);
    }
    gDirectory->cd(name);

    for(auto subdir : *subdirectories) {
        gDirectory->cd(subdir);
        
        TH1D* inter;
        gDirectory->GetObject(histName, inter);

        if (output == nullptr) {
            output = new TH1D(*inter);
        } else {
            output->Add(inter);
        }

        // Read stuff, add to outputhistogram (maybe first output is stack but then print it to th?)

        gDirectory->cd("..");
    }

    output->SetName(histName + name);
    output->SetTitle(histName + name);
    
    output->SetLineColor(color);
    output->SetFillColor(color);
    output->SetMarkerColor(color);

    legend->AddEntry(output, cleanedName);
    
    return output;
}
