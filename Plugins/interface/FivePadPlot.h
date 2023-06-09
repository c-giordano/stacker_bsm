#ifndef FIVEPADPLOT_H
#define FIVEPADPLOT_H

#include "TPad.h"

#include "../../Stacker/interface/Stacker.h"

// tools to build plot with five pads

namespace FivePadPlot
{   
    void FivePadPlot(Stacker& stacker);
    void BuildMainPlot(Stacker& stacker, std::vector<Histogram*>& histograms);
    void BuildSubPlot(Stacker& stacker, Histogram* histogram, TPad* plotPad, int plotNb);
}

// namespace FivePadPlot

#endif