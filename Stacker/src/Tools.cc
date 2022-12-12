#include "../interface/Tools.h"

TH1D* ApplyBinWidthUnification(TH1D* HistToReplace) {
    // gen new th1 with right number of bins. Bin labels shouldn't matter
    // transfer contents one by one and apply right errors

    TString newName = HistToReplace->GetName();
    HistToReplace->SetName(newName + TString("OLD_UNIBIN"));

    TH1D* tmpOut = new TH1D(newName, newName, HistToReplace->GetNbinsX(), 0., 1.);

    for (unsigned i=1; i<HistToReplace->GetNbinsX()+1; i++) {
        tmpOut->SetBinContent(i, HistToReplace->GetBinContent(i));
        tmpOut->SetBinError(i, HistToReplace->GetBinError(i));
    }
    //delete HistToReplace;
    return tmpOut;    
}