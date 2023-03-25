#include "../interface/StringTools.h"

TString cleanTString(TString& toClean) {
    TString cleaned(toClean);

    if (stringContainsSubstr(toClean.Data(), "gamma")) {
        cleaned.ReplaceAll("gamma", "#gamma");
    } else {
        cleaned.ReplaceAll("gam", "#gamma");
    }
    cleaned.ReplaceAll("TTX", "t(#bar{t})+X");
    cleaned.ReplaceAll("TT", "t#bar{t}");
    cleaned.ReplaceAll("T(T)", "t(#bar{t})");
    cleaned.ReplaceAll("tt", "t#bar{t}");
    cleaned.ReplaceAll("VVV", "VV(V)");
    cleaned.ReplaceAll("_", " ");


    /*if (cleaned.Contains("e+")) {

    }*/

    // TODO

    return cleaned;
}

bool stringContains(std::string& str, char toFind) {
    if (str.find(toFind) != std::string::npos) {
        return true;
    } else {
        return false;
    }
}

bool stringContainsSubstr( const std::string& s, const std::string& substring ){
    return ( s.find( substring ) != std::string::npos );
}

void cleanWhitespace(std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    if (start == std::string::npos) {
        start = 0;
    }
    str = str.substr(start);
    size_t end = str.find_last_not_of(" \t");
    
    str = str.substr(0, end + 1);
}

std::string removeExt(std::string& str) {
    size_t splitPoint = str.find_last_of('.');
    if (splitPoint == std::string::npos) return str;
    
    return str.substr(0, splitPoint);
}

std::string getChannel(std::string& str) {
    size_t split = str.find_last_of("_");

    return str.substr(split+1);
}

std::string getFilename(std::string& str) {
    size_t split = str.find_last_of("/");
    if (split == std::string::npos) return str;
    return str.substr(split+1);
}

std::string splitAtUnderscore(std::string& str) {
    size_t split = str.find('_');

    return str.substr(split + 1);
}

std::string getYearFromRootFile(std::string& rootfile) {
    std::string year = removeExt(rootfile);
    size_t split = year.find_last_of('_');

    return year.substr(split + 1);
}

std::string replace( const std::string& s, const std::string& oldstring, const std::string& newstring ){
    std::string ret( s );
    std::string::size_type pos;
    while( ( pos = ret.find( oldstring ) ) != std::string::npos ){
        ret.erase( pos, oldstring.size() );
        ret.insert( pos, newstring );
    }
    return ret;
}

//remove all occurences of a substring from a string
std::string removeOccurencesOf( const std::string& s, const std::string& substring ){
    return replace( s, substring, "" );
}

std::vector< std::string > split( const std::string& s, const std::string& substring ){
    std::vector< std::string > ret;
    std::string remainingString( s );
    std::string::size_type pos;
    do{
        pos = remainingString.find( substring );
        ret.push_back( remainingString.substr( 0, pos ) );

        //if the split string occurs several times in a row all occurences should be removed
        while( ( pos != std::string::npos ) && ( remainingString.substr( pos + substring.size(), substring.size() ) == substring ) ){
            remainingString.erase( 0, pos + substring.size() );
            pos = remainingString.find( substring );
        }

        remainingString.erase( 0, pos + substring.size() );
    } while( pos != std::string::npos );

    return ret;
}