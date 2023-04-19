#ifndef DOUBLERATIOPLOT_H
#define DOUBLERATIOPLOT_H

#include "TPad.h"

#include "../../Stacker/interface/Stacker.h"

namespace DoubleRatioPlot
{
    void DoubleRatioPlot(Stacker& stacker);
    void BuildMainPlot(Stacker& stacker, std::vector<Histogram*>& histograms);
    void BuildSubPlot(Stacker& stacker, Histogram* histogram, TPad* plotPad, int plotNb);
} // namespace DoubleRatioPlot



#endif