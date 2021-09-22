#include "../interface/Process.h"

Process::Process(TString& procName, int procColor, TFile* procInputfile) : name(procName), rootFile(procInputfile) {
    color = procColor;
    cleanedName = cleanTString(name);

    if (! rootFile->GetDirectory(procName)) {
        std::cout << "ERROR: Process " << procName << " not found." << std::endl;
        exit(2);
    }

    subdirectories = new std::vector<const char*>;

    TList* folders = gDirectory->GetListOfKeys();

    for(const auto&& obj: *folders) {
        subdirectories->push_back(obj->GetName());
    }
}

TH1D* Process::getHistogram(TString& histName, TLegend* legend) {
    TH1D* output = new TH1D();// = new TH1D(histName + "_" + name, name)

    rootFile->cd(name);

    for(auto subdir : *subdirectories) {
        gDirectory->cd(subdir);
        
        TH1D* inter;
        gDirectory->GetObject(histName, inter);

        output->Add(inter);
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
