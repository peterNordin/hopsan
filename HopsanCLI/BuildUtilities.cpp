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


//$Id$

#include "BuildUtilities.h"

#include "CoreUtilities/GeneratorHandler.h"
#include "CliUtilities.h"

#include <iostream>

using namespace std ;

bool buildComponentLibrary(const std::string &rLibraryXML, std::string &rOutput)
{
    hopsan::GeneratorHandler *pHandler = new hopsan::GeneratorHandler();
    if(pHandler->isLoadedSuccessfully())
    {
        string path = getCurrentExecPath();
        // Expected include and bin path for HopsanCore
        string hopsanIncludePath = path+"../HopsanCore/include/";
        string hopsanBinPath = path;

        pHandler->callComponentLibraryCompiler(rLibraryXML.c_str(), "", "", hopsanIncludePath.c_str(), hopsanBinPath.c_str(), "", true);
        delete(pHandler);
        return true;
    }
    delete(pHandler);
    return false;
}
