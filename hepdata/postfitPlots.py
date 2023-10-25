import hepdata_lib
import ROOT
import numpy as np
import json

def get_list_of_folders_and_processes(tfile):
    folders = []
    processes = []
    for key in tfile.GetListOfKeys():
        if "_postfit" in key.GetName():
            folders.append(key.GetName())
    
    tfile.cd(folders[0])
    for key in ROOT.gDirectory.GetListOfKeys():
        if "Total" in key.GetName() or "data_obs" in key.GetName(): continue
        processes.append(key.GetName())

    return folders, processes

def read_postfit_plot(filename, name, axislabel, axisunits):
    ## open root file
    # loop over processes and put in hepdata object
    reader = hepdata_lib.RootFileReader("input_figures/"+filename)
    folders, processes = get_list_of_folders_and_processes(reader.tfile)

    xaxis = hepdata_lib.Variable(axislabel, is_independent=True, is_binned=True, units=axisunits)
    xaxis.values = reader.read_hist_1d(folders[0]+"/"+processes[0])["x_edges"]

    output_process = dict()
    output_process_unc = dict()
    for i, folder in enumerate(folders):
        for process in processes:
            hist = reader.read_hist_1d(folder+"/"+process)
            if i==0:
                output_process[process] = np.array(hist['y'])
                output_process_unc[process] = np.power(np.array(hist['dy']), 2)
            else:
                output_process[process] += np.array(hist['y'])
                output_process_unc[process] += np.power(np.array(hist['dy']), 2)
    
    table = hepdata_lib.Table(name)
    table.add_variable(xaxis)

    for process in processes:
        var = hepdata_lib.Variable(process, is_independent=False, is_binned=False, units="Events per bin")
        var.values = output_process[process]

        var_unc = hepdata_lib.Uncertainty("Syst", is_symmetric=True)
        var_unc.values = np.sqrt(output_process_unc[process])

        var.add_uncertainty(var_unc)

        table.add_variable(var)

    var = hepdata_lib.Variable("Observed", is_independent=False, is_binned=False, units="Events per bin")
    var.add_qualifier("SQRT(S)", 13, units="TeV")
    var.add_qualifier("LUMINOSITY", 138, units=r"fb$^{-1}$")

    hist_data = reader.read_hist_1d(folder+"/data_obs")
    var.values = hist_data["y"]

    # var_unc = hepdata_lib.Uncertainty("Poisson", is_symmetric=True)
    # var_unc.values = hist_data["dy"]
    # var.add_uncertainty(var_unc)
    table.add_variable(var)

    return table

def create_postfit_plot(entryname, entry_dict):
    table = read_postfit_plot(entry_dict["filename"], entryname, entry_dict["axislabel"], entry_dict["axisunits"])

    return table

def apply_cosmetics(table, info):
    # cmenergies
    # observables
    # phrases
    # reactions$
    # description
    # location
    # image

    return table

def read_json_pfplots(filepath):
    # read
    plots = []
    file = open(filepath)
    jsonfile = json.load(file)
    for key in jsonfile:
        for subkey in jsonfile[key]:
            table = create_postfit_plot(subkey, jsonfile[key][subkey])
            table = apply_cosmetics(table, jsonfile[key][subkey])
            plots.append(table)

    return plots

if __name__ == "__main__":
    submission = hepdata_lib.Submission()

    tables = read_json_pfplots("input_others/test.json")

    for table in tables:
        submission.add_table(table)
    submission.create_files("output",remove_old=True)
    
