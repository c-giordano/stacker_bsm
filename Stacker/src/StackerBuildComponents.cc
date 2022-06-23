#include "../interface/Stacker.h"

TCanvas* Stacker::getCanvas(TString& histID) {
	if (isRatioPlot) {
		gStyle->SetCanvasDefH(941);
	}
    return new TCanvas(histID + "_canvas");
}

TPad* Stacker::getPad(TString& histID, int position) {
	double margin = 0.013;
	TPad* outputPad;
	if (position == 0) {
		if (isRatioPlot) {
			outputPad = new TPad(histID + "_pad", histID + "_pad", 0., 0.24, 1., 1.);
			outputPad->SetBottomMargin(margin);		
		} else {
			outputPad = new TPad(histID + "_pad", histID + "_pad", 0., 0., 1., 1.);
		}
	}  else if (position == 1) {
		outputPad = new TPad(histID + "_pad", histID + "_pad", 0., 0., 1., 0.24);
		outputPad->SetTopMargin(margin);
		outputPad->SetBottomMargin(0.4);
	} else {
		outputPad = new TPad(histID + "_pad", histID + "_pad", 0., 0., 1., 1.);
	}

	return outputPad;
}

TLegend* Stacker::getLegend() {
    double x1 = 0.4;
    double y1 = 0.69;
    double x2 = 1 - gStyle->GetPadLeftMargin() - gStyle->GetTickLength("X")*1.1 ;
    double y2 = 1 - gStyle->GetPadTopMargin() - gStyle->GetTickLength("Y")*1.1;
    int nColumns = 2;

    TLegend* legend = new TLegend(x1, y1, x2, y2);
    legend->SetNColumns(nColumns);
    legend->SetMargin(.4);
    legend->SetColumnSeparation(0.1);
	legend->SetFillStyle(0);

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
    TString extraText = "Work in progress";

    const float l = pad->GetLeftMargin() + gStyle->GetTickLength()*1.2;
  	const float t = pad->GetTopMargin();
  	const float r = pad->GetRightMargin();
  	//const float b = pad->GetBottomMargin();

	float CMSTextSize = pad->GetTopMargin()*0.75;
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
	//float cmsX = latex->GetXsize();
	latex->DrawLatex(l,1  - (2 * t),"CMS");

	float extraTextSize = CMSTextSize*0.76;	 
	latex->SetTextFont(52);
	latex->SetTextSize(extraTextSize);
	latex->SetTextAlign(11);
	//std::cout << extraText << " " << l + 1.2 * cmsX << std::endl;
	// using cmsX gave strange results
	latex->DrawLatex(l, 1-(2 * t) - (extraTextSize*1.1), extraText);

	latex->SetTextFont(42);
	latex->SetTextAlign(31);
	latex->SetTextSize(lumiTextSize);  
	latex->DrawLatex(1-r,1-t+lumiTextOffset,lumiText);

	return latex;
}
