from pythonStacker.createEFTWeights import get_eftvariations_filename
import awkward as ak

import numpy as np
import matplotlib.pyplot as plt
import uproot

if __name__ == "__main__":
    weights = ak.from_parquet(get_eftvariations_filename("", 12))
    ctt_lin = weights[:, 4]
    ctt_sq = weights[:, 26]
    cqq_lin = weights[:, 2]
    cqq_sq = weights[:, 15]

    # load eft variation weights and nominal weight, check avg, min, max and print
    testfilepath = "testfiles/MainTree_MCPrompt_2023-10-19_11-31_TTTT_EFT_MiniAOD2018_processed_localSubmission_tttt_eft_v1.root"
    tree: uproot.TTree = uproot.open(testfilepath+":test")
    arrs = tree.arrays(["nominalWeight", "eftVariations", "eftVariationsNorm", "eventNb"], aliases={"eftVariationsNorm": "eftVariations/eftVariations[:,0]"})

    print("max cttlin:", ak.max(np.abs(ctt_lin)), "; located at: ", np.argmax(np.abs(ctt_lin)), "with event number: ", arrs.eventNb[np.argmax(np.abs(ctt_lin))])
    print("max cttsq:", ak.max(np.abs(ctt_sq)), "; located at: ", np.argmax(np.abs(ctt_sq)), "with event number: ", arrs.eventNb[np.argmax(np.abs(ctt_sq))])
    print("max cqqlin:", ak.max(np.abs(cqq_lin)), "; located at: ", np.argmax(np.abs(cqq_lin)), "with event number: ", arrs.eventNb[np.argmax(np.abs(cqq_lin))])
    print("max cqqsq:", ak.max(np.abs(cqq_sq)), "; located at: ", np.argmax(np.abs(cqq_sq)), "with event number: ", arrs.eventNb[np.argmax(np.abs(cqq_sq))])
    x = np.linspace(0, len(weights), num=len(weights))

    orig_ctt_lin = weights[:, 5]
    orig_ctt_sq = weights[:, 27]
    orig_cqq_lin = weights[:, 3]
    orig_cqq_sq = weights[:, 16]

    print("max cttlin:", ak.max(np.abs(orig_ctt_lin)))
    print("max cttsq:", ak.max(np.abs(orig_ctt_sq)))
    print("max cqqlin:", ak.max(np.abs(orig_cqq_lin)))
    print("max cqqsq:", ak.max(np.abs(orig_cqq_sq)))

    plt.scatter(np.linspace(0, len(ctt_lin), len(ctt_lin)), ctt_lin)
    plt.show()
    plt.scatter(np.linspace(0, len(ctt_lin), len(ctt_lin)), orig_ctt_lin)
    plt.show()
    plt.scatter(np.linspace(0, len(ctt_lin), len(ctt_lin)), ctt_sq)
    plt.show()
    plt.scatter(np.linspace(0, len(ctt_lin), len(ctt_lin)), orig_ctt_sq)
    plt.show()
    plt.scatter(np.linspace(0, len(ctt_lin), len(ctt_lin)), cqq_lin)
    plt.show()
    plt.scatter(np.linspace(0, len(ctt_lin), len(ctt_lin)), orig_cqq_lin)
    plt.show()
    plt.scatter(np.linspace(0, len(ctt_lin), len(ctt_lin)), cqq_sq)
    plt.show()
    plt.scatter(np.linspace(0, len(ctt_lin), len(ctt_lin)), orig_cqq_sq)
    plt.show()
