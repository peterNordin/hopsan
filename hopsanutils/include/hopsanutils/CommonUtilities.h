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
//! @file   CommonUtilities.h
//! @brief Contains common utility functions
//!
//$Id$

#ifndef COMMONUTILITIES_H
#define COMMONUTILITIES_H

#include <string>
#include <vector>
#include <algorithm>

namespace hopsan {

// ===== Help Functions for String Paths =====
void splitFilePath(const std::string &fullPath, std::string &rBasePath, std::string &rFileName);
void splitFileName(const std::string &fileName, std::string &rBaseName, std::string &rExtension);
void splitStringOnDelimiter(const std::string &string, char delim, std::vector<std::string> &rSplitVector);
std::string relativePath(std::string basePath, std::string fullPath);

// ===== Template Help Function =====
template <typename ContainerT, typename ValueT>
bool contains(const ContainerT& container, const ValueT& value) {
    auto it = std::find(container.begin(), container.end(), value);
    return (it != container.end());
}


}

#endif // COMMONUTILITIES_H
