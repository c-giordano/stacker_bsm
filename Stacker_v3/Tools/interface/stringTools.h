/*
methods for std::string formatting 
*/

#ifndef stringTools_h
#define stringTools_h


//include c++ library classes
#include <string>
#include <utility>
#include <vector>

namespace stringTools{

    //remove all trailing spaces and tabs 
    std::string removeBackSpaces( const std::string& s );

	//remove all leading spaces and tabs
    std::string removeFrontSpaces( const std::string&s );

	//remove all leading and trailing spaces and tabs
    std::string cleanSpaces( const std::string& s );

	//extract first word from string ( separated by spaces )
    std::string extractFirst( std::string& s );

    //add trailing / to directoryName if needed
    std::string formatDirectoryName( const std::string& );

    //check whether string contains substring
    bool stringContains( const std::string& s, const std::string& substring );

    //check whether string ends with substring 
    bool stringEndsWith( const std::string& s, const std::string& ending );

    //check whether string starts with substring
    bool stringStartsWith( const std::string& s, const std::string& beginning );

    //split file name and extentions
    std::pair< std::string, std::string > splitFileExtension( const std::string& );

    //return file name without extension
    std::string fileNameWithoutExtension( const std::string& );

    //split file base name and directory
    std::pair< std::string, std::string > splitDirectoryFileName( const std::string& );

    //return directory name 
    std::string directoryNameFromPath( const std::string& );

    //return file name 
    std::string fileNameFromPath( const std::string& );

    //convert double to string, with given precision
    std::string doubleToString(const double, const unsigned precision = 0);

    //replace all occurences of a substring with another string
    std::string replace( const std::string& s, const std::string& oldstring, const std::string& newstring );

    //remove occurences of substring 
    std::string removeOccurencesOf( const std::string& s, const std::string& substring );

    //split string on every occurence of the substring
    std::vector< std::string > split( const std::string& s, const std::string& substring );
    std::vector< std::string > split( const std::string& s, const std::vector< std::string >& substring );
    std::vector< std::string > split( const std::string& s, std::initializer_list< std::string > );
    std::vector< std::string > split( const std::string& s );

    // merge string
    std::string mergeString(const std::vector<std::string>& splitString, const std::string& substring);
}
#endif
