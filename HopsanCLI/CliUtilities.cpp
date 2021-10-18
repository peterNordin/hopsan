/*-----------------------------------------------------------------------------

 Copyright 2017 Hopsan Group

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.


 The full license is available in the file LICENSE.
 For details about the 'Hopsan Group' or information about Authors and
 Contributors see the HOPSANGROUP and AUTHORS files that are located in
 the Hopsan source code root directory.

-----------------------------------------------------------------------------*/

//!
//! @file   HopsanCLI/CliUtilities.cpp
//! @author peter.nordin@liu.se
//! @date   2014-12-09
//!
//! @brief Contains helpfunctions for CLI
//!
//$Id$

#include "CliUtilities.h"
#include "hopsanutils/CommonUtilities.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <cmath>

#if defined(_WIN32)
  #include <windows.h>
#elif defined(__APPLE__)
  #include <mach-o/dyld.h>
  #include <unistd.h>
#else
  #include <unistd.h>
  #include <limits.h>
#endif

using namespace std;

//! @brief Prints a message with red color (resets color to defaul after)
//! @param[in] rError The error message
void printErrorMessage(const std::string &rError, bool silent)
{
    if(silent) return;

    setTerminalColor(Red);
    cout << "Error: " << rError << endl;
    setTerminalColor(Reset);
}

//! @brief Prints a message with yellow color (resets color to defaul after)
//! @param[in] rWarning The warning message
void printWarningMessage(const std::string &rWarning, bool silent)
{
    if(silent) return;

    setTerminalColor(Yellow);
    cout << "Warning: " << rWarning << endl;
    setTerminalColor(Reset);
}

void printMessage(const std::string &rMessage, bool silent)
{
    if(silent) return;

    setTerminalColor(Reset);
    cout << rMessage << endl;
}

//! @brief Prints a message with green color (resets color to defaul after)
//! @param[in] color The text color for the message
//! @param[in] rMessage The message
void printColorMessage(const ColorsEnumT color, const std::string &rMessage, bool silent)
{
    if(silent) return;

    setTerminalColor(color);
    cout << rMessage << endl;
    setTerminalColor(Reset);
}

//! @brief Changes color on console output
//! @param color Color number (0-15)
//! @todo Need yellow color also
void setTerminalColor(const ColorsEnumT color)
{
#ifdef _WIN32
    WORD c;
    switch (color)
    {
    case Red:
        c = FOREGROUND_INTENSITY | FOREGROUND_RED;
        break;
    case Green:
        c = FOREGROUND_INTENSITY | FOREGROUND_GREEN;
        break;
    case Yellow:
        c = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_RED;
        break;
    case Blue:
        c = FOREGROUND_INTENSITY | FOREGROUND_BLUE;
        break;
    case White:
    default:
        c = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
    }

    HANDLE hcon = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hcon,c);
#else
    string c;
    switch (color)
    {
    case Red:
        c = "\e[0;91m";
        break;
    case Green:
        c = "\e[0;92m";
        break;
    case Yellow:
        c = "\e[0;93m";
        break;
    case Blue:
        c = "\e[0;94m";
        break;
    case White:
    default:
        c = "\e[0m";
    }

    cout << c;
#endif
}



//! @brief Helper function that returns the number of availible cores
size_t getNumAvailibleCores()
{
    size_t nCores = 1; // If non-Windows system, make sure there is at least one thread
#ifdef _WIN32
    // Obtain number of processor cores from environment variable
    if(getenv("NUMBER_OF_PROCESSORS") != 0)
    {
        string temp = getenv("NUMBER_OF_PROCESSORS");
        nCores = atoi(temp.c_str());
    }
#else
    nCores = max((long)1, sysconf(_SC_NPROCESSORS_ONLN));
#endif
    return nCores;
}

//! @brief Compares a vector with a reference vector
//! @param rVec Vector to compare
//! @param rRef Reference vector
//! @param tol (%) for acceptance
bool compareVectors(const std::vector<double> &rVec, const std::vector<double> &rRef, const double tol)
{
    if(rVec.size() != rRef.size())
    {
        printErrorMessage("compareVectors() Size mismatch!");
        return false;
    }

    for(size_t i=0; i<rVec.size(); ++i)
    {
        if( (fabs(1-rVec[i]/rRef[i]) > tol) && !(fabs(rVec[i])<1e-100) && !(fabs(rRef[i])<1e-10) )
        {
            //cout << "Test failed, Tol: " << tol << " comparing: " << rVec.at(i) << " with " << rRef.at(i) << " at index " << i << endl;
            return false;
        }
    }
    return true;
}

//! @brief Read the paths of external liubs from a text file
//! @param[in] filePath The file to read from
//! @param[out] rExtLibFileNames A vector with paths to the external libs to load
void readExternalLibsFromTxtFile(const std::string filePath, std::vector<std::string> &rExtLibFileNames)
{
    rExtLibFileNames.clear();
    std::string line;
    std::ifstream file;
    file.open(filePath.c_str());
    if ( file.is_open() )
    {
        while ( file.good() )
        {
            getline(file, line);
            if (*line.begin() != '#')
            {
                rExtLibFileNames.push_back(line);
            }
        }
        file.close();
    }
    else
    {
        printErrorMessage(string("Could not open externalLibsToLoadFile: ") + filePath );
    }
}


//! @brief Return the path to the current executable (HopsanCLI most likely)
//! @returns The path or empty string if path can not be found
string getCurrentExecPath()
{
#if defined(_WIN32)
    char result[ MAX_PATH ];
    size_t pathSize = GetModuleFileNameA( NULL, result, MAX_PATH );
#elif defined(__APPLE__)
    unsigned int pathSize;
    std::vector<char> buffer(1024+1);
    if(_NSGetExecutablePath(&buffer[0], &pathSize))
    {
        buffer.resize(pathSize);
        _NSGetExecutablePath(&buffer[0], &pathSize);
    }
    char* result = buffer.data();
#else
    char result[ PATH_MAX ];
    ssize_t pathSize = readlink( "/proc/self/exe", result, PATH_MAX );
#endif

    if (pathSize > 0)
    {
        string base, file;
        hopsan::splitFilePath(string(result, pathSize), base, file);
        //cout << "base, file: " << base << " " << file << endl;
        return base;
    }

    return "";
}
