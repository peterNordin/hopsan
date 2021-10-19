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
//! @file   ModelUtilities.h
//! @brief Contains model specific help functions
//!
//$Id$

#ifndef MODELUTILITIES_H
#define MODELUTILITIES_H

#include <string>
#include <vector>

#include "hopsanutils/CommonUtilities.h"

#include "HopsanEssentials.h"

namespace hopsan {

// ===== Save Functions =====
enum SaveResults {Final, Full};
RV<> saveResultsToCSV(ComponentSystem *pRootSystem, const std::string &fileName, const std::vector<std::string>& includeFilter, SaveResults howMany);
RV<> saveResultsToHDF5(ComponentSystem *pRootSystem, const std::string &fileName, const std::vector<std::string>& includeFilter, SaveResults howMany);

void transposeCSVresults(const std::string &rFileName);
RV<> exportParameterValuesToCSV(const std::string &rFileName, ComponentSystem* pSystem, std::string prefix="", std::ofstream *pFile=0);

// ===== Load Functions =====
RV<> importParameterValuesFromCSV(const std::string &filePath, ComponentSystem* pSystem);
RV<> readNodesToSaveFromTxtFile(const std::string &filePath, std::vector<std::string> &rComponents, std::vector<std::string> &rPorts);

// ===== Help Functions =====
void generateFullSubSystemHierarchyName(const hopsan::ComponentSystem *pSystem, const hopsan::HString &separator, hopsan::HString &rFullSysName);
hopsan::HString generateFullSubSystemHierarchyName(const hopsan::Component *pComponent, const hopsan::HString &separator, bool includeLastSeparator=true);
hopsan::HString generateFullPortVariableName(const hopsan::Port *pPort, const size_t dataId);


RV<hopsan::Component*> getComponentWithFullName(hopsan::ComponentSystem *pRootSystem, const std::string &fullComponentName);
RV<hopsan::Port*> getPortWithFullName(hopsan::ComponentSystem *pRootSystem, const std::string &fullPortName);


// ===== Template Help Function =====
template<typename PortFunction>
void forEachPort(hopsan::ComponentSystem *pRootSystem, PortFunction function )
{
    auto components = pRootSystem->getSubComponents();
    for (auto& pComponent : components) {
        auto ports = pComponent->getPortPtrVector();
        for (auto& pPort : ports) {
            function(*pPort);
        }

        // Recurse into subsystems
        if (pComponent->isComponentSystem()) {
            forEachPort(dynamic_cast<hopsan::ComponentSystem*>(pComponent), function);
        }
    }
}

template <typename SaveTimeFunc, typename SaveVariableFunc>
void saveResultsTo(hopsan::ComponentSystem *pCurrentSystem, const std::vector<std::string>& includeFilter, SaveTimeFunc writeTime,
                   SaveVariableFunc writeVariable )
{
    hopsan::HString prefix = generateFullSubSystemHierarchyName(pCurrentSystem, "$"); //!< @todo not necessary every time if we use recursion instead

    // Use names to get components in alphabetical order
    const std::vector<hopsan::HString> componentNames = pCurrentSystem->getSubComponentNames();
    size_t numberOfLoggedComponentsInThisSystem = 0;
    for (const auto& name : componentNames) {
        hopsan::Component *pComponent = pCurrentSystem->getSubComponent(name);

        size_t numberOfLoggedVariablesInThisComponent = 0;
        const std::vector<hopsan::Port*> ports = pComponent->getPortPtrVector();
        for (const hopsan::Port* pPort : ports) {
            // Ignore ports that have logging disabled
            if (!pPort->isLoggingEnabled()) {
                continue;
            }

            hopsan::HString fullPortName = prefix + pComponent->getName() + "#" + pPort->getName();
            const bool includeAllVariablesInThisPort = (includeFilter.empty() || hopsan::contains(includeFilter, fullPortName.c_str())) &&
                    pPort->isLoggingEnabled();

            const std::vector<hopsan::NodeDataDescription> *pVariables = pPort->getNodeDataDescriptions(); // TODO do not return pointer
            if (pVariables)
            {
                for (size_t v=0; v<pVariables->size(); ++v)
                {
                    const hopsan::NodeDataDescription* pVariable = &pVariables->at(v);

                    // Create data vector
                    const std::vector< std::vector<double> > *pLogData = pPort->getLogDataVectorPtr();
                    if(pLogData == nullptr || pLogData->empty()) {
                        continue;
                    }

                    hopsan::HString fullVarName = fullPortName+"#"+pVariable->name;
                    const bool includeThisVariable = includeAllVariablesInThisPort || contains(includeFilter, fullVarName.c_str());
                    if (!includeThisVariable) {
                        continue;
                    }

                    // Add variable to exporter
                    writeVariable(pCurrentSystem, pComponent, pPort, v);

                    numberOfLoggedVariablesInThisComponent++;
                }
            }
        }
        if (numberOfLoggedVariablesInThisComponent > 0) {
            numberOfLoggedComponentsInThisSystem++;
        }

        // Recurse into subsystems
        if (pComponent->isComponentSystem()) {
            saveResultsTo(dynamic_cast<hopsan::ComponentSystem*>(pComponent), includeFilter, writeTime, writeVariable);
        }
    }

    // Save this sytems time vector
    if (numberOfLoggedComponentsInThisSystem > 0) {
        writeTime(pCurrentSystem);
    }
}

}

#endif // MODELUTILITIES_H
