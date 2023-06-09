#include "../interface/DoubleRatioPlot.h"

void DoubleRatioPlot::DoubleRatioPlot(Stacker& stacker) {
    std::vector<Histogram*> histsToPlot;
    for (auto& histogram : stacker.GetHistograms()) {
        histogram->setPrintToFile(false);
        //std::cout << std::string(histogramID->getID().Data()) << std::endl;
        //if (std::string(histogramID->getID().Data()) != "BDT_FinalresultSignal_TriClass_SR-2Lee" && std::string(histogramID->getID().Data()) != "BDT_FinalresultSignal_TriClass_SR-3L") continue;
        //if (! stringContainsSubstr(std::string(histogramID->getID().Data()), "CR-3L-Z")) continue;
        if (std::string(histogram->getID().Data()) == "BDT_Finalresult_tanh_Signal_TriClass_SR-2Lee") {
            histsToPlot.push_back(histogram);
            break;
        }
    }
    DoubleRatioPlot::BuildMainPlot(stacker, histsToPlot);
}

void DoubleRatioPlot::BuildMainPlot(Stacker& stacker, std::vector<Histogram*>& histograms) {

}

void DoubleRatioPlot::BuildSubPlot(Stacker& stacker, Histogram* histogram, TPad* plotPad, int plotNb) {

}
