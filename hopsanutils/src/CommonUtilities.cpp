#include "hopsanutils/CommonUtilities.h"

#include <sstream>
//#include <iostream>

using namespace hopsan;

//! @brief Help function that splits a full path into basepath and filename
void splitFilePath(const std::string &fullPath, std::string &rBasePath, std::string &rFileName)
{
    size_t pos = fullPath.rfind('/');
    // If not found try a windows backslash instead
    if (pos == std::string::npos) {
        pos = fullPath.rfind('\\');
    }
    if (pos != std::string::npos) {
        rBasePath = fullPath.substr(0, pos+1) ;
        rFileName = fullPath.substr(pos+1);
    }
    else {
        rBasePath = "";
        rFileName = fullPath;
    }
}

//! @brief Help function that splits a filename into basename and extension
void splitFileName(const std::string &fileName, std::string &rBaseName, std::string &rExtension)
{
    size_t pos = fileName.rfind('.');
    if (pos != std::string::npos) {
        rBaseName = fileName.substr(0, pos) ;
        rExtension = fileName.substr(pos+1);
    }
    else {
        rExtension = "";
        rBaseName = fileName;
    }
}

void splitStringOnDelimiter(const std::string &string, char delim, std::vector<std::string> &rSplitVector)
{
    rSplitVector.clear();
    std::string item;
    std::stringstream ss(string);
    while(std::getline(ss, item, delim)) {
        rSplitVector.push_back(item);
    }
}

//! @brief Convert the fullpath into a path relative to basepath, both paths must be relative the same directoryo (or absolute)
std::string relativePath(std::string basePath, std::string fullPath)
{
    std::string result;

    if ( (basePath.size()>0) && basePath[basePath.size()-1] != '/')
    {
        basePath.push_back('/');
    }

    // First chew up the initial common part of the string
    size_t pe = basePath.find('/');
    while (pe != std::string::npos)
    {
        //cout << "basePath: " << basePath << endl;
        //cout << "fullPath: " << fullPath << endl;
        std::string sub = basePath.substr(0, pe+1);

        size_t fe = fullPath.find(sub);
        if (fe == 0)
        {
            basePath.erase(0, pe+1);
            fullPath.erase(0, sub.size());
            pe = basePath.find('/');
        }
        else
        {
            break;
        }
    }

    // Now determine the number of levels to "go up" based on remaning '/' in basePath
    pe = basePath.find('/');
    while (pe != std::string::npos)
    {
        result += "../";
        basePath.erase(0, pe+1);
        pe = basePath.find('/');
    }
    // Append what is remaning of base path
    result += fullPath;

    //cout << "Result: " << result << endl;

    return result;
}




