import glob
import shutil

if __name__ == "__main__":
    basedir = "/user/nivanden/Stacker_v2/combineFiles/Variations/Impacts/"
    baseOutDir  = "/user/nivanden/public_html/datacards/Variations/"
    files = glob.glob("/user/nivanden/Stacker_v2/combineFiles/Variations/Impacts/*/impacts.pdf")

    for file in files:
        subdirName = file.split("/")[-2]

        shutil.copy2(basedir + subdirName + "/impacts.pdf", baseOutDir+subdirName+".pdf")

