#include "../interface/Stacker.h"

TCanvas* Stacker::getCanvas(TString& histID) {
    return new TCanvas(histID + "_canvas");
}

TPad* Stacker::getPad(TString& histID) {
    return new TPad(histID + "_pad", histID + "_pad", 0., 0., 1., 1.);
}

TLegend* Stacker::getLegend() {
    double x1 = 0.5;
    double y1 = 0.75;
    double x2 = 0.95;
    double y2 = 0.9;
    int nColumns = 3;

    TLegend* legend = new TLegend(x1, y1, x2, y2);
    legend->SetNColumns(nColumns);
    legend->SetMargin(.4);
    legend->SetColumnSeparation(0.1);

    return legend;
}