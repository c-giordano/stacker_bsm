import hepdata_lib
import ROOT
import numpy as np
import json
import postfitPlots as pfPlots
import postfitYields as pfYields

def cross_section_table():
    table_cross_sections = hepdata_lib.Table("Figure 8")

    table_cross_sections.keywords["cmenergies"] = ["13000.0"]
    table_cross_sections.keywords["observables"] = ["SIG"]
    table_cross_sections.keywords["phrases"] = ["Top", "Cross Section", "Inclusive"]
    table_cross_sections.keywords["reactions"] = ["P P --> TQ TQBAR TQ TQBAR", "P P --> TOP TOPBAR TOP TOPBAR"]

    table_cross_sections.add_image("/home/njovdnbo/Documents/Stacker_v2/hepdata/input_others/significance.pdf")
    table_cross_sections.description = "Comparison of fit results in the channels individually and in their combination. The left panel shows the values of the measured cross section relative to the SM prediction from Ref. [6]. The right panel shows the expected and observed significance, with the printed values rounded to the first decimal."
    table_cross_sections.location = "Figure 8, page 14"
    
    channels = hepdata_lib.Variable("Channel", is_independent=True, is_binned=False)
    channels.values = [r"$t\bar{t}t\bar{t}$ in 2$\ell$", r"$t\bar{t}t\bar{t}$ in 3$\ell$", r"$t\bar{t}t\bar{t}$ in 4$\ell$", r"$t\bar{t}t\bar{t}$ combined", r"$t\bar{t}W$", r"$t\bar{t}Z$"]

    cross_section = hepdata_lib.Variable(r"Cross section [fb]", is_independent=False, is_binned=False)
    cross_section.add_qualifier("SQRT(S)", 13, units="TeV")
    cross_section.add_qualifier("LUMINOSITY", 138, units=r"fb$^{-1}$")

    cross_section.values = [17.61, 19.69, -14.65, 17.69, 990., 945.]
    cross_section_staterr = hepdata_lib.Uncertainty("Stat", is_symmetric=False)
    cross_section_staterr.values = [[-4.31, 4.69], [-6.36, 7.07], [-2.07, 12.82], [-3.49, 3.72], [-58., 58.], [-43., 43.]]
    cross_section.add_uncertainty(cross_section_staterr)

    cross_section_systerr = hepdata_lib.Uncertainty("Syst", is_symmetric=False)
    cross_section_systerr.values = [[-2.59, 2.50], [-2.03, 2.59], [-4.89, 2.45], [-1.80, 2.21], [-79., 79.], [-69., 69.]]
    cross_section.add_uncertainty(cross_section_systerr)

    table_cross_sections.add_variable(channels)
    table_cross_sections.add_variable(cross_section)

    significance = hepdata_lib.Variable("Observed significance", is_independent=False, is_binned=False)
    significance.values = [4.0, 3.5, 0., 5.6, 0., 0.]
    table_cross_sections.add_variable(significance)
    
    significance_expected = hepdata_lib.Variable("Expected significance", is_independent=False, is_binned=False)
    significance_expected.values = [4.0, 2.9, 0.8, 4.9, 0., 0.]
    table_cross_sections.add_variable(significance_expected)

    return table_cross_sections

if __name__ == "__main__":
    submission = hepdata_lib.Submission()

    ## general setup

    submission.read_abstract("input_others/abstract.txt")
    # submission.add_link() ## public webpages
    # submission.add_link("arXiv") ## Arxiv
    # submission.add_record_id(0, "inspire") ## inspire ID


    ### tables with central result
    table_cross_sections = cross_section_table()
    submission.add_table(table_cross_sections)

    pfYields.read_table_json(submission, "/home/njovdnbo/Documents/Stacker_v2/hepdata/input_others/tables_paper.json")
    
    # table = pfPlots.read_postfit_plot("njets_crz_OneMedB_srFit_pf_variables.txt_obs_crz_shapes.root", "TestFigure", r"N_{jets}")
    # submission.add_table(table)

    submission.create_files("output",remove_old=True)