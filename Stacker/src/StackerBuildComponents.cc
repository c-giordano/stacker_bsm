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

TLatex* Stacker::getDatasetInfo(TPad* pad) {
    /*
    TLatex* datasetInfo = new TLatex();
    datasetInfo->SetTextSize(0.034);
    datasetInfo->SetNDC();
    datasetInfo->SetTextAlign(33);
    datasetInfo->SetTextFont(42);

    TString newString = intLumi + " fb^{-1} (13 TeV)";
    //newString += " fb^{-1} (13 TeV)";

    datasetInfo->DrawLatex(0.975, 0.99, newString);

    return datasetInfo;
    */

    TString lumiText = intLumi + " fb^{-1} (13 TeV)";
    TString extraText = "";

    const float l = pad->GetLeftMargin();
  	const float t = pad->GetTopMargin();
  	const float r = pad->GetRightMargin();
  	//const float b = pad->GetBottomMargin();

	float CMSTextSize = pad->GetTopMargin()*0.8;
	float lumiTextSize = pad->GetTopMargin()*0.6;

	//float CMSTextOffset = pad->GetTopMargin()*0.2;
	float lumiTextOffset = pad->GetTopMargin()*0.2;
	
	pad->cd();
	//Define latex text to draw on plot
	TLatex* latex = new TLatex(l,1+lumiTextOffset*t,"CMS");
	latex->SetNDC();
	latex->SetTextAngle(0);
	latex->SetTextColor(kBlack); 

	latex->SetTextFont(61);
	latex->SetTextAlign(11); 
	latex->SetTextSize(CMSTextSize);
	const float cmsX = latex->GetXsize();
	latex->DrawLatex(l,1  -t + lumiTextOffset,"CMS");

	const float extraTextSize = CMSTextSize*0.76;	 
	latex->SetTextFont(52);
	latex->SetTextSize(extraTextSize);
	latex->SetTextAlign(11);
	latex->DrawLatex(l + 1.2*cmsX, 1-t+lumiTextOffset, extraText);

	latex->SetTextFont(42);
	latex->SetTextAlign(31);
	latex->SetTextSize(lumiTextSize);  
	latex->DrawLatex(1-r,1-t+lumiTextOffset,lumiText);
	return latex;
}
