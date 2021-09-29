#include "../interface/thTools.h"

void normalizeHistograms(std::vector<TH1D*>& histos) {
    for (auto curr : histos) {
        curr->Scale(1/curr->Integral());
    }
}