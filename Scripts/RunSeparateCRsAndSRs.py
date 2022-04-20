### run Stacker first for requested samples. Next get datacards and rewrite them in a way that removes signal/control regions
# is there a combine tool for this? -> Might be easier to get a list of SR's/CR's, put them in lists and those potentially in lists. Basically always start from our 4 uuncertainty files, copy until relevant lines are there and we change stuff
# For each of those setups, run stacker one by one. (multiple simultaneous for a single change in unc is certainly possible)
# next: on a set of DC_s, combine them and pull them trough a series of tasks in combine -> one condor job per task might be easiest cause don't know how to wait for job to finish



## Define here list of SRs and CRs to be checked. 
