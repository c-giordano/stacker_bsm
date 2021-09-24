#include "../interface/Stacker.h"

TCanvas* Stacker::getCanvas(TString& histID) {
    return new TCanvas(histID + "_canvas");
}

TPad* Stacker::getPad(TString& histID) {
    return new TPad(histID + "_pad", histID + "_pad", 0., 0., 1., 1.);
}

TLegend* Stacker::getLegend() {
    double x1 = 0.45;
    double y1 = 0.75;
    double x2 = 0.94;
    double y2 = 0.92;
    int nColumns = 3;

    TLegend* legend = new TLegend(x1, y1, x2, y2);
    legend->SetNColumns(nColumns);
    legend->SetMargin(.4);
    legend->SetColumnSeparation(0.1);

    return legend;
}

TLatex* Stacker::getDatasetInfo() {
    TLatex* datasetInfo = new TLatex();
    datasetInfo->SetTextSize(0.034);
    datasetInfo->SetNDC();
    datasetInfo->SetTextAlign(33);
    datasetInfo->SetTextFont(42);

    TString newString = intLumi + " fb^{-1} (13 TeV)";
    //newString += " fb^{-1} (13 TeV)";

    datasetInfo->DrawLatex(0.975, 0.99, newString);

    return datasetInfo;
}
