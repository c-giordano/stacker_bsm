import json

if __name__ == "__main__":
    file = "/user/nivanden/Stacker_v2/combineFiles/Variations/Results.json"

    significances = dict()
    output = []
    with open(file, 'r') as f:
        significances = json.load(f)

    for entry, sigString in significances.items():
        entryName = entry.split("/")[-1][3:-4]
        
        if "Combi" in entryName:
            entryName = entryName[6:]
        
        entryElements = entryName.split("_")
        
        outputSub = [entryElements[0], "_".join(entryElements[1:])]

        if not "20" in outputSub[0]:
            outputSub[0] = "20" + outputSub[0]
        significanceCleaned = sigString.split("\n")[0][14:]

        outputSub.append(significanceCleaned)
        output.append(outputSub)

    outputSorted = sorted(output, key=lambda x: x[0])

    outputCSV = ""
    outputLatex = ""

    for element in outputSorted:
        outputCSV += ", ".join(element) + "\n"
        outputLatex += " & ".join(element) + "$\\sigma$ \\\\" + "\n"

    
    print(outputCSV)
    print(outputLatex)
        

#    "2016PostVFP_crw": "Significance: 0.0222346\nDone in 0.00 min (cpu), 0.00 min (real)\n",
#    "Combi_16PostVFP_cro": "Significance: 0.0251812\nDone in 0.00 min (cpu), 0.00 min (real)\n",
