import hepdata_lib
import json

def format_table_line(line):
    # line = str()
    split_line = line[:line.rfind("\\\\")].rstrip().lstrip().split("&")
    return split_line

def latex_to_hepdata(filepath, name):
    xaxis = [] # processes
    values = []
    uncertainties = []
    # for each column we need values! The first filling is the name of the thing! 
    # while not non-\, loop. If we found non: stop and read based on &!
    file = open(filepath)
    lines = file.readlines()
    i=0
    
    while not "\\caption" in lines[i]:
        i+=1
    
    description = lines[i].replace("\\caption{", "").replace("}\n", "")
    # print(description)

    while lines[i][0] == "\\":
        i+=1
    
    split_header = format_table_line(lines[i])
    xaxis.append(split_header[0].lstrip().rstrip())

    for el in split_header[1:]:
        values.append([el.lstrip().rstrip()])
        uncertainties.append([])

    for line in lines[i+1:]:
        if line[0] == "\\": continue
        split_line = format_table_line(line)
        xaxis.append(split_line[0].lstrip().rstrip())
        for i, entry in enumerate(split_line[1:]):
            if "\\pm" in entry:
                val, unc = entry.split("$\\pm$")
                val_d = float(val)
                unc_d = float(unc)
                values[i].append(val_d)
                uncertainties[i].append(unc_d)
            else:
                if entry:
                    values[i].append(float(entry))
                else:
                    values[i].append(0.)
        
        # line starting with $ is fine and should be kept
        # also this one is the number stuff! split at $\pm$
    # now here start hepdata logic
    xaxis_hd = hepdata_lib.Variable(xaxis[0], is_independent=True, is_binned=False)
    xaxis_hd.values = xaxis[1:]

    table = hepdata_lib.Table(name)
    table.description = description
    table.add_variable(xaxis_hd)
    
    for i, val_entry in enumerate(values):
        variable = hepdata_lib.Variable(val_entry[0], is_independent=False, is_binned=False, units="Events")
        variable.values = val_entry[1:]

        if i == 0:
            variable.add_qualifier("SQRT(S)", 13, units="TeV")
            variable.add_qualifier("LUMINOSITY", 138, units=r"fb$^{-1}$")

        if len(uncertainties[i]) > 1:
            variable_unc = hepdata_lib.Uncertainty("Syst.", is_symmetric=True)
            diff = len(val_entry[1:]) - len(uncertainties[i])

            uncertainties[i].extend(diff * [0.])

            variable_unc.values = uncertainties[i]
            variable.add_uncertainty(variable_unc)

        table.add_variable(variable)
    

    table.keywords["cmenergies"] = ["13000.0"]
    table.keywords["observables"] = ["N"]
    table.keywords["phrases"] = ["Top", "Cross Section", "Inclusive"]
    table.keywords["reactions"] = ["P P --> TQ TQBAR TQ TQBAR", "P P --> TOP TOPBAR TOP TOPBAR"]

    return table
        

def read_table_json(submission, filepath):
    f = open(filepath)
    config = json.load(f)

    for key in config:
        table = latex_to_hepdata(config[key]["path"], config[key]["name"])
        table.location = config[key]["location"]
        submission.add_table(table)

    return



if __name__ == "__main__":
    submission = hepdata_lib.Submission()

    # read_table_json(submission, "/home/njovdnbo/Documents/Stacker_v2/hepdata/input_others/Tables.json")
    read_table_json(submission, "/home/njovdnbo/Documents/Stacker_v2/hepdata/input_others/tables_paper.json")
    submission.create_files("test_tables",remove_old=True)

    #table = latex_to_hepdata("/home/njovdnbo/Documents/FourTop/AN-21-182/tables/postfit/pfPlot_srFit_obs_bdt_DLee_sig_shapes_table.tex", "test")
#
    #submission = hepdata_lib.Submission()
    #submission.add_table(table)
    #submission.create_files("test_tables",remove_old=True)

