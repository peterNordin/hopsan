/*-----------------------------------------------------------------------------
 This source file is part of Hopsan NG

 Copyright (c) 2011 
    Mikael Axin, Robert Braun, Alessandro Dell'Amico, Björn Eriksson,
    Peter Nordin, Karl Pettersson, Petter Krus, Ingo Staack

 This file is provided "as is", with no guarantee or warranty for the
 functionality or reliability of the contents. All contents in this file is
 the original work of the copyright holders at the Division of Fluid and
 Mechatronic Systems (Flumes) at Linköping University. Modifying, using or
 redistributing any part of this file is prohibited without explicit
 permission from the copyright holders.
-----------------------------------------------------------------------------*/

//!
//! @file   ComponentSystem.cc
//! @author FluMeS
//! @date   2009-12-20
//!
//! @brief Contains the subsystem component class and connection assistant help class
//!
//$Id$

#include <sstream>
#include <cassert>
#include <limits>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <algorithm>

#include "ComponentSystem.h"
#include "CoreUtilities/HopsanCoreMessageHandler.h"
#include "CoreUtilities/StringUtilities.h"
#include "HopsanEssentials.h"
#include "CoreUtilities/MultiThreadingUtilities.h"
#include "CoreUtilities/CoSimulationUtilities.h"
#include "CoreUtilities/HmfLoader.h"
#include "ComponentUtilities/num2string.hpp"

#ifdef USETBB
#include "tbb/parallel_for.h"
#endif

using namespace std;
using namespace hopsan;

bool SimulationHandler::initializeSystem(const double startT, const double stopT, ComponentSystem* pSystem)
{
    if (pSystem->checkModelBeforeSimulation())
    {
        return pSystem->initialize(startT, stopT);
    }
    return false;
}

bool SimulationHandler::initializeSystem(const double startT, const double stopT, std::vector<ComponentSystem*> &rSystemVector)
{
    //No multicore init support
    bool isOk = true;
    for (size_t i=0; i<rSystemVector.size(); ++i)
    {
        isOk = isOk && initializeSystem(startT, stopT, rSystemVector[i]);
        if (!isOk)
        {
            break;
        }
    }
    return isOk;
}

bool SimulationHandler::simulateSystem(const double startT, const double stopT, const int nDesiredThreads, ComponentSystem* pSystem, bool noChanges, ParallelAlgorithmT algorithm)
{
    if (nDesiredThreads < 0)
    {
        pSystem->addInfoMessage("Using single-threaded algorithm.");
        pSystem->simulate(stopT);
    }
    else
    {
        pSystem->simulateMultiThreaded(startT, stopT, nDesiredThreads, noChanges, algorithm);
    }

    return !pSystem->wasSimulationAborted();
}

bool SimulationHandler::simulateSystem(const double startT, const double stopT, const int nDesiredThreads, std::vector<ComponentSystem*> &rSystemVector, bool noChanges, ParallelAlgorithmT algorithm)
{
    if (rSystemVector.size() > 1)
    {
        if (nDesiredThreads >= 0)
        {
            return simulateMultipleSystemsMultiThreaded(startT, stopT, nDesiredThreads, rSystemVector, noChanges);
        }
        else
        {
            return simulateMultipleSystems(stopT, rSystemVector);
        }
    }
    else if (rSystemVector.size() == 1)
    {
        return simulateSystem(startT, stopT, nDesiredThreads, rSystemVector[0], noChanges, algorithm);
    }

    return false;
}

void SimulationHandler::finalizeSystem(ComponentSystem* pSystem)
{
    pSystem->finalize();
}

void SimulationHandler::finalizeSystem(std::vector<ComponentSystem*> &rSystemVector)
{
    //No multicore finalize
    for (size_t i=0; i<rSystemVector.size(); ++i)
    {
        finalizeSystem(rSystemVector[i]);
    }
}

void SimulationHandler::runCoSimulation(ComponentSystem *pSystem)
{
    (void)pSystem;
#ifdef USEBOOST

    std::vector<HString> inputComponents;
    std::vector<HString> inputPorts;
    std::vector<int> inputData;

    std::vector<HString> outputComponents;
    std::vector<HString> outputPorts;
    std::vector<int> outputData;

    //////////////////////////

    std::vector<HString> names = pSystem->getSubComponentNames();
    for(size_t i=0; i<names.size(); ++i)
    {
        Component *pComponent = pSystem->getSubComponent(names[i]);
        if(pComponent->getTypeName() == "SignalInputInterface")
        {
            inputComponents.push_back(names[i]);
            inputPorts.push_back("out");
            inputData.push_back(0);
        }
        else if(pComponent->getTypeName() == "SignalOutputInterface")
        {
            outputComponents.push_back(names[i]);
            outputPorts.push_back("in");
            outputData.push_back(0);
        }
    }

    //////////////////////////

    std::vector<double*> inputSockets;
    std::vector<double*> outputSockets;

    //Initialize shared memory sockets

    //Simulate
    double *sim_socket = getDoubleSharedMemoryPointer("hopsan_sim");

    //Stop
    bool *stop_socket = getBoolSharedMemoryPointer("hopsan_stop");


    //Input
    for(int i=0; i<inputData.size(); ++i)
    {
        std::stringstream ss;
        ss << "hopsan_in" << i;
        inputSockets.push_back(getDoubleSharedMemoryPointer(ss.str().c_str()));
    }


    //Output
    for(int i=0; i<inputData.size(); ++i)
    {
        std::stringstream ss;
        ss << "hopsan_out" << i;
        outputSockets.push_back(getDoubleSharedMemoryPointer(ss.str().c_str()));
    }

    //Initialize simulation
    //! @todo We must be able to log data without knowing the stop time
    pSystem->initialize(0, 100);

    (*stop_socket) = false;
    (*sim_socket) = 0;

    (*inputSockets[0]) = 34.125;

    while(!(*stop_socket))
    {
        if((*sim_socket)>5)
        {
            //Set input variables
            for(int i=0; i<inputSockets.size(); ++i)
            {
                pSystem->getSubComponent(inputComponents[i])->getPort(inputPorts[i])->writeNode(inputData[i], (*inputSockets[i]));
            }

            //Simulate one step
            double time = pSystem->getTime();
            double timestep = pSystem->getDesiredTimeStep();
            pSystem->simulate(time, time+timestep);

            //Write back output variables
            for(int i=0; i<outputSockets.size(); ++i)
            {
                (*outputSockets[i]) = pSystem->getSubComponent(outputComponents[i])->getPort(outputPorts[i])->readNode(outputData[i]);
            }

            //Reset simulation flag
            (*sim_socket) = 0;
        }
    }

#endif
}

//! @brief Distributes component system pointers evenly over one vector per thread, depending on their simulation time
//! @param[in] rSystemVector Vector to distribute
//! @param[in] nThreads Number of threads to distribute for
vector< vector<ComponentSystem *> > SimulationHandler::distributeSystems(const std::vector<ComponentSystem *> &rSystemVector, size_t nThreads)
{
    vector< vector<ComponentSystem *> > splitSystemVector;
    vector<double> timeVector;

    nThreads = min(nThreads, rSystemVector.size()); //Prevent adding for more threads then systems
    splitSystemVector.resize(nThreads);
    timeVector.resize(nThreads,0);
    size_t sysNum=0;
    while(true)         //! @todo Poor algorithm for distributing, will not give optimal results
    {
        for(size_t t=0; t<nThreads; ++t)
        {
            if(sysNum == rSystemVector.size())
                break;
            splitSystemVector[t].push_back(rSystemVector[sysNum]);
            timeVector[t] += rSystemVector[sysNum]->getMeasuredTime();
            ++sysNum;
        }
        if(sysNum == rSystemVector.size())
            break;
    }
    return splitSystemVector;
}

//Constructor
ComponentSystem::ComponentSystem() : Component(), mAliasHandler(this)
{
    mTypeName = "ComponentSystem";
    mTypeCQS = Component::UndefinedCQSType;
    mName = mTypeName; //Make sure initial name is same as typename
    mWarnIfUnusedSystemParameters = true;
    mDesiredTimestep = 0.001;
    mInheritTimestep = true;
    mKeepStartValues = false;
    mRequestedNumLogSamples = 0; //This has to be 0 since we want logging to be disabled by default
    mRequestedLogStartTime = 0;
#ifdef USETBB
    mpStopMutex = new tbb::mutex();
#else
    mpStopMutex = 0;
#endif

    // Set default (disabled) values for log data
    disableLog();
}


ComponentSystem::~ComponentSystem()
{
    // Clear the contents of the system
    clear();
#ifdef USETBB
    delete mpStopMutex;
#endif
}

void ComponentSystem::configure()
{
    //Does nothing
}

Component::CQSEnumT ComponentSystem::getTypeCQS() const
{
    return mTypeCQS;
}

double ComponentSystem::getDesiredTimeStep() const
{
    return mDesiredTimestep;
}

//! @brief Set the desired number of log samples
void ComponentSystem::setNumLogSamples(const size_t nLogSamples)
{
    mRequestedNumLogSamples = nLogSamples;
}

double ComponentSystem::getLogStartTime() const
{
    return mRequestedLogStartTime;
}

void ComponentSystem::setLogStartTime(const double logStartTime)
{
    mRequestedLogStartTime = logStartTime;
}

//! @brief Returns the desired number of log samples
size_t ComponentSystem::getNumLogSamples() const
{
    return mRequestedNumLogSamples;
}

//! @brief Returns the number of actually logged data samples
//! @return Number of available logged data samples in storage
size_t ComponentSystem::getNumActuallyLoggedSamples() const
{
    // This assumes that the logCtr has been incremented after each saved log step
    return mLogCtr;
}


//! @brief Set the stop simulation flag to abort the initialization or simulation loops
//! @param[in] rReason An optional HString describing the reason for the stop
//! This method can be used by users e.g. GUIs to stop an start a initialization/simulation process
void ComponentSystem::stopSimulation(const HString &rReason)
{
    if (rReason.empty())
    {
        addInfoMessage("Simulation was stopped at t="+to_hstring(mTime), "StopSimulation");
    }
    else
    {
        addInfoMessage("Simulation was stopped at t="+to_hstring(mTime)+ " : "+rReason, "StopSimulation");
    }
#ifdef USETBB
    mpStopMutex->lock();
    mStopSimulation = true;
    mpStopMutex->unlock();
#else
    mStopSimulation = true;
#endif
    // Now propagate stop signal upwards, to parent
    if (mpSystemParent)
    {
        mpSystemParent->stopSimulation(""); // We use string version here to make sure sub system hierarchy is printed
    }
}

//! @brief Set the stop simulation flag to abort the initialization or simulation loops, (without messages being added)
//! @todo maybe we should only have with messages version
void ComponentSystem::stopSimulation()
{
#ifdef USETBB
    mpStopMutex->lock();
    mStopSimulation = true;
    mpStopMutex->unlock();
#else
    mStopSimulation = true;
#endif
    // Now propagate stop signal upwards, to parent
    if (mpSystemParent != 0)
    {
        mpSystemParent->stopSimulation();
    }
}

//! @brief Check if the simulation was aborted
//! @returns true if Initialize, Simulate, or Finalize was aborted
bool ComponentSystem::wasSimulationAborted()
{
    return mStopSimulation;
}


//! @brief Adds a search path that can be used by its components to look for external files, e.g. area curves
//! @param [in] rSearchPath The search path to be added
void ComponentSystem::addSearchPath(const HString &rSearchPath)
{
    HString fixedSearchString;
    fixedSearchString = rSearchPath;
    if (!fixedSearchString.empty())
    {
        while( (!fixedSearchString.empty()) && ((fixedSearchString.back() == '/') || (fixedSearchString.back() == '\\')) )
        {
            fixedSearchString = fixedSearchString.substr(0,fixedSearchString.size()-1);
        }
    }

    bool contain = false;
    for(size_t i=0; i<mSearchPaths.size();++i)
    {
        if(mSearchPaths[i] == fixedSearchString)
            contain = true;
    }
    if(!contain)
        mSearchPaths.push_back(fixedSearchString);
}


//! @todo this one (if it should even exist) should be in component as parameter map is there, best is if we can code around having one
ParameterEvaluatorHandler &ComponentSystem::getSystemParameters()
{
    return *mpParameters;
}

//!
bool ComponentSystem::setSystemParameter(const HString &rName, const HString &rValue, const HString &rType, const HString &rDescription, const HString &rUnit, const bool force)
{
    bool success;
    if(mpParameters->hasParameter(rName.c_str()))
    {
        success = mpParameters->setParameter(rName.c_str(), rValue.c_str(), rDescription.c_str(), rUnit.c_str(), rType.c_str(), force);
    }
    else
    {
        if (this->hasReservedUniqueName(rName) || !isNameValid(rName, "#"))
        {
            addErrorMessage("The desired system parameter name: "+rName+" is invalid or already used by somthing else");
            success=false;
        }
        else
        {
            success = mpParameters->addParameter(rName.c_str(), rValue.c_str(), rDescription.c_str(), rUnit.c_str(), rType.c_str(), 0, force);
            if (success)
            {
                reserveUniqueName(rName,UniqueSysparamNameType);
            }
        }
    }

    return success;
}

void ComponentSystem::unRegisterParameter(const HString &rName)
{
    Component::unRegisterParameter(rName);
    unReserveUniqueName(rName);
}

//! @brief Add multiple components to the system
void ComponentSystem::addComponents(std::vector<Component*> &rComponents)
{
    std::vector<Component*>::iterator itx;
    for(itx = rComponents.begin(); itx != rComponents.end(); ++itx)
    {
        addComponent(*itx);
    }
}

//! @brief Add a component to the system
void ComponentSystem::addComponent(Component *pComponent)
{
    // Prevent adding null ptr
    if (pComponent)
    {
        // First check if the name already exists, in that case change the suffix
        HString modname = this->reserveUniqueName(pComponent->getName(), UniqueComponentNameType);
        pComponent->setName(modname);

        // Add to the cqs component vectors
        addSubComponentPtrToStorage(pComponent);

        // Set system parent and model system depth hierarchy
        pComponent->setSystemParent(this);
        pComponent->mModelHierarchyDepth = mModelHierarchyDepth+1; //Set the ModelHierarchyDepth counter

        // Go through the components ports and take ownership of any dummy nodes
        //! @todo what happens if I take ownership of an other systems components (shouldn't we take ownership of all ports nodes by default) Not sure!! especially difficult with system border nodes
        //! @todo maybe node ownership should be decided early in initialize instead to make this less complicated
        std::vector<Port*> ports = pComponent->getPortPtrVector();
        for (size_t i=0; i<ports.size(); ++i)
        {
            if (ports[i]->getPortType() < MultiportType)
            {
                // Dummy nodes have only one port (the port itself)
                if (ports[i]->getNodePtr()->getNumConnectedPorts() == 1)
                {
                    this->addSubNode(ports[i]->getNodePtr());
                }
            }
        }

        if (pComponent->isExperimental())
        {
            pComponent->addWarningMessage("This component is experimental!", "ExperimentalTag");
        }
        if (pComponent->isObsolete())
        {
            pComponent->addWarningMessage("This component is obsolete and will be removed in the future!", "ObsoleteTag");
        }
    }
    else
    {
        addErrorMessage("Trying to add NULL component to system");
    }
}


//! @brief Rename a sub component and automatically fix unique names
void ComponentSystem::renameSubComponent(const HString &rOldName, const HString &rNewName)
{
    // First find the post in the map where the old name resides, copy the data stored there
    SubComponentMapT::iterator it = mSubComponentMap.find(rOldName);
    if (it != mSubComponentMap.end())
    {
        // If found, erase old record
        Component* pTempComp = it->second;
        mSubComponentMap.erase(it);

        // Insert new (with new name)
        HString mod_new_name = this->reserveUniqueName(rNewName, UniqueComponentNameType);
        this->unReserveUniqueName(rOldName);
        mSubComponentMap.insert(pair<HString, Component*>(mod_new_name, pTempComp));

        // Rename aliases
        mAliasHandler.componentRenamed(rOldName, mod_new_name);

        // Now change the actual component name, without trying to do rename (we are in rename now, would cause infinite loop)
        pTempComp->mName = mod_new_name;
    }
    else
    {
        addErrorMessage("No component with old_name: "+rOldName+" found when renaming!");
    }
}


//! @brief Remove a dub component from a system, can also be used to actually delete the component
//! @param[in] rName The name of the component to remove from the system
//! @param[in] doDelete Set this to true if the component should be deleted after removal
void ComponentSystem::removeSubComponent(const HString &rName, bool doDelete)
{
    Component* pComp = getSubComponent(rName);
    removeSubComponent(pComp, doDelete);
}


//! @brief Remove a sub component from a system, can also be used to actually delete the component
//! @param[in] pComponent A pointer to the component to remove
//! @param[in] doDelete Set this to true if the component should be deleted after removal
void ComponentSystem::removeSubComponent(Component* pComponent, bool doDelete)
{
   HString compName = pComponent->getName();

    // Disconnect all ports before erase from system
    PortPtrMapT::iterator ports_it;
    vector<Port*>::iterator conn_ports_it;
    for (ports_it = pComponent->mPortPtrMap.begin(); ports_it != pComponent->mPortPtrMap.end(); ++ports_it)
    {
        //! @todo what about multiports here
        vector<Port*> connected_ports = ports_it->second->getConnectedPorts(); //Get a copy of the connected ports ptr vector
        //We can not use an iterator directly connected to the vector inside the port as this will be changed by the disconnect calls
        for (conn_ports_it = connected_ports.begin(); conn_ports_it != connected_ports.end(); ++conn_ports_it)
        {
            disconnect(ports_it->second, *conn_ports_it);
        }
    }

    // Remove any component aliases
    mAliasHandler.componentRemoved(pComponent->getName());

    // Remove from storage
    removeSubComponentPtrFromStorage(pComponent);

    // Remove any dummy node ptrs
    //! @todo (shouldn't we remove ownership of all port nodes by default) Not sure!! especially difficult with system border nodes
    std::vector<Port*> ports = pComponent->getPortPtrVector();
    for (size_t i=0; i<ports.size(); ++i)
    {
        if (ports[i]->getPortType() < MultiportType)
        {
            if (ports[i]->getNodePtr()->getNumConnectedPorts() == 1)
            {
                this->removeSubNode(ports[i]->getNodePtr());
            }
        }
    }


    // Shall we also delete the component completely
    if (doDelete)
    {
        mpHopsanEssentials->removeComponent(pComponent);
    }

    // Unreserve the name
    unReserveUniqueName(compName);

    addDebugMessage("Removed component: \"" + compName + "\" from system: \"" + this->getName() + "\"", "removedcomponent");
}

//! @brief Reserves a unique name in the system
//! @param [in] rDesiredName The desired name to reserve
//! @param [in] type The type of entity that the unique name represents
//! @returns The actual name reserved
HString ComponentSystem::reserveUniqueName(const HString &rDesiredName, const UniqeNameEnumT type)
{
    HString newname = this->determineUniqueComponentName(rDesiredName);
    mTakenNames.insert(std::pair<HString, UniqeNameEnumT>(newname, type));
    return newname;
}

//! @brief unReserves a unique name in the system
//! @param [in] rName The name to unreserve
void ComponentSystem::unReserveUniqueName(const HString &rName)
{
    mTakenNames.erase(rName);
}

void ComponentSystem::addSubComponentPtrToStorage(Component* pComponent)
{
    switch (pComponent->getTypeCQS())
    {
    case Component::CType :
        mComponentCptrs.push_back(pComponent);
        break;
    case Component::QType :
        mComponentQptrs.push_back(pComponent);
        break;
    case Component::SType :
        mComponentSignalptrs.push_back(pComponent);
        break;
    case Component::UndefinedCQSType :
        mComponentUndefinedptrs.push_back(pComponent);
        break;
    default :
        addErrorMessage("Trying to add module with unspecified CQS type: "+pComponent->getTypeCQSString()+", (Not added)");
        return;
    }

    mSubComponentMap.insert(pair<HString, Component*>(pComponent->getName(), pComponent));
}

void ComponentSystem::removeSubComponentPtrFromStorage(Component* pComponent)
{
    SubComponentMapT::iterator it = mSubComponentMap.find(pComponent->getName());
    if (it != mSubComponentMap.end())
    {
        vector<Component*>::iterator cit; //Component iterator
        switch (it->second->getTypeCQS())
        {
        case Component::CType :
            for (cit = mComponentCptrs.begin(); cit != mComponentCptrs.end(); ++cit)
            {
                if ( *cit == pComponent )
                {
                    mComponentCptrs.erase(cit);
                    break;
                }
            }
            break;
        case Component::QType :
            for (cit = mComponentQptrs.begin(); cit != mComponentQptrs.end(); ++cit)
            {
                if ( *cit == pComponent )
                {
                    mComponentQptrs.erase(cit);
                    break;
                }
            }
            break;
        case Component::SType :
            for (cit = mComponentSignalptrs.begin(); cit != mComponentSignalptrs.end(); ++cit)
            {
                if ( *cit == pComponent )
                {
                    mComponentSignalptrs.erase(cit);
                    break;
                }
            }
            break;
        case Component::UndefinedCQSType :
            for (cit = mComponentUndefinedptrs.begin(); cit != mComponentUndefinedptrs.end(); ++cit)
            {
                if ( *cit == pComponent )
                {
                    mComponentUndefinedptrs.erase(cit);
                    break;
                }
            }
            break;
        default :
            addFatalMessage("In removeSubComponentPtrFromStorage(): Component is not of CType, QType, SType or UndefinedCQSType.");
        }
        mSubComponentMap.erase(it);
    }
    else
    {
        addErrorMessage("The component you are trying to remove: "+pComponent->getName()+" does not exist (Does Nothing)");
    }
}

//! @brief Clear all the contents of a system (deleting any remaining components and connections)
void ComponentSystem::clear()
{
    // Remove and delete every subcomponent, one by one
    while (!mSubComponentMap.empty())
    {
        removeSubComponent((*mSubComponentMap.begin()).second, true);
    }
}


//! @brief Sorts a component vector
//! Components are sorted so that they are always simulated after the components they receive signals from. Algebraic loops can be detected, in that case this function does nothing.
bool ComponentSystem::sortComponentVector(std::vector<Component*> &rComponentVector)
{
    std::vector<Component*> newComponentVector;

    bool didSomething = true;
    while(didSomething)
    {
        didSomething = false;
        std::vector<Component*>::iterator it;
        for(it=rComponentVector.begin(); it!=rComponentVector.end(); ++it)  //Loop through the unsorted signal component vector
        {
            if(!componentVectorContains(newComponentVector, (*it)))    //Ignore components that are already added to the new vector
            {
                bool readyToAdd=true;
                std::vector<Port*>::iterator itp;
                std::vector<Port*> portVector = (*it)->getPortPtrVector();
                for(itp=portVector.begin(); itp!=portVector.end(); ++itp) //Ask each port for its node, then ask the node for its write port component
                {
                    Component* requiredComponent=0;
                    if(((*itp)->getPortType() == ReadPortType || (*itp)->getPortType() == ReadMultiportType ||
                        ((*it)->getTypeName() == "Subsystem" && (*itp)->getInternalPortType() == ReadPortType)) && (*itp)->isConnected())
                    {
                        requiredComponent = (*itp)->getNodePtr()->getWritePortComponentPtr();
                    }
                    if(requiredComponent != 0 && requiredComponent->getTypeName() != "SignalUnitDelay")
                    {
                        if(requiredComponent->mpSystemParent == this)
                        {
                            if(!componentVectorContains(newComponentVector, requiredComponent) &&
                               componentVectorContains(rComponentVector, requiredComponent)
                               /*requiredComponent->getTypeCQS() == (*itp)->getComponent()->getTypeCQS()*//*Component::S*/)
                            {
                                readyToAdd = false;     //Depending on normal component which has not yet been added
                            }
                        }
                        else
                        {
                            if(!componentVectorContains(newComponentVector, requiredComponent->mpSystemParent) &&
                               requiredComponent->mpSystemParent->getTypeCQS() == (*itp)->getComponent()->getTypeCQS() &&
                               componentVectorContains(rComponentVector,requiredComponent->mpSystemParent))
                            {
                                readyToAdd = false;     //Depending on subsystem component which has not yer been added
                            }
                        }
                    }
                }
                if(readyToAdd)  //Add the component if all required write port components was already added
                {
                    newComponentVector.push_back((*it));
                    didSomething = true;
                }
            }
        }
    }

    if(newComponentVector.size() == rComponentVector.size())   //All components moved to new vector = success!
    {
        rComponentVector = newComponentVector;
        if(newComponentVector.size() > 0 && newComponentVector[0]->getTypeCQS() == SType)
        {
            HString ss;
            std::vector<Component*>::iterator it;
            for(it=newComponentVector.begin(); it!=newComponentVector.end(); ++it)
            {
                ss += (*it)->getName()+"\n";                                                                                               //DEBUG
            }
            addDebugMessage("Sorted components successfully!\nSignal components will be simulated in the following order:\n" + ss);
        }
    }
    else    //Something went wrong, all components were not moved. This is likely due to an algebraic loop.
    {
        addErrorMessage("Initialize: Algebraic loops was found, signal components could not be sorted.");
        if(!newComponentVector.empty())
            addInfoMessage("Last component that was successfully sorted: " + newComponentVector.back()->getName());
        addInfoMessage("Initialize: Hint: Use unit delay components to resolve loops.");
        return false;
    }

    return true;
}


//! @brief Figures out whether or not a component vector contains a certain component
bool ComponentSystem::componentVectorContains(std::vector<Component*> vector, Component *pComp)
{
    std::vector<Component*>::iterator it;
    for(it=vector.begin(); it!=vector.end(); ++it)
    {
        if((*it) == pComp)
        {
            return true;
        }
    }
    return false;
}


//! @brief Overloaded function that behaves slightly different when determining unique port names
//! In systemcomponents we must make sure that systemports and subcomponents have unique names, this simplifies things in the GUI later on
//! It is VERY important that systemports don't have the same name as a subcomponent
HString ComponentSystem::determineUniquePortName(const HString &rPortname)
{
    return this->reserveUniqueName(rPortname, UniqueSysportNameTyp);
}

//! @brief Overloaded function that behaves slightly different when determining unique component names
//! In systemcomponents we must make sure that systemports and subcomponents have unique names, this simplifies things in the GUI later on
//! It is VERY important that systemports don't have the same name as a subcomponent
//! @todo the determineUniquePortNAme and ComponentName looks VERY similar maybe we could use the same function for both
HString ComponentSystem::determineUniqueComponentName(const HString &rName) const
{
    return findUniqueName<TakenNamesMapT>(mTakenNames, rName);
}

bool ComponentSystem::hasReservedUniqueName(const HString &rName) const
{
    return (mTakenNames.find(rName) != mTakenNames.end());
}


//! @brief Get a Component ptr to the component with supplied name, can also return a ptr to self if no subcomponent found but systemport with name found
//! @details For this to work we need to make sure that the sub components and systemports have unique names
Component* ComponentSystem::getSubComponentOrThisIfSysPort(const HString &rName)
{
    // First try to find among subcomponents
    Component *tmp = getSubComponent(rName);
    if (tmp == 0)
    {
        // Now try to find among systemports
        Port* pPort = this->getPort(rName);
        if (pPort != 0)
        {
            if (pPort->getPortType() == SystemPortType)
            {
                // Return the systemports owner (the system component)
                tmp = pPort->getComponent();
            }
        }
    }
    return tmp;
}


Component* ComponentSystem::getSubComponent(const HString &rName) const
{
    SubComponentMapT::const_iterator it = mSubComponentMap.find(rName);
    if (it != mSubComponentMap.end())
    {
        return it->second;
    }
    else
    {
        addLogMess("ComponentSystem::getSubComponent(): The requested component does not exist.");
        return 0;
    }
}

const std::vector<Component *> ComponentSystem::getSubComponents() const
{
    vector<Component *> ptrs;
    SubComponentMapT::const_iterator it;
    for (it = mSubComponentMap.begin(); it != mSubComponentMap.end(); ++it)
    {
        ptrs.push_back(it->second);
    }

    return ptrs;
}


ComponentSystem* ComponentSystem::getSubComponentSystem(const HString &rName) const
{
    return dynamic_cast<ComponentSystem*>(getSubComponent(rName));
}


std::vector<HString> ComponentSystem::getSubComponentNames() const
{
    //! @todo for now create a vector of the component names, later maybe we should return a pointer to the real internal map
    vector<HString> names;
    SubComponentMapT::const_iterator it;
    for (it = mSubComponentMap.begin(); it != mSubComponentMap.end(); ++it)
    {
        names.push_back(it->first);
    }

    return names;
}

//! @brief Check if a system has a subcomponent with given name
//! @param [in] rName The name to check for
//! @returns true or false
bool ComponentSystem::haveSubComponent(const HString &rName) const
{
    return (mSubComponentMap.count(rName) > 0);
}

//! @brief Checks if a system is empty (if there are no components or systemports)
bool ComponentSystem::isEmpty() const
{
    return ((mSubComponentMap.size() + mPortPtrMap.size()) == 0);
}

AliasHandler &ComponentSystem::getAliasHandler()
{
    return mAliasHandler;
}


//! @brief Add a node as subnode in the system, if the node is already owned by someone else, transfer ownership to this system
void ComponentSystem::addSubNode(Node* pNode)
{
    if (pNode->getOwnerSystem() != 0)
    {
        pNode->getOwnerSystem()->removeSubNode(pNode);
    }
    mSubNodePtrs.push_back(pNode);
    pNode->mpOwnerSystem = this;
}


//! @brief Removes a previously added node
void ComponentSystem::removeSubNode(Node* pNode)
{
    vector<Node*>::iterator it;
    for (it=mSubNodePtrs.begin(); it!=mSubNodePtrs.end(); ++it)
    {
        if (*it == pNode)
        {
            pNode->mpOwnerSystem = 0;
            mSubNodePtrs.erase(it);
            break;
        }
    }
}


//! @brief preAllocates log space (to speed up later access for log writing)
void ComponentSystem::preAllocateLogSpace()
{
    bool success = true;
//    //cout << "stopT = " << stopT << ", startT = " << startT << ", mTimestep = " << mTimestep << endl;
//    this->setLogSettingsNSamples(nSamples, startT, stopT, mTimestep);
    //! @todo Fix /Peter
    mLogCtr = 0;
    if (mEnableLogData)
    {
        try
        {
            mTimeStorage.resize(mnLogSlots, 0);

            // Allocate log data memory for subnodes
            //! @todo we should have an other vector with those nodes that should be logged, if we make individual nodes possible to disable logging
            vector<Node*>::iterator it;
            for (it=mSubNodePtrs.begin(); it!=mSubNodePtrs.end(); ++it)
            {
                // Abort if we are told to stop or if memory allocation fails
                if (mStopSimulation || !success)
                    break;

                // Prepare the node log data allocation and determine if loggings should be on
                //! @todo What if we want to use one of the other ways of setting logsample time steps

                // Now try to allocate log memory for each node
                try
                {
                    // If the node is in a read port and if that port is not connected (node only have one connected port)
                    // Then we should disable logging for that node as logging the startvalue does not make sense
                    if ( ((*it)->getNumConnectedPorts() < 2) && ((*it)->getNumberOfPortsByType(ReadPortType) == 1) )
                    {
                        (*it)->setLoggingEnabled(false);
                    }
                    else
                    {
                        (*it)->setLoggingEnabled(true);
                        (*it)->preAllocateLogSpace(mnLogSlots);
                    }
                    success = true;
                }
                catch (exception &e)
                {
                    //cout << "preAllocateLogSpace: Standard exception: " << e.what() << endl;
                    addErrorMessage("Failed to allocate log data memmory, try reducing the amount of log data", "FailedMemmoryAllocation");
                    (*it)->setLoggingEnabled(false);
                    success = false;
                }
            }
        }
        catch (exception &e)
        {
            addErrorMessage("Failed to allocate log data memmory, try reducing the amount of log data", "FailedMemmoryAllocation");
            disableLog();
            success = false;
        }
    }

    // If we failed to allocate log memory then stop simulation
    if (!success)
    {
        mStopSimulation = true;
    }
}


void ComponentSystem::logTimeAndNodes(const size_t simStep)
{
    if (mEnableLogData)
    {
        if (mLogTheseTimeSteps[mLogCtr] ==  simStep)
        {
            mTimeStorage[mLogCtr] = mTime;   //We log the "real"  simulation time for the sample

            //! @todo we should have an other vector with those nodes that should be logged, if we make individual nodes possible to disable logging
            vector<Node*>::iterator it;
            for (it=mSubNodePtrs.begin(); it!=mSubNodePtrs.end(); ++it)
            {
                (*it)->logData(mLogCtr);
            }
            ++mLogCtr;
        }
    }
}


//! @brief Rename a system parameter
bool ComponentSystem::renameParameter(const HString &rOldName, const HString &rNewName)
{
    if (hasReservedUniqueName(rNewName))
    {
        addErrorMessage("The desired system parameter name: "+rNewName+" is already used");
    }
    else if (mpParameters->renameParameter(rOldName, rNewName))
    {
        unReserveUniqueName(rOldName);
        reserveUniqueName(rNewName);
        return true;
    }
    return false;
}

//! @brief Adds a transparent SubSystemPort
Port* ComponentSystem::addSystemPort(HString portName, const HString &rDescription)
{
    // Force default portname p, if nothing else specified
    if (portName.empty())
    {
        portName = "p";
    }

    return addPort(portName, SystemPortType, "NodeEmpty", rDescription, Port::Required);
}


//! @brief Rename system port
HString ComponentSystem::renameSystemPort(const HString &rOldname, const HString &rNewname)
{
    HString newmodename = renamePort(rOldname,rNewname);
    if (newmodename != rOldname)
    {
        unReserveUniqueName(rOldname);
    }
    return newmodename;
}


//! @brief Delete a System port from the component
//! @param [in] rName The name of the port to delete
void ComponentSystem::deleteSystemPort(const HString &rName)
{
    deletePort(rName);
    unReserveUniqueName(rName);
}


//! @brief Set the type C, Q, or S of the subsystem
void ComponentSystem::setTypeCQS(CQSEnumT cqs_type, bool doOnlyLocalSet)
{
    //! @todo should really try to figure out a better way to do this
    //! @todo need to do error checking, and make sure that the specified type really is valid, first and last component should be of this type (i think)

    //If type same as before do nothing
    //if (cqs_type !=  mTypeCQS)
    //{
        // If we have a system parent, then tell it to change our CQS type
        if ( !this->isTopLevelSystem() && !doOnlyLocalSet )
        {
            //Request change by our parent (some parent changes are needed)
            mpSystemParent->changeSubComponentSystemTypeCQS(mName, cqs_type);
        }
        else
        {
            switch (cqs_type)
            {
            case Component::CType :
                mTypeCQS = Component::CType;
                for(size_t i=0; i<mPortPtrVector.size(); ++i)   //C-type, create start node for all power ports
                {
                    if(mPortPtrVector[i]->getInternalPortType() == PowerPortType)
                    {
                        Node *pStartNode = mPortPtrVector[i]->getStartNodePtr();
                        if (!pStartNode || pStartNode->getNodeType() == "NodeEmpty")
                        {
                            mPortPtrVector[i]->createStartNode(mPortPtrVector[i]->getNodeType());
                        }
                    }
                }
                break;

            case Component::QType :
                mTypeCQS = Component::QType;
                for(size_t i=0; i<mPortPtrVector.size(); ++i)   //Q-type, remove start node for all powerports
                {
                    if(mPortPtrVector[i]->getInternalPortType() == PowerPortType)
                    {
                        mPortPtrVector[i]->eraseStartNode();
                    }
                }
                break;

            case Component::SType :
                mTypeCQS = Component::SType;
                for(size_t i=0; i<mPortPtrVector.size(); ++i)   //S-type, remove start node for all powerports
                {
                    if(mPortPtrVector[i]->getInternalPortType() == PowerPortType)
                    {
                        mPortPtrVector[i]->eraseStartNode();
                    }
                }
                break;

            case Component::UndefinedCQSType :
                mTypeCQS = Component::UndefinedCQSType;
                break;

            default :
                addWarningMessage("Specified type: "+getTypeCQSString()+" does not exist!, System CQStype unchanged");
            }
        }
   //}
}

//! @brief Change the cqs type of a stored subsystem component
bool ComponentSystem::changeSubComponentSystemTypeCQS(const HString &rName, const CQSEnumT newType)
{
    //First get the componentsystem ptr and check if we are requesting new type
    ComponentSystem* tmpptr = getSubComponentSystem(rName);
    if (tmpptr != 0)
    {
        // If the ptr was not = 0 then we have found a subsystem, lets change the type
        //if (newType != tmpptr->getTypeCQS())
        //{
            //Remove old version
            this->removeSubComponentPtrFromStorage(tmpptr);

            //Change cqsType locally in the subcomponent, make sure to set true to avoid looping back to this rename
            tmpptr->setTypeCQS(newType, true);

            //re-add to system
            this->addSubComponentPtrToStorage(tmpptr);
        //}
        return true;
    }
    return false;
}

//! @brief This function automatically determines the CQS type depending on the what has been connected to the systemports
//! @todo This function will go through all connected ports every time it is run, maybe a quicker version would only be run on the port being connected or disconnected, in the connect and disconnect function
void ComponentSystem::determineCQSType()
{
    size_t c_ctr=0;
    size_t q_ctr=0;
    size_t s_ctr=0;

    PortPtrMapT::iterator ppmit;
    for (ppmit=mPortPtrMap.begin(); ppmit!=mPortPtrMap.end(); ++ppmit)
    {
        //! @todo I don't think that I really need to ask for ALL connected subports here, as it is actually only the component that is directly connected to the system port that is interesting
        //! @todo this means that I will be able to UNDO the Port getConnectedPorts madness, maybe, if we don't want it in some other place
        vector<Port*> connectedPorts = (*ppmit).second->getConnectedPorts(-1); //Make a copy of connected ports
        vector<Port*>::iterator cpit;
        for (cpit=connectedPorts.begin(); cpit!=connectedPorts.end(); ++cpit)
        {
            if ( (*cpit)->getComponent()->getSystemParent() == this )
            {
                if( (*cpit)->getPortType() == ReadPortType || (*cpit)->getPortType() == WritePortType)
                {
                    ++s_ctr;
                    continue;
                }

                switch ((*cpit)->getComponent()->getTypeCQS())
                {
                case CType :
                    ++c_ctr;
                    break;
                case QType :
                    ++q_ctr;
                    break;
                case SType :
                    ++s_ctr;
                    break;
                default :
                    ;
                    //Do nothing, (connecting a port from a system with no cqs type set yet)
                }
            }
        }
    }

    // Ok now lets determine if we have a valid CQS type or not
    if ( (c_ctr > 0) && (q_ctr == 0) )
    {
        setTypeCQS(CType);
    }
    else if ( (q_ctr > 0) && (c_ctr == 0) )
    {
        setTypeCQS(QType);
    }
    else if ( (s_ctr > 0) && (c_ctr==0) && (q_ctr==0) )
    {
        setTypeCQS(SType);
    }
    else
    {
        setTypeCQS(UndefinedCQSType);
    }
}

bool ComponentSystem::isTopLevelSystem() const
{
    return (mpSystemParent==0);
}


//! @brief Connect two components, string version
//! @param [in] compname1 The name of the first component
//! @param [in] portname1 The name of the port on the first component
//! @param [in] compname2 The name of the second component
//! @param [in] portname2 The name of the port on the second component
//! @returns True if success else False
bool ComponentSystem::connect(const HString &compname1, const HString &portname1, const HString &compname2, const HString &portname2)
{
    // Check if the components exist (and can be found)
    Component* pComp1 = getSubComponentOrThisIfSysPort(compname1);
    Component* pComp2 = getSubComponentOrThisIfSysPort(compname2);

    if (pComp1 == 0)
    {
        addErrorMessage("Component1: '"+compname1+"' can not be found when attempting connect", "connectwithoutcomponent");
        return false;
    }

    if (pComp2 == 0)
    {
        addErrorMessage("Component2: '"+compname2+"' can not be found when attempting connect", "connectwithoutcomponent");
        return false;
    }

    // Check if components have specified ports
    Port *pPort1, *pPort2;
    if (!pComp1->getPort(portname1, pPort1))
    {
        addErrorMessage("Component: '"+pComp1->getName()+"' does not have a port named '"+portname1+"'", "portdoesnotexist");
        return false;
    }

    if (!pComp2->getPort(portname2, pPort2)) //Not else if because pPort2 has to be set in getPort
    {
        addErrorMessage("Component: '"+pComp2->getName()+"' does not have a port named '"+portname2+"'", "portdoesnotexist");
        return false;
    }

    // Ok components and ports exist, lets attempt the connect
    return connect( pPort1, pPort2 );
}


bool ConnectionAssistant::ensureSameNodeType(Port *pPort1, Port *pPort2)
{
    // Check if both ports have the same node type specified
    if (pPort1->getNodeType() != pPort2->getNodeType())
    {
        HString ss;
        ss+="You can not connect a {"+pPort1->getNodeType()+"} port to a {"+pPort2->getNodeType()+"} port."+
              " When connecting: {"+pPort1->getComponent()->getName()+"::"+pPort1->getName()+"} to {"+pPort2->getComponent()->getName()+"::"+pPort2->getName()+"}";
        mpComponentSystem->addErrorMessage(ss);
        return false;
    }
    return true;
}

//! @note requires that input ports are not multiports (they can be subports in multiports)
bool ConnectionAssistant::mergeNodeConnection(Port *pPort1, Port *pPort2)
{
    if (!ensureSameNodeType(pPort1, pPort2))
    {
        return false;
    }

    Node *pOldNode1 = pPort1->getNodePtr();
    Node *pOldNode2 = pPort2->getNodePtr();

    // Check for very rare occurrence, (Looping a subsystem, and connecting an out port to an in port that are actually directly connected to each other)
    if (pOldNode1 == pOldNode2)
    {
        mpComponentSystem->addErrorMessage("This connection would mean that a node is joined with it self, this does not make any sense and is not allowed");
        return false;
    }

    // Create a new node and recursively set in all ports
    Node *pNewNode = mpComponentSystem->getHopsanEssentials()->createNode(pPort1->getNodeType().c_str());
    recursivelySetNode(pPort1, 0, pNewNode);
    recursivelySetNode(pPort2, 0, pNewNode);

    // Let the ports know about each other
    pPort1->addConnectedPort(pPort2);
    pPort2->addConnectedPort(pPort1);

    // Now delete the old nodes
    removeNode(pOldNode1);
    removeNode(pOldNode2);

    // Update the node placement
    determineWhereToStoreNodeAndStoreIt(pNewNode);

    if (ensureConnectionOK(pNewNode, pPort1, pPort2))
    {
        return true;
    }
    else
    {
        splitNodeConnection(pPort1, pPort2); //Undo connection
        return false;
    }
}

//! @brief Find the system highest up in the model hierarchy for the ports connected to this node and store the node there
//! @param[in] pNode The node to store
void ConnectionAssistant::determineWhereToStoreNodeAndStoreIt(Node* pNode)
{
    // Node ptr should not be zero
    if(pNode == 0)
    {
        mpComponentSystem->addFatalMessage("ConnectionAssistant::determineWhereToStoreNodeAndStoreIt(): Node pointer is zero.");
        return;
    }

    Component *pMinLevelComp=0;
    //size_t min = (size_t)-1;
    size_t min = std::numeric_limits<size_t>::max();
    vector<Port*>::iterator pit;
    for (pit=pNode->mConnectedPorts.begin(); pit!=pNode->mConnectedPorts.end(); ++pit)
    {
        if ((*pit)->getComponent()->getModelHierarchyDepth() < min)
        {
            min = (*pit)->getComponent()->getModelHierarchyDepth();
            pMinLevelComp = (*pit)->getComponent();
        }
    }

    // Now add the node to the system owning the minimum level component
    if (pMinLevelComp)
    {
        if (pMinLevelComp->getSystemParent())
        {
            pMinLevelComp->getSystemParent()->addSubNode(pNode);
        }
        else if (pMinLevelComp->isComponentSystem())
        {
            // This will trigger if we are connecting to our parent system which happens to be the top level system
            ComponentSystem *pRootSystem = dynamic_cast<ComponentSystem*>(pMinLevelComp);
            pRootSystem->addSubNode(pNode);
        }
        else
        {
            mpComponentSystem->addFatalMessage("ConnectionAssistant::determineWhereToStoreNodeAndStoreIt(): No system found for node storage!");
        }
    }
    else
    {
        mpComponentSystem->addFatalMessage("ConnectionAssistant::determineWhereToStoreNodeAndStoreIt(): No system found!");
    }

//    //! @todo what if we are connecting only subsystems within the same level AND they have different timesteps
//    if (pMinLevelComp==0)
//    {
//        mpComponentSystem->addSubNode(pNode);
//    }
//    else if (pMinLevelComp->isComponentSystem())
//    {
//        // If minimum level component is a system (we are connecting to our system parant), dyncast the pointer
//        ComponentSystem *pParentSystem = dynamic_cast<ComponentSystem*>(pMinLevelComp);
//        pParentSystem->addSubNode(pNode);
//    }
//    else
//    {
//        pMinLevelComp->getSystemParent()->addSubNode(pNode);
//    }
}

void ConnectionAssistant::recursivelySetNode(Port *pPort, Port *pParentPort, Node *pNode)
{
    pPort->setNode(pNode);
    vector<Port*>::iterator pit;
    vector<Port*> conn_ports = pPort->getConnectedPorts();
    for (pit=conn_ports.begin(); pit!=conn_ports.end(); ++pit)
    {
        //don't recurse back to parent will get stuck in infinite recursion
        if (*pit == pParentPort)
        {
            continue;
        }
        recursivelySetNode(*pit, pPort, pNode);
    }
}

Port* ConnectionAssistant::findMultiportSubportFromOtherPort(const Port *pMultiPort, Port *pOtherPort)
{
    if(pOtherPort->getPortType() >= MultiportType)
    {
        mpComponentSystem->addFatalMessage("ConnectionAssistant::findMultiportSubportFromOtherPort(): Other port shall not be a multiport.");
        return 0;
    }

    std::vector<Port*> otherConnPorts = pOtherPort->getConnectedPorts();
    for (size_t i=0; i<otherConnPorts.size(); ++i)
    {
        // We assume that a port can not be connected multiple times to the same multiport
        if (otherConnPorts[i]->mpParentPort == pMultiPort)
        {
            return otherConnPorts[i];
        }
    }
    return 0;
}


//! @note Requires that the input ports are not multiports
bool ConnectionAssistant::splitNodeConnection(Port *pPort1, Port *pPort2)
{
    if ((pPort1==0) || (pPort2==0))
    {
        mpComponentSystem->addFatalMessage("splitNodeConnection(): One of the ports is NULL");
        return false;
    }

    Node *pOldNode = pPort1->getNodePtr();
    Node *pNewNode1 = mpComponentSystem->getHopsanEssentials()->createNode(pOldNode->getNodeType().c_str());
    Node *pNewNode2 = mpComponentSystem->getHopsanEssentials()->createNode(pOldNode->getNodeType().c_str());

    // Make the ports forget about each other, If the ports becomes empty the nodes will be reset
    pPort1->eraseConnectedPort(pPort2);
    pPort2->eraseConnectedPort(pPort1);

    // Recursively set new nodes
    recursivelySetNode(pPort1, 0, pNewNode1);
    recursivelySetNode(pPort2, 0, pNewNode2);

    // Remove the old node
    removeNode(pOldNode);

    // Now determine what system should own the node
    determineWhereToStoreNodeAndStoreIt(pNewNode1);
    determineWhereToStoreNodeAndStoreIt(pNewNode2);

    return true;
}


ConnectionAssistant::ConnectionAssistant(ComponentSystem *pComponentSystem)
{
    mpComponentSystem = pComponentSystem;
}

//! Helpfunction that clears the nodetype in empty systemports, It will not clear the type if the port is not empty or if the port is not a systemport
void ConnectionAssistant::clearSysPortNodeTypeIfEmpty(Port *pPort)
{
    if ( (pPort->getPortType() == SystemPortType) && (!pPort->isConnected()) )
    {
        Node *pOldNode = pPort->getNodePtr();
        pPort->setNode(mpComponentSystem->getHopsanEssentials()->createNode("NodeEmpty"));
        removeNode(pOldNode);
        pPort->mNodeType = "NodeEmpty";
    }
}

//! @brief Connect two components with specified ports to each other
//! @param [in] pPort1 A pointer to the first port
//! @param [in] pPort2 A pointer to the second port
//! @returns True if success, False if failed
bool ComponentSystem::connect(Port *pPort1, Port *pPort2)
{
    if ((pPort1==0) || (pPort2==0))
    {
        addErrorMessage("Trying to connect NULL port(s)", "nullport");
        return false;
    }

    // Prevent connection with self
    if (pPort1 == pPort2)
    {
        addErrorMessage("You can not connect a port to it self", "selfconnection");
        return false;
    }

    ConnectionAssistant connAssist(this);
    Component* pComp1 = pPort1->getComponent();
    Component* pComp2 = pPort2->getComponent();
    bool sucess=false;

    // Prevent connection between two multiports
    //! @todo we might want to allow this in the future, right now disconnecting two multiports is also not implemented
    if ( pPort1->isMultiPort() && pPort2->isMultiPort() )
    {
        addErrorMessage("You are not allowed to connect two MultiPorts to each other, (this may be allowed in the future)");
        return false;
    }


    // Prevent connection if ports are already connected to each other, but we make an exception if one of the ports is aread multiport (to allow connecting same signal multiple times to scopes)
    if (!((pPort1->getPortType() == ReadMultiportType) || ((pPort2->getPortType() == ReadMultiportType))) && pPort1->isConnectedTo(pPort2) )
    {
        addErrorMessage("Port: " + pComp1->getName()+"::"+pPort1->getName() + "  is already connected to: " + pComp2->getName()+"::"+pPort2->getName());
        return false;
    }

    // Prevent cross connection between systems
    if (!connAssist.ensureNotCrossConnecting(pPort1, pPort2))
    {
        addErrorMessage("You can not cross-connect between systems", "crossconnection");
        return false;
    }

    // Prevent connection of two blank systemports
    if ( (pPort1->getPortType() == SystemPortType) && (pPort2->getPortType() == SystemPortType) )
    {
        if ( (!pPort1->isConnected()) && (!pPort2->isConnected()) )
        {
            addErrorMessage("You are not allowed to connect two blank systemports to each other");
            return false;
        }
    }

    // Prevent connection of readport to multiport, (what do you really want to read problem)
    if (pPort1->isMultiPort() || pPort2->isMultiPort())
    {
        if ( (pPort1->getPortType() == ReadPortType) || (pPort2->getPortType() == ReadPortType) )
        {
            addErrorMessage("You are not allowed to connect a readport to a multiport, (undefined what you will actually read). Connect to an ordinary port instead.");
            return false;
        }
    }

    // Now lets find out if one of the ports is a blank systemport
    //! @todo better way to find out if systemports are blank might give more clear code
    if ( ( (pPort1->getPortType() == SystemPortType) && (!pPort1->isConnected()) ) || ( (pPort2->getPortType() == SystemPortType) && (!pPort2->isConnected()) ) )
    {
        // Now lets find out which of the ports that is a blank systemport
        Port *pBlankSysPort=0;
        Port *pOtherPort=0;

        //! @todo write help function
        if ( (pPort1->getPortType() == SystemPortType) && (!pPort1->isConnected()) )
        {
            pBlankSysPort = pPort1;
            pOtherPort = pPort2;
        }
        else if ( (pPort2->getPortType() == SystemPortType) && (!pPort2->isConnected()) )
        {
            pBlankSysPort = pPort2;
            pOtherPort = pPort1;
        }

        pBlankSysPort->mNodeType = pOtherPort->getNodeType(); //set the nodetype in the sysport

        // Check if we are connecting multiports, in that case add new subport, remember original portPointer though so that we can clean up if failure
        Port *pActualPort = connAssist.ifMultiportAddSubport(pOtherPort);

        sucess = connAssist.mergeNodeConnection(pBlankSysPort, pActualPort);

        // Handle multiport connection success or failure
        connAssist.ifMultiportCleanupAfterConnect(pOtherPort, pActualPort, sucess);
    }
    // Non of the ports  are blank systemports
    else
    {
        // Check if we are connecting multiports, in that case add new subport, remember original portPointer though so that we can clean up if failure
        Port *pActualPort1 = connAssist.ifMultiportAddSubport(pPort1);
        Port *pActualPort2 = connAssist.ifMultiportAddSubport(pPort2);

        sucess = connAssist.mergeNodeConnection(pActualPort1, pActualPort2);

        // Handle multiport connection success or failure
        connAssist.ifMultiportCleanupAfterConnect(pPort1, pActualPort1, sucess);
        connAssist.ifMultiportCleanupAfterConnect(pPort2, pActualPort2, sucess);
    }

    // Abort connection if there was a connect failure
    if (!sucess)
    {
        return false;
    }

    // Update the CQS type
    this->determineCQSType();

    // Update parent cqs-type
    //! @todo we should only do this if we are actually connected directly to our parent, but I don't know what will take the most time, to check if we are connected to parent or to just refresh parent
    if (!this->isTopLevelSystem())
    {
        mpSystemParent->determineCQSType();
    }

    addDebugMessage("Connected: {"+pComp1->getName()+"::"+pPort1->getName()+"} and {"+pComp2->getName()+"::"+pPort2->getName()+"}", "succesfulconnect");
    return true;
}



bool ConnectionAssistant::ensureConnectionOK(Node *pNode, Port *pPort1, Port *pPort2)
{
//    size_t nReadPorts = 0;
//    size_t nWritePorts = 0;
//    size_t nPowerPorts = 0;
//    size_t nSystemPorts = 0;
//    size_t nOwnSystemPorts = 0; // Number of systemports that belong to the connecting system
//    size_t nInterfacePorts = 0; // This can be system ports or other ports acting as interface ports in systems
//    //size_t n_MultiPorts = 0;

//    size_t nCComponents = 0;
//    size_t nQComponents = 0;
//    size_t nSYScomponentCs = 0;
//    size_t nSYScomponentQs = 0;

//    size_t nNonInterfaceQPowerPorts = 0;
//    size_t nNonInterfaceCPowerPorts = 0;
    ConnOKCounters counters;

    //Count the different kind of ports and C,Q components in the node
    vector<Port*>::iterator it;
    for (it=(*pNode).mConnectedPorts.begin(); it!=(*pNode).mConnectedPorts.end(); ++it)
    {
//        if ((*it)->isInterfacePort())
//        {
//            counters.nInterfacePorts += 1;
//        }

//        if ((*it)->getPortType() == ReadPortType)
//        {
//            counters.nReadPorts += 1;
//        }
//        else if ((*it)->getPortType() == WritePortType)
//        {
//            counters.nWritePorts += 1;
//        }
//        else if ((*it)->getPortType() == PowerPortType)
//        {
//            counters.nPowerPorts += 1;
//            if ((*it)->getComponent()->isComponentC())
//            {
//                counters.nNonInterfaceCPowerPorts += 1;
//            }
//            else if ((*it)->getComponent()->isComponentQ())
//            {
//                counters.nNonInterfaceQPowerPorts += 1;
//            }
//        }
//        else if ((*it)->getPortType() == SystemPortType)
//        {
//            counters.nSystemPorts += 1;
//            if ((*it)->getComponent() == mpComponentSystem)
//            {
//                counters.nOwnSystemPorts += 1;
//            }
//        }
////        else if((*it)->getPortType() > MULTIPORT)
////        {
////            counters.n_MultiPorts += 1;
////        }
//        if ((*it)->getComponent()->isComponentC())
//        {
//            counters.nCComponents += 1;
//            if ((*it)->getComponent()->isComponentSystem())
//            {
//                counters.nSYScomponentCs += 1;
//            }
//        }
//        else if ((*it)->getComponent()->isComponentQ())
//        {
//            counters.nQComponents += 1;
//            if ((*it)->getComponent()->isComponentSystem())
//            {
//                counters.nSYScomponentQs += 1;
//            }
//        }

        checkPort(*it, counters);

        // Also count how many own systemports are already connected
        //! @todo maybe this counter should always be counted in checkPort()
        if ((*it)->getPortType() == SystemPortType)
        {
            if ((*it)->getComponent() == mpComponentSystem)
            {
                counters.nOwnSystemPorts += 1;
            }
        }
    }

    //Check the kind of ports in the components subjected for connection
    //Don't count port if it is already connected to node as it was counted in the code above (avoids double counting)
    if ( !pNode->isConnectedToPort(pPort1) )
    {
        checkPort(pPort1, counters);
//        if (pPort1->isInterfacePort())
//        {
//            nInterfacePorts += 1;
//        }

//        if ( pPort1->getPortType() == ReadPortType )
//        {
//            nReadPorts += 1;
//        }
//        if ( pPort1->getPortType() == WritePortType )
//        {
//            nWritePorts += 1;
//        }
//        if ( pPort1->getPortType() == PowerPortType )
//        {
//            nPowerPorts += 1;
//            //if ((*it)->getComponent()->isComponentC())
//            if (pPort1->getComponent()->isComponentC())
//            {
//                nNonInterfaceCPowerPorts += 1;
//            }
//            //else if ((*it)->getComponent()->isComponentQ())
//            else if (pPort2->getComponent()->isComponentQ())
//            {
//                nNonInterfaceQPowerPorts += 1;
//            }
//        }
//        if ( pPort1->getPortType() == SystemPortType )
//        {
//            nSystemPorts += 1;
//        }
////        if( pPort1->getPortType() > MULTIPORT)
////        {
////            n_MultiPorts += 1;
////        }
//        if ( pPort1->getComponent()->isComponentC() )
//        {
//            nCComponents += 1;
//            if ( pPort1->getComponent()->isComponentSystem() )
//            {
//                nSYScomponentCs += 1;
//            }
//        }
//        if ( pPort1->getComponent()->isComponentQ() )
//        {
//            nQComponents += 1;
//            if ( pPort1->getComponent()->isComponentSystem() )
//            {
//                nSYScomponentQs += 1;
//            }
//        }
    }

    //Don't count port if it is already connected to node as it was counted in the code above (avoids double counting)
    if ( !pNode->isConnectedToPort(pPort2) )
    {
        checkPort(pPort2, counters);
//        if (pPort2->isInterfacePort())
//        {
//            nInterfacePorts += 1;
//        }

//        if ( pPort2->getPortType() == ReadPortType )
//        {
//            nReadPorts += 1;
//        }
//        if ( pPort2->getPortType() == WritePortType )
//        {
//            nWritePorts += 1;
//        }
//        if ( pPort2->getPortType() == PowerPortType )
//        {
//            nPowerPorts += 1;
//            //if ((*it)->getComponent()->isComponentC())
//            if (pPort2->getComponent()->isComponentC())
//            {
//                nNonInterfaceCPowerPorts += 1;
//            }
//            //else if ((*it)->getComponent()->isComponentQ())
//            else if (pPort2->getComponent()->isComponentQ())
//            {
//                nNonInterfaceQPowerPorts += 1;
//            }
//        }
//        if ( pPort2->getPortType() == SystemPortType )
//        {
//            nSystemPorts += 1;
//        }
//        if ( pPort2->getComponent()->isComponentC() )
//        {
//            nCComponents += 1;
//            if ( pPort2->getComponent()->isComponentSystem() )
//            {
//                nSYScomponentCs += 1;
//            }
//        }
//        if ( pPort2->getComponent()->isComponentQ() )
//        {
//            nQComponents += 1;
//            if ( pPort2->getComponent()->isComponentSystem() )
//            {
//                nSYScomponentQs += 1;
//            }
//        }
    }

    //  Check if there are some problems with the connection

    if ((counters.nPowerPorts > 0) && (counters.nOwnSystemPorts > 1))
    {
        mpComponentSystem->addErrorMessage("Trying to connect one powerport to two systemports, this is not allowed");
        return false;
    }
//    if(n_MultiPorts > 1)
//    {
//        addErrorMessage("Trying to connect two MultiPorts to each other");
//        return false;
//    }
    if (counters.nPowerPorts > 2+counters.nInterfacePorts-counters.nSystemPorts)
    {
        mpComponentSystem->addErrorMessage("Trying to connect more than two PowerPorts to same node");
        return false;
    }
    if (counters.nWritePorts > 1+counters.nInterfacePorts-counters.nSystemPorts)
    {
        mpComponentSystem->addErrorMessage("Trying to connect more than one WritePort to same node");
        return false;
    }
    if ((counters.nPowerPorts > 0) && (counters.nWritePorts > 0))
    {
        mpComponentSystem->addErrorMessage("Trying to connect WritePort and PowerPort to same node");
        return false;
    }
    //! @todo maybe this only readport check should give a warning, but only if we do Strict check mode (maybe send in a bool for that), but we want to allow it when loading in case connectors are saved in the wrong order
//    if ((n_PowerPorts == 0) && (n_WritePorts == 0) && (n_SystemPorts == 0))
//    {
//        cout << "Trying to connect only ReadPorts" << endl;
//        mpComponentSystem->addErrorMessage("Trying to connect only ReadPorts");
//        return false;
//    }

    //cout << "nQ: " << n_Qcomponents << " nC: " << n_Ccomponents << endl;

    // We want at most one C and one Q component in a connection
    //! @todo not 100% sure that this will work always. Only work if we assume that the subsystem has the correct cqs type when connecting
    if (counters.nNonInterfaceCPowerPorts > 1)
    {
        mpComponentSystem->addErrorMessage("You can not connect two C-Component power ports to each other");
        return false;
    }
    if (counters.nNonInterfaceQPowerPorts > 1)
    {
        mpComponentSystem->addErrorMessage("You can not connect two Q-Component power ports to each other");
        return false;
    }

//    if( ((pPort1->getPortType() == Port::READPORT) && pPort2->getPortType() == Port::POWERPORT && n_PowerPorts > 1) or
//        ((pPort2->getPortType() == Port::READPORT) && pPort1->getPortType() == Port::POWERPORT && n_PowerPorts > 1) )
//    {
//        addErrorMessage("Trying to connect one ReadPort to more than one PowerPort");
//        return false;
//    }

    // It seems to be OK!
    return true;
}

bool ConnectionAssistant::ensureNotCrossConnecting(Port *pPort1, Port *pPort2)
{
    // Check so that both components to connect have been added to the same system (or we are connecting to parent system)
    if ( (pPort1->getComponent()->getSystemParent() != pPort2->getComponent()->getSystemParent()) )
    {
        if ( (pPort1->getComponent()->getSystemParent() != pPort2->getComponent()) && (pPort2->getComponent()->getSystemParent() != pPort1->getComponent()) )
        {
            mpComponentSystem->addErrorMessage("The components, {"+pPort1->getComponentName()+"} and {"+pPort2->getComponentName()+"}, "+"must belong to the same subsystem");
            return false;
        }
    }
    return true;
}

//! @brief Detects if a port is a multiport and then adds and returns a subport
//! @param [in] pMaybeMultiport A pointer to the port that may be a multiport
//! @returns A pointer to a new subport in the multiport, or the pMaybeMultiport itself if it was not a multiport
Port *ConnectionAssistant::ifMultiportAddSubport(Port *pMaybeMultiport)
{
    // If the port is a multiport then create a new subport and then return it (as the actual port)
    if (pMaybeMultiport->getPortType() >= MultiportType)
    {
        return pMaybeMultiport->addSubPort();
    }

    // As the port was not a multiport lets return it
    return pMaybeMultiport;
}

void ConnectionAssistant::ifMultiportPrepareDissconnect(Port *pMaybeMultiport1, Port *pMaybeMultiport2, Port *&rpActualPort1, Port *&rpActualPort2)
{
    if ((pMaybeMultiport1->getPortType() >= MultiportType) && (pMaybeMultiport2->getPortType() >= MultiportType))
    {
        mpComponentSystem->addFatalMessage("ifMultiportFindActualPort():Both ports can not be multiports");
        rpActualPort1 = 0;
        rpActualPort2 = 0;
        return;
    }

    // if pMaybeMultiport1 is a multiport, but not other port
    if (pMaybeMultiport1->getPortType() >= MultiportType)
    {
        rpActualPort1 = findMultiportSubportFromOtherPort(pMaybeMultiport1, pMaybeMultiport2);
        rpActualPort2 = pMaybeMultiport2;
        if(rpActualPort1 == 0)
        {
            mpComponentSystem->addFatalMessage("ifMultiportFindActualPort(): pActualPort1 == 0");
        }
    }


    // if pMaybeMultiport2 is a multiport, but not other port
    if (pMaybeMultiport2->getPortType() >= MultiportType)
    {
        rpActualPort1 = pMaybeMultiport1;
        rpActualPort2 = findMultiportSubportFromOtherPort(pMaybeMultiport2, pMaybeMultiport1);
        if(rpActualPort2 == 0)
        {
            mpComponentSystem->addFatalMessage("ifMultiportFindActualPort(): pActualPort2 == 0");
        }
    }
}

void ConnectionAssistant::ifMultiportCleanupAfterConnect(Port *pMaybeMultiport, Port *pActualPort, const bool wasSucess)
{
    if (pMaybeMultiport == pActualPort->getParentPort())
    {
        if (wasSucess)
        {
            //! @todo What do we need to do to handle success
        }
        else
        {
            //We need to remove the last created subport
            pMaybeMultiport->removeSubPort(pActualPort);
        }
    }
}

void ConnectionAssistant::ifMultiportCleanupAfterDissconnect(Port *pMaybeMultiport, Port *pActualPort, const bool wasSucess)
{
    if (pMaybeMultiport == pActualPort->getParentPort())
    {
        if (wasSucess)
        {
            //If successful we should remove the empty port
            pMaybeMultiport->removeSubPort(pActualPort);
            pActualPort = 0;
        }
        else
        {
            //! @todo What do we need to do to handle failure, nothing maybe
        }
    }
}

void ConnectionAssistant::checkPort(const Port *pPort, ConnectionAssistant::ConnOKCounters &rCounters)
{
    if (pPort->isInterfacePort())
    {
        rCounters.nInterfacePorts += 1;
    }

    if ( pPort->getPortType() == ReadPortType )
    {
        rCounters.nReadPorts += 1;
    }
    if ( pPort->getPortType() == WritePortType )
    {
        rCounters.nWritePorts += 1;
    }
    if ( pPort->getPortType() == PowerPortType )
    {
        rCounters.nPowerPorts += 1;
        if (pPort->getComponent()->isComponentC())
        {
            rCounters.nNonInterfaceCPowerPorts += 1;
        }
        else if (pPort->getComponent()->isComponentQ())
        {
            rCounters.nNonInterfaceQPowerPorts += 1;
        }
    }
    if ( pPort->getPortType() == SystemPortType )
    {
        rCounters.nSystemPorts += 1;
    }
//        if( pPort->getPortType() > MULTIPORT)
//        {
//            rCounters.n_MultiPorts += 1;
//        }
    if ( pPort->getComponent()->isComponentC() )
    {
        rCounters.nCComponents += 1;
        if ( pPort->getComponent()->isComponentSystem() )
        {
            rCounters.nSYScomponentCs += 1;
        }
    }
    else if ( pPort->getComponent()->isComponentQ() )
    {
        rCounters.nQComponents += 1;
        if ( pPort->getComponent()->isComponentSystem() )
        {
            rCounters.nSYScomponentQs += 1;
        }
    }
}

void ConnectionAssistant::removeNode(Node *pNode)
{
    if (pNode->getOwnerSystem())
    {
        pNode->getOwnerSystem()->removeSubNode(pNode);
    }
    mpComponentSystem->getHopsanEssentials()->removeNode(pNode);
}

////! @brief Prepares port pointers for multiport disconnections,
//void ConnectionAssistant::ifMultiportPrepareForDisconnect(Port *&rpPort1, Port *&rpPort2, Port *&rpMultiSubPort1, Port *&rpMultiSubPort2)
//{
//    // First make sure that multiport pointers are zero if no multiports are being connected
//    rpMultiSubPort1=0;
//    rpMultiSubPort2=0;

//    // Port 1 is a multiport, but not port2
//    if (rpPort1->getPortType() >= MultiportType && rpPort2->getPortType() < MultiportType )
//    {
//        rpMultiSubPort1 = findMultiportSubportFromOtherPort(rpPort1, rpPort2);
//        if(rpMultiSubPort1 == 0)
//        {
//            mpComponentSystem->addFatalMessage("ifMultiportPrepareForDisconnect(): rpMultiSubPort1 == 0");
//        }
//    }
//    // Port 2 is a multiport, but not port1
//    else if (rpPort1->getPortType() < MultiportType && rpPort2->getPortType() >= MultiportType )
//    {
//        rpMultiSubPort2 = findMultiportSubportFromOtherPort(rpPort2, rpPort1);
//        if(rpMultiSubPort2 == 0)
//        {
//            mpComponentSystem->addFatalMessage("ifMultiportPrepareForDisconnect(): rpMultiSubPort2 == 0");
//        }
//    }
//    // both ports are multiports
//    else if (rpPort1->getPortType() >= MultiportType && rpPort2->getPortType() >= MultiportType )
//    {
//        mpComponentSystem->addFatalMessage("ifMultiportPrepareForDisconnect(): Multiport <-> Multiport disconnection has not been implemented yet.");
//        //! @todo need to search around to find correct subports
//    }
//}


//! @brief Disconnect two ports, string version
//! @param [in] compname1 The name of the first component
//! @param [in] portname1 The name of the port on the first component
//! @param [in] compname2 The name of the second component
//! @param [in] portname2 The name of the port on the second component
//! @returns True if success, False if failed
bool ComponentSystem::disconnect(const HString &compname1, const HString &portname1, const HString &compname2, const HString &portname2)
{
    Component *pComp1 = getSubComponentOrThisIfSysPort(compname1);
    Component *pComp2 = getSubComponentOrThisIfSysPort(compname2);

    if ( (pComp1!=0) && (pComp2!=0) )
    {
        Port *pPort1 = pComp1->getPort(portname1);
        Port *pPort2 = pComp2->getPort(portname2);

        if ( (pComp1!=0) && (pComp2!=0) )
        {
            return disconnect(pPort1, pPort2);
        }
    }

    addDebugMessage("Disconnect: Could not find either " +compname1+"->"+portname1+" or "+compname2+"->"+portname2);
    return false;
}

//! @brief Disconnects two ports and remove node if no one is using it any more.
//! @param pPort1 Pointer to first port
//! @param pPort2 Pointer to second port
bool ComponentSystem::disconnect(Port *pPort1, Port *pPort2)
{
    // First check if ports not null
    if (pPort1 && pPort2)
    {
        //cout << "disconnecting " << pPort1->getComponentName().c_str() << " " << pPort1->getName().c_str() << "  and  " << pPort2->getComponentName().c_str() << " " << pPort2->getName().c_str() << endl;

        ConnectionAssistant disconnAssistant(this);
        //! @todo some more advanced error handling
        if (pPort1->isConnectedTo(pPort2))
        {
            bool success = false;

            // If non of the ports are multiports
            if ( !(pPort1->isMultiPort() || pPort2->isMultiPort()) )
            {
                success = disconnAssistant.splitNodeConnection(pPort1, pPort2);
            }
            // If one of the ports is a multiport
            else if ( pPort1->isMultiPort() || pPort2->isMultiPort() )
            {
                //! @todo what happens if we disconnect a multiport from a port with multiple connections (can that even happen)
                if(pPort1->isMultiPort() && pPort2->isMultiPort())
                {
                    addFatalMessage("ComponentSystem::disconnect(): Trying to disconnect two multiports.");
                    return false;
                }

                // Handle multiports
                Port *pActualPort1, *pActualPort2;
                disconnAssistant.ifMultiportPrepareDissconnect(pPort1, pPort2, pActualPort1, pActualPort2);

                success = disconnAssistant.splitNodeConnection(pActualPort1, pActualPort2);

                // Handle multiport connection success or failure
                disconnAssistant.ifMultiportCleanupAfterDissconnect(pPort1, pActualPort1, success);
                disconnAssistant.ifMultiportCleanupAfterDissconnect(pPort2, pActualPort2, success);
            }

            disconnAssistant.clearSysPortNodeTypeIfEmpty(pPort1);
            disconnAssistant.clearSysPortNodeTypeIfEmpty(pPort2);
            //! @todo maybe incorporate the clear checks into delete node and unmerge

            //Update the CQS type
            this->determineCQSType();

            //Update parent cqs-type
            //! @todo we should only do this if we are actually connected directly to our parent, but I don't know what will take the most time, to check if we are connected to parent or to just always refresh parent
            if (!this->isTopLevelSystem())
            {
                this->mpSystemParent->determineCQSType();
            }

            HString msg;
            msg = "Disconnected: {"+pPort1->getComponent()->getName()+"::"+pPort1->getName()+"} and {"+pPort2->getComponent()->getName()+"::"+pPort2->getName()+"}";
            addDebugMessage(msg, "succesfuldisconnect");

            return success;
        }
        else
        {
            addErrorMessage("When attempting disconnect: Port: " + pPort1->getComponentName()+"::"+pPort1->getName() + "  is not connected to: " + pPort2->getComponentName()+"::"+pPort2->getName());
            return false;
        }
        addFatalMessage("When attempting disconnect: One of the ports is NULL");
    }
    return false;
}


//! @brief Sets the desired time step in a component system.
//! @brief param timestep New desired time step
void ComponentSystem::setDesiredTimestep(const double timestep)
{
    mDesiredTimestep = timestep;
    setTimestep(timestep);
}


void ComponentSystem::setInheritTimestep(const bool inherit)
{
    mInheritTimestep = inherit;
}


bool ComponentSystem::doesInheritTimestep() const
{
    return mInheritTimestep;
}


//void ComponentSystem::setTimestep(const double timestep)
//{
//    mTimestep = timestep;
//
//    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
//    {
//        if (!(mComponentSignalptrs[s]->isComponentSystem()))
//        {
//            mComponentSignalptrs[s]->setTimestep(timestep);
//        }
//    }
//
//    //C components
//    for (size_t c=0; c < mComponentCptrs.size(); ++c)
//    {
//        if (!(mComponentCptrs[c]->isComponentSystem()))
//        {
//            mComponentCptrs[c]->setTimestep(timestep);
//        }
//    }
//
//    //Q components
//    for (size_t q=0; q < mComponentQptrs.size(); ++q)
//    {
//        if (!(mComponentQptrs[q]->isComponentSystem()))
//        {
//            mComponentQptrs[q]->setTimestep(timestep);
//        }
//    }
//}


//! @brief Sets the time step in a component system.
//! Also propagates it to all contained components
//! @brief param timestep New time step
void ComponentSystem::setTimestep(const double timestep)
{
    mTimestep = timestep;

    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
    {
        if (!(mComponentSignalptrs[s]->isComponentSystem())/* && mComponentSignalptrs[s]->doesInheritTimestep()*/)
        {
            mComponentSignalptrs[s]->setTimestep(timestep);
        }
    }

    //C components
    for (size_t c=0; c < mComponentCptrs.size(); ++c)
    {
        if (!(mComponentCptrs[c]->isComponentSystem())/* && mComponentCptrs[c]->doesInheritTimestep()*/)
        {
            mComponentCptrs[c]->setTimestep(timestep);
        }
    }

    //Q components
    for (size_t q=0; q < mComponentQptrs.size(); ++q)
    {
        if (!(mComponentQptrs[q]->isComponentSystem())/* && mComponentQptrs[q]->doesInheritTimestep()*/)
        {
            mComponentQptrs[q]->setTimestep(timestep);
        }
    }
}


//! @brief Figure out which timestep to use for all sub components
//! @param componentPtrs Vector with pointers to all sub components
void ComponentSystem::adjustTimestep(vector<Component*> componentPtrs)
{
    for (size_t c=0; c < componentPtrs.size(); ++c)
    {
        // Check if component should inherit timestep from its parent system (this system)
        if(componentPtrs[c]->doesInheritTimestep())
        {
            componentPtrs[c]->setTimestep(mTimestep);
        }
        // Else use the desired timestep, and adjust it if necessary
        else
        {
            // Prevent negative or zero timesteps
            double subTs = componentPtrs[c]->mDesiredTimestep;
            if (subTs <= 0.0)
            {
                subTs = mTimestep;
            }
            componentPtrs[c]->setTimestep(subTs);
        }
    }
}

size_t limitNumLogSlotsToLogOrSimTimeInterval(const double simStartT, const double simStopT, const double simTs, const double logStartT, const size_t nRequestedLogSamples)
{
    double startT = max(simStartT, logStartT);
    // Saturate startT to stopT to avoid underflow in size_t if someone enters a logT that is higher the startT
    if (startT > simStopT)
    {
        startT = simStopT;
    }

    // Make sure we don't try to log more samples than we will simulate
    //! @todo may need some rounding tricks here
    if ( ((simStopT - startT) / simTs + 1) < nRequestedLogSamples )
    {
        return size_t( (simStopT - startT) / simTs + 1);

    }
    else
    {
        return nRequestedLogSamples;
    }
}

void ComponentSystem::setupLogSlotsAndTs(const double simStartT, const double simStopT, const double simTs)
{
    mnLogSlots = limitNumLogSlotsToLogOrSimTimeInterval(simStartT, simStopT, simTs, mRequestedLogStartTime, mRequestedNumLogSamples);
    if (mnLogSlots != mRequestedNumLogSamples)
    {
        addWarningMessage("Requested nLogSamples: "+to_hstring(mRequestedNumLogSamples)+" but this is more than the total number of simulation samples, limiting to: "+to_hstring(mnLogSlots), "toofewsamples");
    }

    if (mnLogSlots > 0)
    {
        enableLog();

        // We do not want to log before simStartT
        const double logStartT = max(simStartT,mRequestedLogStartTime);

        // Calc logDt and
        mLogTimeDt = (simStopT-logStartT)/double(mnLogSlots-1);

        // Figure out at which samples logging should happen
        double logT=logStartT;
        double simT=simStartT;

        mLogTheseTimeSteps.clear();
        mLogTheseTimeSteps.reserve(mnLogSlots);

        // Figure out the first simulation step to log (the one where simT >= logT)
        size_t n = (logT-simT)/double(simTs)+0.5;
        mLogTheseTimeSteps.push_back(n);
        // Fast forward simT
        simT += double(n)*simTs;

        // Now Calculate which additional simulation steps should be logged
        while (mLogTheseTimeSteps.size() < mnLogSlots)
        {
            logT += mLogTimeDt;
            n = size_t((logT-simT)/simTs+0.5);
            simT += double(n)*simTs;

            //cout << "SimT: " << simT << " logT: " << logT << " logT-simT: " << logT-simT << endl;
            mLogTheseTimeSteps.push_back(mLogTheseTimeSteps.back() + n);
        }

        //! @todo sanity check on log slots
        if (mnLogSlots != mLogTheseTimeSteps.size())
        {
            cout << "Error: mnLogSlots: " << mnLogSlots << " mLogTheseTimeSteps.size(): " << mLogTheseTimeSteps.size() << endl;
        }

        //        //cout << "n: " << n << endl;
        //        cout << "mNumSimulationSteps: " << size_t((stopT-logStartT)/Ts+0.5) << endl;
        //        cout << "mLastStepToLog: " << mLogTheseTimeSteps.back() << endl;
        //        cout << "mLogTimeDt: " << mLogTimeDt << " mTimeStepsToLog.size(): " << mLogTheseTimeSteps.size() << endl;

        //    for (int i=0; i<mTimeStepsToLog.size(); ++i)
        //    {
        //        cout << mTimeStepsToLog[i] << " ";
        //    }
        //    cout << endl;
    }
    else
    {
        disableLog();
    }
}

//! @brief Determines if all subnodes and subsystems subnodes should log data, Turn ALL ON or OFF
//! @todo name of this function is bad, this is a toggle function
void ComponentSystem::setAllNodesDoLogData(const bool logornot)
{
    // Do this systems nodes
    if (logornot)
    {

        for (size_t i=0; i<mSubNodePtrs.size(); ++i)
        {
            mSubNodePtrs[i]->setLoggingEnabled(true);
        }
    }
    else
    {
        for (size_t i=0; i<mSubNodePtrs.size(); ++i)
        {
            mSubNodePtrs[i]->setLoggingEnabled(false);
        }
    }

    // Do all subsystems
    SubComponentMapT::iterator scit;
    for (scit=mSubComponentMap.begin(); scit!=mSubComponentMap.end(); ++scit)
    {
        if (scit->second->isComponentSystem())
        {
            //!< @todo maybe should use static cast (quicker) or overloaded function in Component instead of casting
            dynamic_cast<ComponentSystem*>(scit->second)->setAllNodesDoLogData(logornot);
        }
    }
}


//! @brief Returns if start values should be loaded before simulation. If not, old simulation results is used as startvalues.
bool ComponentSystem::doesKeepStartValues()
{
    return mKeepStartValues;
}


//! @brief Set if or not start values should be loaded before simulation. If not, old simulation results is used as startvalues.
void ComponentSystem::setLoadStartValues(bool load)
{
    mKeepStartValues = load;
}


//! @brief Checks that everything is OK before simulation
bool ComponentSystem::checkModelBeforeSimulation()
{
    // Make sure that there are no components or systems with an undefined cqs_type present
    if (mComponentUndefinedptrs.size() > 0)
    {

        for (size_t i=0; i<mComponentUndefinedptrs.size(); ++i)
        {
            addErrorMessage("The Component:  "+mComponentUndefinedptrs[i]->getName()+"  has an invalid CQS-Type:  "+mComponentUndefinedptrs[i]->getTypeCQSString());
        }
        return false;
    }

    // Check this systems own SystemPorts, are they connected (they must be)
    vector<Port*> ports = getPortPtrVector();
    for (size_t i=0; i<ports.size(); ++i)
    {
        if ( ports[i]->isConnectionRequired() && !ports[i]->isConnected() )
        {
            addErrorMessage("Port:  "+ports[i]->getName()+"  on SystemComponent:  "+getName()+"  is not connected!");
            return false;
        }
        else if( ports[i]->isConnected() )
        {
            if(ports[i]->getNodePtr()->getNumberOfPortsByType(PowerPortType) == 1)
            {
                addErrorMessage("Port:  "+ports[i]->getName()+"  on Component:  "+ports[i]->getComponentName()+"  is connected to a node with only one attached power port!");
                return false;
            }
        }
    }

    // Generate a list of all system parameters (constants), to check if any are unused
    std::vector<HString> unusedSysParNames;
    const std::vector<ParameterEvaluator*> *pSysParameters = getParametersVectorPtr();
    unusedSysParNames.reserve(pSysParameters->size());
    for (size_t sp=0; sp<pSysParameters->size(); ++sp)
    {
        // We want to ignore those containing # as they are most likely start values in interface ports
        const HString& rName = pSysParameters->at(sp)->getName();
        if (!rName.containes('#'))
        {
            unusedSysParNames.push_back(rName);
        }
    }

    // Check all subcomponents to make sure that all requirements for simulation are met
    // scmit = The subcomponent map iterator
    SubComponentMapT::iterator scmit = mSubComponentMap.begin();
    for ( ; scmit!=mSubComponentMap.end(); ++scmit)
    {
        Component* pComp = scmit->second; //Component pointer

        // Check that ALL ports that MUST be connected are connected
        vector<Port*> ports = pComp->getPortPtrVector();
        for (size_t i=0; i<ports.size(); ++i)
        {
            if ( ports[i]->isConnectionRequired() && !ports[i]->isConnected() )
            {
                addErrorMessage("Port:  "+ports[i]->getName()+"  on Component:  " + pComp->getName() + "  is not connected!");
                return false;
            }
            else if( ports[i]->isConnected() )
            {
                size_t numPP = ports[i]->getNodePtr()->getNumberOfPortsByType(PowerPortType);
                if (ports[i]->isInterfacePort() && ports[i]->getPortType()==PowerPortType)
                {
                    if( numPP > 0 && numPP < 3)
                    {
                        addErrorMessage("InterfacePort:  "+ports[i]->getName()+"  on Component:  "+ports[i]->getComponentName()+"  is connected to a node with only two power ports!");
                        return false;
                    }
                }
                else if(numPP == 1)
                {
                    addErrorMessage("Port:  "+ports[i]->getName()+"  on Component:  "+ports[i]->getComponentName()+"  is connected to a node with only one power port!");
                    return false;
                }
            }

            // Check parameters in subcomponents
            HString errParName;
            if(!(pComp->checkParameters(errParName)))
            {
                HString val;
                pComp->getParameterValue(errParName, val);
                addErrorMessage("The parameter:  "+errParName+"  in System:  "+getName()+"  and Component:  "+pComp->getName()+" with value:  "+val+"  could not be evaluated!");
                return false;
            }
        }

        // Check if component uses a system parameter and remove it from the unused list (if not already removed)
        const std::vector<ParameterEvaluator*> *pCompParameters = pComp->getParametersVectorPtr();
        for(size_t p=0; p<pCompParameters->size(); ++p)
        {
            // Break the loop if we do not have any system parameter names to check
            if (unusedSysParNames.empty())
            {
                break;
            }

            // If a system parameter is used in the component, then erase it from the list
            std::vector<HString>::iterator itp = std::find(unusedSysParNames.begin(), unusedSysParNames.end(), pCompParameters->at(p)->getValue());
            if(itp != unusedSysParNames.end())
            {
                unusedSysParNames.erase(itp);
            }
        }

        // Check parameters in system
        HString errParName;
        if(!(checkParameters(errParName)))
        {
            addErrorMessage("The system parameter:  "+errParName+"  in System:  "+getName()+"  can not be evaluated, it maybe depend on a deleted system parameter.");
            return false;
        }

        // Recurse testing into subsystems
        if (pComp->isComponentSystem())
        {
            if (!pComp->checkModelBeforeSimulation())
            {
                return false;
            }
        }

        //! @todo check that all C-component required ports are connected to Q-component ports

        //! @todo check more stuff
    }

    // Add warning message if at least one system parameter is unused
    if(mWarnIfUnusedSystemParameters && (unusedSysParNames.size() > 0))
    {
        std::stringstream ss;
        ss << "The following system parameters are not used by any sub component:";
        for(size_t p=0; p<unusedSysParNames.size(); ++p)
        {
            ss << "\n" << unusedSysParNames[p].c_str();
        }
        addWarningMessage(ss.str().c_str());
    }

    return true;
}

//! @todo if we reconnect we should actually run check before simulation, BEFORE simulating, this is not done right now
bool ComponentSystem::preInitialize()
{
    // Recursively run preinitialize
    std::vector<Component*>::iterator compIt;
    for(compIt = mComponentSignalptrs.begin(); compIt != mComponentSignalptrs.end(); ++compIt)
    {
        if (!(*compIt)->preInitialize())
        {
            return false;
        }
    }
    for(compIt = mComponentCptrs.begin(); compIt != mComponentCptrs.end(); ++compIt)
    {
        if (!(*compIt)->preInitialize())
        {
            return false;
        }
    }
    for(compIt = mComponentQptrs.begin(); compIt != mComponentQptrs.end(); ++compIt)
    {
        if (!(*compIt)->preInitialize())
        {
            return false;
        }
    }
    return true;
}

//! @brief Load start values by telling each component to load their start values
void ComponentSystem::loadStartValues()
{
    // First load startvalues for all sub components
    std::vector<Component*>::iterator compIt;
    for(compIt = mComponentSignalptrs.begin(); compIt != mComponentSignalptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValues();
    }
    for(compIt = mComponentCptrs.begin(); compIt != mComponentCptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValues();
    }
    for(compIt = mComponentQptrs.begin(); compIt != mComponentQptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValues();
    }

    // Now load all startvalues for the interface ports, they should override internally set startvalues
    PortPtrMapT::iterator pit;
    for (pit=mPortPtrMap.begin(); pit!=mPortPtrMap.end(); ++pit)
    {
        Port* pPort = pit->second;
        if (pPort->getPortType() == ReadPortType) //! @todo what about readmultiport
        {
            ReadPort *pReadPort = dynamic_cast<ReadPort*>(pPort);
            if (pReadPort)
            {
                // Only write readport start value if it is connected to other readports
                // This is the case of input variable ports on systems that are not externally connected
                if (!pReadPort->isConnectedToWriteOrPowerPort())
                {
                    pReadPort->forceLoadStartValue();
                }
            }
        }
        else
        {
            pPort->loadStartValues();
        }
    }
}


//! @brief Load start values from last simulation to start value container
void ComponentSystem::loadStartValuesFromSimulation()
{
    std::vector<Component*>::iterator compIt;
    for(compIt = mComponentSignalptrs.begin(); compIt != mComponentSignalptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValuesFromSimulation();
    }
    for(compIt = mComponentCptrs.begin(); compIt != mComponentCptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValuesFromSimulation();
    }
    for(compIt = mComponentQptrs.begin(); compIt != mComponentQptrs.end(); ++compIt)
    {
        (*compIt)->loadStartValuesFromSimulation();
    }
}


//! @brief Loads parameters from a file
//! @param[in] rFilePath The file to load from
void ComponentSystem::loadParameters(const HString &rFilePath)
{
    loadHopsanParameterFile(rFilePath, getHopsanEssentials(), this);
}

//! @brief Loads parameters from a map
//! @param[in] rParameterMap The map to load from
void ComponentSystem::loadParameters(const SetParametersMapT &rParameterMap)
{
    std::map<HString, std::pair<std::vector<HString>, std::vector<HString> > >::const_iterator it;
    for(it=rParameterMap.begin(); it!=rParameterMap.end(); ++it)
    {
        // First try to get the component
        Component *pComponent = this->getSubComponent(it->first);
        if(pComponent)
        {
            // Now set each parameter name,value pair
            std::vector<HString> parNames = it->second.first;
            std::vector<HString> parValues = it->second.second;
            for(size_t i=0; i<parNames.size(); ++i)
            {
                pComponent->setParameterValue(parNames[i], parValues[i]);
            }
        }
    }
}


//! @brief Initializes a system and all its contained components before a simulation.
//! Also allocates log data memory.
//! @param[in] startT Start time of simulation
//! @param[in] stopT Stop time of simulation
bool ComponentSystem::initialize(const double startT, const double stopT)
{
    addLogMess("ComponentSystem::initialize()");

    if (this->isTopLevelSystem())
    {
        preInitialize();
    }


    //cout << "Initializing SubSystem: " << this->mName << endl;
    mStopSimulation = false; //This variable cannot be written on below, then problem might occur with thread safety, it's a bit ugly to write on it on this row.

    // Set initial time
    mTime = startT;
    mTotalTakenSimulationSteps=0;

    // Make sure timestep is not to low
    if (mTimestep < 10*(std::numeric_limits<double>::min)())
    {
        addErrorMessage("The timestep is too low.");
        return false;
    }

    //cout << "stopT = " << stopT << ", startT = " << startT << ", mTimestep = " << mTimestep << endl;
    //this->setLogSettingsNSamples(mRequestedNumLogSamples, startT, stopT, mTimestep);

    // This will calculate the mnLogSlots and other log related variables
    this->setupLogSlotsAndTs(startT, stopT, mTimestep);
    //! @todo make it possible to use other logtimestep methods then nLogSamples

    // preAllocate local logspace based on necessary number of logslots
    this->preAllocateLogSpace();

    // If we failed allocation then abort
    if (mStopSimulation)
    {
        return false;
    }

    adjustTimestep(mComponentSignalptrs);
    adjustTimestep(mComponentCptrs);
    adjustTimestep(mComponentQptrs);

    if(!this->sortComponentVector(mComponentSignalptrs))
    {
        return false;
    }
    this->sortComponentVector(mComponentCptrs);
    this->sortComponentVector(mComponentQptrs);

    // Only set startvalues from top-level system, else they will be set again in the subsystem initialize calls
    if (this->isTopLevelSystem())
    {
        if(!mKeepStartValues)
        {
            loadStartValues();
        }
    }

    //Init
    updateParameters();
    //Signal components
    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
    {
        if (mStopSimulation)
        {
            return false;
        }

        mComponentSignalptrs[s]->initializeAutoSignalNodeDataPtrs();
        mComponentSignalptrs[s]->updateParameters();

        if (mComponentSignalptrs[s]->isComponentSystem())
        {
            //! @todo should we use our own nSamples or the subsystems own ?
            static_cast<ComponentSystem*>(mComponentSignalptrs[s])->setNumLogSamples(mRequestedNumLogSamples);
            static_cast<ComponentSystem*>(mComponentSignalptrs[s])->setLogStartTime(mRequestedLogStartTime);
        }

        if(!mComponentSignalptrs[s]->initialize(startT, stopT))
        {
            stopSimulation();
        }
    }

    //C components
    for (size_t c=0; c < mComponentCptrs.size(); ++c)
    {
        if (mStopSimulation)
        {
            return false;
        }

        mComponentCptrs[c]->initializeAutoSignalNodeDataPtrs();
        mComponentCptrs[c]->updateParameters();

        if (mComponentCptrs[c]->isComponentSystem())
        {
            //! @todo should we use our own nSamples ore the subsystems own ?
            static_cast<ComponentSystem*>(mComponentCptrs[c])->setNumLogSamples(mRequestedNumLogSamples);
            static_cast<ComponentSystem*>(mComponentCptrs[c])->setLogStartTime(mRequestedLogStartTime);
        }

        if(!mComponentCptrs[c]->initialize(startT, stopT))
        {
            stopSimulation();
        }
    }

    //Q components
    for (size_t q=0; q < mComponentQptrs.size(); ++q)
    {
        if (mStopSimulation)
        {
            return false;
        }

        mComponentQptrs[q]->initializeAutoSignalNodeDataPtrs();
        mComponentQptrs[q]->updateParameters();

        if (mComponentQptrs[q]->isComponentSystem())
        {
            //! @todo should we use our own nSamples ore the subsystems own ?
            static_cast<ComponentSystem*>(mComponentQptrs[q])->setNumLogSamples(mRequestedNumLogSamples);
            static_cast<ComponentSystem*>(mComponentQptrs[q])->setLogStartTime(mRequestedLogStartTime);
        }

        if(!mComponentQptrs[q]->initialize(startT, stopT))
        {
            stopSimulation();
        }
    }

    if (mStopSimulation)
    {
        return false;
    }

    logTimeAndNodes(mTotalTakenSimulationSteps); // Log the startvalues

    // We seems to have initialized successfully
    return true;
}


#ifdef USETBB






//! @brief Simulate function for multi-threaded simulations.
//! @param startT Start time of simulation
//! @param stopT Stop time of simulation
//! @param nDesiredThreads Desired amount of simulation threads
//void ComponentSystem::simulateMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, const bool noChanges)
//{
//    mTime = startT;
//    double stopTsafe = stopT - mTimestep/2.0;                   //Calculate the "actual" stop time, minus half a timestep is here to ensure that no numerical issues occur

//    logTimeAndNodes(mTime);                                         //Log the first time step

//    size_t nThreads = determineActualNumberOfThreads(nDesiredThreads);      //Calculate how many threads to actually use

//    if(!noChanges)
//    {
//        mSplitCVector.clear();
//        mSplitQVector.clear();
//        mSplitSignalVector.clear();
//        mSplitNodeVector.clear();

//        simulateAndMeasureTime(5);                                  //Measure time
//        sortComponentVectorsByMeasuredTime();                       //Sort component vectors

//        for(size_t q=0; q<mComponentQptrs.size(); ++q)
//        {
//            std::stringstream ss;
//            ss << "Time for " << mComponentQptrs.at(q)->getName() << ": " << mComponentQptrs.at(q)->getMeasuredTime();
//            addDebugMessage(ss.str());
//        }
//        for(size_t c=0; c<mComponentCptrs.size(); ++c)
//        {
//            std::stringstream ss;
//            ss << "Time for " << mComponentCptrs.at(c)->getName() << ": " << mComponentCptrs.at(c)->getMeasuredTime();
//            addDebugMessage(ss.str());
//        }
//        for(size_t s=0; s<mComponentSignalptrs.size(); ++s)
//        {
//            std::stringstream ss;
//            ss << "Time for " << mComponentSignalptrs.at(s)->getName() << ": " << mComponentSignalptrs.at(s)->getMeasuredTime();
//            addDebugMessage(ss.str());
//        }

//        distributeCcomponents(mSplitCVector, nThreads);              //Distribute components and nodes
//        distributeQcomponents(mSplitQVector, nThreads);
//        distributeSignalcomponents(mSplitSignalVector, nThreads);
//        distributeNodePointers(mSplitNodeVector, nThreads);

//        //! @todo Reinit

//    }

//    tbb::task_group *simTasks;                                  //Initialize TBB routines for parallel  simulation
//    simTasks = new tbb::task_group;

//    //Execute simulation
//#define BARRIER_SYNC
//#ifdef BARRIER_SYNC
//    mvTimePtrs.push_back(&mTime);
//    BarrierLock *pBarrierLock_S = new BarrierLock(nThreads);    //Create synchronization barriers
//    BarrierLock *pBarrierLock_C = new BarrierLock(nThreads);
//    BarrierLock *pBarrierLock_Q = new BarrierLock(nThreads);
//    BarrierLock *pBarrierLock_N = new BarrierLock(nThreads);

//    simTasks->run(taskSimMaster(this, mSplitSignalVector[0], mSplitCVector[0], mSplitQVector[0],             //Create master thread
//                                mSplitNodeVector[0], mvTimePtrs, mTime, mTimestep, stopTsafe, nThreads, 0,
//                                pBarrierLock_S, pBarrierLock_C, pBarrierLock_Q, pBarrierLock_N));

//    for(size_t t=1; t < nThreads; ++t)
//    {
//        simTasks->run(taskSimSlave(mSplitSignalVector[t], mSplitCVector[t], mSplitQVector[t],          //Create slave threads
//                                   mSplitNodeVector[t], mTime, mTimestep, stopTsafe, nThreads, t,
//                                   pBarrierLock_S, pBarrierLock_C, pBarrierLock_Q, pBarrierLock_N));
//    }

//    simTasks->wait();                                           //Wait for all tasks to finish

//    delete(simTasks);                                           //Clean up
//    delete(pBarrierLock_S);
//    delete(pBarrierLock_C);
//    delete(pBarrierLock_Q);
//    delete(pBarrierLock_N);
//#else
//    vector<Component*> tempVector;
//    for(int i=mComponentSignalptrs.size()-1; i>-1; --i)
//    {
//        tempVector.push_back(mComponentSignalptrs[i]);
//    }
//    mComponentSignalptrs = tempVector;
//    tempVector.clear();
//    for(int i=mComponentCptrs.size()-1; i>-1; --i)
//    {
//        tempVector.push_back(mComponentCptrs[i]);
//    }
//    mComponentCptrs = tempVector;
//    tempVector.clear();
//    for(int i=mComponentQptrs.size()-1; i>-1; --i)
//    {
//        tempVector.push_back(mComponentQptrs[i]);
//    }
//    mComponentQptrs = tempVector;

//    cout << "Creating task pools!" << endl;

//    TaskPool<Component> *sPool = new TaskPool<Component>(mComponentSignalptrs, nThreads);
//    TaskPool<Component> *qPool = new TaskPool<Component>(mComponentQptrs, nThreads);
//    TaskPool<Component> *cPool = new TaskPool<Component>(mComponentCptrs, nThreads);
//    TaskPool<Node> *nPool = new TaskPool<Node>(mSubNodePtrs, nThreads);

//    cout << "Starting task threads!";
//   // assert("Starting task threads"==0);

//    for(size_t t=0; t < nThreads; ++t)
//    {
//        simTasks->run(taskSimPool(sPool, qPool, cPool, nPool, mTime, mTimestep, stopTsafe, t, this));
//    }
//    simTasks->wait();                                           //Wait for all tasks to finish

//    delete(simTasks);                                           //Clean up
//#endif
//}

void ComponentSystem::simulateMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, const bool noChanges, const ParallelAlgorithmT algorithm)
{
    size_t nThreads = determineActualNumberOfThreads(nDesiredThreads);      //Calculate how many threads to actually use

    std::stringstream ss;
    ss << nThreads;
    HString threadStr = ss.str().c_str();

    if(!noChanges)
    {
        if(algorithm != TaskStealingAlgorithm)
        {
            mSplitCVector.clear();
            mSplitQVector.clear();
            mSplitSignalVector.clear();
            mSplitNodeVector.clear();

            simulateAndMeasureTime(100);                                //Measure time
            sortComponentVectorsByMeasuredTime();                       //Sort component vectors

            for(size_t q=0; q<mComponentQptrs.size(); ++q)
            {
                addDebugMessage("Time for "+mComponentQptrs.at(q)->getName()+": "+ to_hstring(mComponentQptrs.at(q)->getMeasuredTime()));
            }
            for(size_t c=0; c<mComponentCptrs.size(); ++c)
            {
                addDebugMessage("Time for "+mComponentCptrs.at(c)->getName()+": "+to_hstring(mComponentCptrs.at(c)->getMeasuredTime()));
            }
            for(size_t s=0; s<mComponentSignalptrs.size(); ++s)
            {
                addDebugMessage("Time for "+mComponentSignalptrs.at(s)->getName()+": "+to_hstring(mComponentSignalptrs.at(s)->getMeasuredTime()));
            }

            distributeCcomponents(mSplitCVector, nThreads);              //Distribute components and nodes
            distributeQcomponents(mSplitQVector, nThreads);
            distributeSignalcomponents(mSplitSignalVector, nThreads);
            distributeNodePointers(mSplitNodeVector, nThreads);

            // Re-initialize the system to reset values and timers
            //! @note This only work for top level systems where the simulateMultiThreaded will not be called more than once
            this->initialize(startT, stopT);
        }
        else
        {
            mSplitCVector.clear();
            mSplitQVector.clear();
            mSplitSignalVector.clear();

            mSplitCVector.resize(nThreads);
            mSplitQVector.resize(nThreads);
            mSplitSignalVector.resize(nThreads);

            for(size_t c=0; c<mComponentCptrs.size();)
            {
                for(size_t t=0; t<nThreads; ++t)
                {
                    if(c>mComponentCptrs.size()-1)
                        break;
                    mSplitCVector[t].push_back(mComponentCptrs[c]);
                    ++c;
                }
            }

            for(size_t q=0; q<mComponentQptrs.size();)
            {
                for(size_t t=0; t<nThreads; ++t)
                {
                    if(q>mComponentQptrs.size()-1)
                        break;
                    mSplitQVector[t].push_back(mComponentQptrs[q]);
                    ++q;
                }
            }

            distributeSignalcomponents(mSplitSignalVector, nThreads);
           // distributeNodePointers(mSplitNodeVector. nThreads);
        }
    }


    size_t nSteps = calcNumSimSteps(startT, stopT);
    tbb::task_group *simTasks;                                  //Initialize TBB routines for parallel  simulation
    simTasks = new tbb::task_group;

    //Execute simulation
    if(algorithm == OfflineSchedulingAlgorithm)
    {
        addInfoMessage("Using offline scheduling algorithm with "+threadStr+" threads.");

        mvTimePtrs.push_back(&mTime);
        BarrierLock *pBarrierLock_S = new BarrierLock(nThreads);    //Create synchronization barriers
        BarrierLock *pBarrierLock_C = new BarrierLock(nThreads);
        BarrierLock *pBarrierLock_Q = new BarrierLock(nThreads);
        BarrierLock *pBarrierLock_N = new BarrierLock(nThreads);

        simTasks->run(taskSimMaster(this, mSplitSignalVector[0], mSplitCVector[0], mSplitQVector[0],             //Create master thread
                                    mSplitNodeVector[0], mvTimePtrs, mTime, mTimestep, nSteps, nThreads, 0,
                                    pBarrierLock_S, pBarrierLock_C, pBarrierLock_Q, pBarrierLock_N));

        for(size_t t=1; t < nThreads; ++t)
        {
            simTasks->run(taskSimSlave(this, mSplitSignalVector[t], mSplitCVector[t], mSplitQVector[t],          //Create slave threads
                                       mSplitNodeVector[t], mTime, mTimestep, nSteps, nThreads, t,
                                       pBarrierLock_S, pBarrierLock_C, pBarrierLock_Q, pBarrierLock_N));
        }

        simTasks->wait();                                           //Wait for all tasks to finish

        delete(simTasks);                                           //Clean up
        delete(pBarrierLock_S);
        delete(pBarrierLock_C);
        delete(pBarrierLock_Q);
        delete(pBarrierLock_N);
    }
    else if(algorithm == OfflineReschedulingAlgorithm)
    {
        addInfoMessage("Using offline rescheduling algorithm with "+threadStr+" threads.");

        mvTimePtrs.push_back(&mTime);
        BarrierLock *pBarrierLock_S = new BarrierLock(nThreads);    //Create synchronization barriers
        BarrierLock *pBarrierLock_C = new BarrierLock(nThreads);
        BarrierLock *pBarrierLock_Q = new BarrierLock(nThreads);
        BarrierLock *pBarrierLock_N = new BarrierLock(nThreads);

        simTasks->run(taskSimMaster(this, mSplitSignalVector[0], mSplitCVector[0], mSplitQVector[0],             //Create master thread
                                    mSplitNodeVector[0], mvTimePtrs, mTime, mTimestep, 1100, nThreads, 0,
                                    pBarrierLock_S, pBarrierLock_C, pBarrierLock_Q, pBarrierLock_N));

        for(size_t t=1; t < nThreads; ++t)
        {
            simTasks->run(taskSimSlave(this, mSplitSignalVector[t], mSplitCVector[t], mSplitQVector[t],          //Create slave threads
                                       mSplitNodeVector[t], mTime, mTimestep, 1100, nThreads, t,
                                       pBarrierLock_S, pBarrierLock_C, pBarrierLock_Q, pBarrierLock_N));
        }

        simTasks->wait();                                           //Wait for all tasks to finish

        reschedule(nThreads);

        simTasks->run(taskSimMaster(this, mSplitSignalVector[0], mSplitCVector[0], mSplitQVector[0],             //Create master thread
                                    mSplitNodeVector[0], mvTimePtrs, mTime, mTimestep, nSteps-1100, nThreads, 0,
                                    pBarrierLock_S, pBarrierLock_C, pBarrierLock_Q, pBarrierLock_N));

        for(size_t t=1; t < nThreads; ++t)
        {
            simTasks->run(taskSimSlave(this, mSplitSignalVector[t], mSplitCVector[t], mSplitQVector[t],          //Create slave threads
                                       mSplitNodeVector[t], mTime, mTimestep, nSteps-1100, nThreads, t,
                                       pBarrierLock_S, pBarrierLock_C, pBarrierLock_Q, pBarrierLock_N));
        }

        simTasks->wait();


        delete(simTasks);                                           //Clean up
        delete(pBarrierLock_S);
        delete(pBarrierLock_C);
        delete(pBarrierLock_Q);
        delete(pBarrierLock_N);
    }
    else if(algorithm == TaskPoolAlgorithm)
    {

        addInfoMessage("Using task pool algorithm with "+threadStr+" threads.");

        TaskPool *pTaskPoolS = new TaskPool(mComponentSignalptrs);
        TaskPool *pTaskPoolC = new TaskPool(mComponentCptrs);
        TaskPool *pTaskPoolQ = new TaskPool(mComponentQptrs);

        tbb::task_group *masterTasks;
        tbb::task_group *slaveTasks;
        masterTasks = new tbb::task_group;
        slaveTasks = new tbb::task_group;

        tbb::atomic<double> *pTime = new tbb::atomic<double>;
        *pTime = mTime;
        tbb::atomic<bool> *pStop = new tbb::atomic<bool>;
        *pStop = false;

        //masterTasks->run(taskSimPoolMaster(pTaskPoolS, pTaskPoolC, pTaskPoolQ, mTimestep, nSteps, this, &mTime, pTime, pStop));

        for(size_t t=1; t < nThreads; ++t)
        {
            slaveTasks->run(taskSimPoolSlave(pTaskPoolC, pTaskPoolQ, pTime, pStop));
        }

        Component *pComp;
        for(size_t i=0; i<nSteps; ++i)
        {
            *pTime = *pTime+mTimestep;

            //S-pool
            pTaskPoolS->open();
            pComp = pTaskPoolS->getComponent();
            while(pComp)
            {
                pComp->simulate(*pTime);
                pTaskPoolS->reportDone();
                pComp = pTaskPoolS->getComponent();
            }
            while(!pTaskPoolS->isReady()) {}
            pTaskPoolS->close();

            //C-pool
            pTaskPoolC->open();
            pComp = pTaskPoolC->getComponent();
            while(pComp)
            {
                pComp->simulate(*pTime);
                pTaskPoolC->reportDone();
                pComp = pTaskPoolC->getComponent();
            }
            while(!pTaskPoolC->isReady()) {}
            pTaskPoolC->close();

            //Q-pool
            pTaskPoolQ->open();
            pComp = pTaskPoolQ->getComponent();
            while(pComp)
            {
                pComp->simulate(*pTime);
                pTaskPoolQ->reportDone();
                pComp = pTaskPoolQ->getComponent();
            }
            while(!pTaskPoolQ->isReady()) {}
            pTaskPoolQ->close();

            mTime =  *pTime;
            logTimeAndNodes(i+1);            //Log all nodes
        }
        *pStop=true;


        //masterTasks->wait();                                           //Wait for all tasks to finish
        slaveTasks->wait();

        delete(masterTasks);                                       //Clean up
        delete(slaveTasks);
        delete(pTaskPoolS);
        delete(pTaskPoolC);
        delete(pTaskPoolQ);
        delete(pStop);
    }
    else if(algorithm == TaskStealingAlgorithm)
    {
        addInfoMessage("Using task stealing algorithm with "+threadStr+" threads.");

        mvTimePtrs.push_back(&mTime);
        BarrierLock *pBarrierLock_S = new BarrierLock(nThreads);    //Create synchronization barriers
        BarrierLock *pBarrierLock_C = new BarrierLock(nThreads);
        BarrierLock *pBarrierLock_Q = new BarrierLock(nThreads);
        BarrierLock *pBarrierLock_N = new BarrierLock(nThreads);

        size_t maxSize = mComponentCptrs.size()+mComponentQptrs.size()+mComponentSignalptrs.size();

        std::vector<ThreadSafeVector *> *pVectorsC = new std::vector<ThreadSafeVector *>();
        for(size_t i=0; i<mSplitCVector.size(); ++i)
        {
            pVectorsC->push_back(new ThreadSafeVector(mSplitCVector[i], maxSize));
        }

        std::vector<ThreadSafeVector *> *pVectorsQ = new std::vector<ThreadSafeVector *>();
        for(size_t i=0; i<mSplitQVector.size(); ++i)
        {
            pVectorsQ->push_back(new ThreadSafeVector(mSplitQVector[i], maxSize));
        }

        simTasks->run(taskSimStealingMaster(this, mComponentSignalptrs, pVectorsC, pVectorsQ,                          //Create master thread
                                            mvTimePtrs, mTime, mTimestep, nSteps, nThreads, 0,
                                            pBarrierLock_S, pBarrierLock_C, pBarrierLock_Q, pBarrierLock_N, maxSize));

        for(size_t t=1; t < nThreads; ++t)
        {
            simTasks->run(taskSimStealingSlave(this, pVectorsC, pVectorsQ, mTime, mTimestep, nSteps, nThreads, t,       //Create slave threads
                                               pBarrierLock_S, pBarrierLock_C, pBarrierLock_Q, pBarrierLock_N, maxSize));
        }

        simTasks->wait();                                           //Wait for all tasks to finish

        delete(simTasks);                                           //Clean up
        delete(pBarrierLock_S);
        delete(pBarrierLock_C);
        delete(pBarrierLock_Q);
        delete(pBarrierLock_N);
        delete(pVectorsC);
        delete(pVectorsQ);
    }
    else if(algorithm == ParallelForAlgorithm)
    {
        addInfoMessage("Using parallel for-loop algorithm 1 with unlimited number of threads.");

        // Round to nearest, we may not get exactly the stop time that we want
        size_t numSimulationSteps = calcNumSimSteps(mTime, stopT); //Here mTime is the last time step since it is not updated yet

        //Simulate
        for (size_t i=0; i<numSimulationSteps; ++i)
        {
            if (mStopSimulation)
            {
                break;
            }

            mTime += mTimestep; //mTime is updated here before the simulation,
                                //mTime is the current time during the simulateOneTimestep

            //Signal components
            for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
            {
                mComponentSignalptrs[s]->simulate(mTime);
            }
            simTasks->wait();

            //C components
            for (size_t c=0; c < mComponentCptrs.size(); ++c)
            {
                simTasks->run(TaskSimOneComponentOneStep(mComponentCptrs[c], mTime));
            }
            simTasks->wait();

            //Q components
            for (size_t q=0; q < mComponentQptrs.size(); ++q)
            {
                simTasks->run(TaskSimOneComponentOneStep(mComponentQptrs[q], mTime));
            }
            simTasks->wait();

            ++mTotalTakenSimulationSteps;

            logTimeAndNodes(mTotalTakenSimulationSteps);
        }
    }
    else if(algorithm == ParallelForTbbAlgorithm)
    {
        addInfoMessage("Using parallel for loop algorithm 2 with unlimited number of threads.");

        // Round to nearest, we may not get exactly the stop time that we want
        size_t numSimulationSteps = calcNumSimSteps(mTime, stopT); //Here mTime is the last time step since it is not updated yet

        //Simulate
        for (size_t i=0; i<numSimulationSteps; ++i)
        {
            if (mStopSimulation)
            {
                break;
            }

            mTime += mTimestep; //mTime is updated here before the simulation,
                                //mTime is the current time during the simulateOneTimestep

            //Signal components
            //tbb::parallel_for(tbb::blocked_range<int>(0, mComponentSignalptrs.size()), BodySimulateComponentVector( mComponentSignalptrs, mTime));
            for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
            {
                mComponentSignalptrs[s]->simulate(mTime);
            }

            //C components
            tbb::parallel_for(tbb::blocked_range<int>(0, mComponentCptrs.size()), BodySimulateComponentVector( mComponentCptrs, mTime));

            //Q components
            tbb::parallel_for(tbb::blocked_range<int>(0, mComponentQptrs.size()), BodySimulateComponentVector( mComponentQptrs, mTime));

            ++mTotalTakenSimulationSteps;

            logTimeAndNodes(mTotalTakenSimulationSteps);
        }
    }
    else if(algorithm == RandomTaskPoolAlgorithm)
    {

        addInfoMessage("Using random task pool algorithm with "+threadStr+" threads.");

        RandomTaskPool *pTaskPoolS = new RandomTaskPool(mComponentSignalptrs);
        RandomTaskPool *pTaskPoolC = new RandomTaskPool(mComponentCptrs);
        RandomTaskPool *pTaskPoolQ = new RandomTaskPool(mComponentQptrs);

        tbb::task_group *masterTasks;
        tbb::task_group *slaveTasks;
        masterTasks = new tbb::task_group;
        slaveTasks = new tbb::task_group;

        tbb::atomic<double> *pTime = new tbb::atomic<double>;
        *pTime = mTime;
        tbb::atomic<bool> *pStop = new tbb::atomic<bool>;
        *pStop = false;

        //masterTasks->run(taskSimPoolMaster(pTaskPoolS, pTaskPoolC, pTaskPoolQ, mTimestep, nSteps, this, &mTime, pTime, pStop));

        for(size_t t=1; t < nThreads; ++t)
        {
            slaveTasks->run(taskSimRandomPoolSlave(pTaskPoolC, pTaskPoolQ, pTime, pStop));
        }

        Component *pComp;
        for(size_t i=0; i<nSteps; ++i)
        {
            *pTime = *pTime+mTimestep;

            //S-pool
            pTaskPoolS->open();
            pComp = pTaskPoolS->getComponent();
            while(pComp)
            {
                pComp->simulate(*pTime);
                pTaskPoolS->reportDone();
                pComp = pTaskPoolS->getComponent();
            }
            while(!pTaskPoolS->isReady()) {}
            pTaskPoolS->close();

            //C-pool
            pTaskPoolC->open();
            pComp = pTaskPoolC->getComponent();
            while(pComp)
            {
                pComp->simulate(*pTime);
                pTaskPoolC->reportDone();
                pComp = pTaskPoolC->getComponent();
            }
            while(!pTaskPoolC->isReady()) {}
            pTaskPoolC->close();

            //Q-pool
            pTaskPoolQ->open();
            pComp = pTaskPoolQ->getComponent();
            while(pComp)
            {
                pComp->simulate(*pTime);
                pTaskPoolQ->reportDone();
                pComp = pTaskPoolQ->getComponent();
            }
            while(!pTaskPoolQ->isReady()) {}
            pTaskPoolQ->close();

            mTime =  *pTime;
            logTimeAndNodes(i+1);            //Log all nodes
        }
        *pStop=true;


        //masterTasks->wait();                                           //Wait for all tasks to finish
        slaveTasks->wait();

        delete(masterTasks);                                       //Clean up
        delete(slaveTasks);
        delete(pTaskPoolS);
        delete(pTaskPoolC);
        delete(pTaskPoolQ);
        delete(pStop);
    }
}




//! @brief Helper function that simulates all components and measure their average time requirements.
//! @param steps How many steps to simulate
//! @todo Could we use the other tictoc to avoid tbb dependency, then we could use it as a bottleneck finder even if tbb not present
bool ComponentSystem::simulateAndMeasureTime(const size_t nSteps)
{
    // Reset all measured times first
    for(size_t s=0; s<mComponentSignalptrs.size(); ++s)
        mComponentSignalptrs[s]->setMeasuredTime(0);
    for(size_t c=0; c<mComponentCptrs.size(); ++c)
        mComponentCptrs[c]->setMeasuredTime(0);
    for(size_t q=0; q<mComponentQptrs.size(); ++q)
        mComponentQptrs[q]->setMeasuredTime(0);


    // Measure time for each component during specified amount of steps
    double time;
    for(size_t s=0; s<mComponentSignalptrs.size(); ++s)
    {
        time = mTime; // Init time
        tbb::tick_count comp_start = tbb::tick_count::now();
        time += mTimestep*nSteps;
        mComponentSignalptrs[s]->simulate(time);
        tbb::tick_count comp_end = tbb::tick_count::now();
        mComponentSignalptrs[s]->setMeasuredTime((comp_end-comp_start).seconds());
    }

    for(size_t c=0; c<mComponentCptrs.size(); ++c)
    {
        time=mTime; // Reset time
        tbb::tick_count comp_start = tbb::tick_count::now();
        time += mTimestep*nSteps;
        mComponentCptrs[c]->simulate(time);
        tbb::tick_count comp_end = tbb::tick_count::now();
        mComponentCptrs[c]->setMeasuredTime((comp_end-comp_start).seconds());
    }

    for(size_t q=0; q<mComponentQptrs.size(); ++q)
    {
        time=mTime; // Reset time
        tbb::tick_count comp_start = tbb::tick_count::now();
        time += mTimestep*nSteps;
        mComponentQptrs[q]->simulate(time);
        tbb::tick_count comp_end = tbb::tick_count::now();
        mComponentQptrs[q]->setMeasuredTime((comp_end-comp_start).seconds());
    }

    return true;
}


//! @brief Returns the total sum of the measured time of the components in the system
double ComponentSystem::getTotalMeasuredTime()
{
    double time = 0;
    for(size_t s=0; s<mComponentSignalptrs.size(); ++s)
    {
        time += mComponentSignalptrs[s]->getMeasuredTime();
    }
    for(size_t c=0; c<mComponentCptrs.size(); ++c)
    {
        time += mComponentCptrs[c]->getMeasuredTime();
    }
    for(size_t q=0; q<mComponentQptrs.size(); ++q)
    {
        time += mComponentQptrs[q]->getMeasuredTime();
    }

    return time;
}


//! @brief Sorts a vector of component system pointers by their required simulation time
//! @param [in out]systemVector Vector with system pointers to sort
void SimulationHandler::sortSystemsByTotalMeasuredTime(std::vector<ComponentSystem*> &rSystemVector)
{
    size_t i, j;
    ComponentSystem *tempSystem;
    for(i = 1; i < rSystemVector.size(); ++i)
    {
        bool didSwap = false;
        for (j=0; j < (rSystemVector.size()-1); ++j)
        {
            if (rSystemVector[j+1]->getTotalMeasuredTime() > rSystemVector[j]->getTotalMeasuredTime())
            {
                tempSystem = rSystemVector[j];             //Swap elements
                rSystemVector[j] = rSystemVector[j+1];
                rSystemVector[j+1] = tempSystem;
                didSwap = true;               //Indicates that a swap occurred
            }
        }

        if(!didSwap)
        {
            break;
        }
    }
}


//! @brief Helper function that sorts C- and Q- component vectors by simulation time for each component.
//! @todo This function uses bubblesort. Maybe change to something faster.
void ComponentSystem::sortComponentVectorsByMeasuredTime()
{
        //Sort the components from longest to shortest time requirement
    size_t i, j;
    bool flag = true;
    Component *tempC;
    for(i = 1; (i < mComponentCptrs.size()) && flag; ++i)
    {
        flag = false;
        for (j=0; j < (mComponentCptrs.size()-1); ++j)
        {
            if (mComponentCptrs[j+1]->getMeasuredTime() > mComponentCptrs[j]->getMeasuredTime())
            {
                tempC = mComponentCptrs[j];             //Swap elements
                mComponentCptrs[j] = mComponentCptrs[j+1];
                mComponentCptrs[j+1] = tempC;
                flag = true;               //Indicates that a swap occurred
            }
        }
    }
    flag = true;
    Component *tempQ;
    for(i = 1; (i < mComponentQptrs.size()) && flag; ++i)
    {
        flag = false;
        for (j=0; j < (mComponentQptrs.size()-1); ++j)
        {
            if (mComponentQptrs[j+1]->getMeasuredTime() > mComponentQptrs[j]->getMeasuredTime())
            {
                tempQ = mComponentQptrs[j];             //Swap elements
                mComponentQptrs[j] = mComponentQptrs[j+1];
                mComponentQptrs[j+1] = tempQ;
                flag = true;               //Indicates that a swap occurred
            }
        }
    }
}


////! @brief Helper function that decides how many thread to use.
////! User specifies desired amount, but it is limited by how many cores the processor has.
////! @param nDesiredThreads How many threads the user wants
//int ComponentSystem::getNumberOfThreads(size_t nDesiredThreads)
//{
//        //Obtain number of processor cores from environment variable, or use user specified value if not zero
//    size_t nThreads;
//    size_t nCores;
//#ifdef _WIN32
//    if(getenv("NUMBER_OF_PROCESSORS") != 0)
//    {
//        string temp = getenv("NUMBER_OF_PROCESSORS");
//        nCores = atoi(temp.c_str());
//    }
//    else
//    {
//        nCores = 1;               //If non-Windows system, make sure there is at least one thread
//    }
//#else
//    nCores = max((long)1, sysconf(_SC_NPROCESSORS_ONLN));
//#endif
//    if(nDesiredThreads != 0)
//    {
//        nThreads = nDesiredThreads;              //If user specifides a number of threads, attempt to use this number
//        if(nThreads > nCores)       //But limit number of threads to the number of system cores
//        {
//            nThreads = nCores;
//        }
//    }
//    else
//    {
//        nThreads = nCores;          //User specified nothing, so use one thread per core
//    }

//    return nThreads;
//}


//! @brief Helper function that distributes C components equally over one vector per thread
//! Greedy algorithm is used. This does not guarantee an optimal solution, but is gives a 4/3-approximation.
//! @param rSplitCVector Reference to vector with vectors of components (one vector per thread)
//! @param nThreads Number of simulation threads
void ComponentSystem::distributeCcomponents(vector< vector<Component*> > &rSplitCVector, size_t nThreads)
{
    vector<double> timeVector;
    timeVector.resize(nThreads);
    for(size_t i=0; i<nThreads; ++i)
    {
        timeVector[i] = 0;
    }
    rSplitCVector.resize(nThreads);

    //Cycle components from largest to smallest
    for(size_t c=0; c<mComponentCptrs.size(); ++c)
    {
        //Find index of vector with smallest total time
        size_t smallestIndex=0;
        double smallestTime=timeVector.at(smallestIndex);
        for(size_t t=1; t<nThreads; ++t)
        {
            if(timeVector.at(t) < smallestTime)
            {
                smallestTime = timeVector.at(t);
                smallestIndex = t;
            }
        }

        //Insert current component to vector with smallest time
        rSplitCVector[smallestIndex].push_back(mComponentCptrs[c]);
        timeVector[smallestIndex] += mComponentCptrs[c]->getMeasuredTime();
    }

    for(size_t i=0; i<nThreads; ++i)
    {
        addDebugMessage("Creating C-type thread vector, measured time = " + to_hstring(timeVector[i]*1000) + " ms", "cvector");
    }

        //Finally we sort each component vector, so that
        //signal components are simulated in correct order:
    for(size_t i=0; i<rSplitCVector.size(); ++i)
    {
        sortComponentVector(rSplitCVector[i]);

        for(size_t j=0; j<rSplitCVector[i].size(); ++j)
        {
            addDebugMessage("   "+rSplitCVector[i][j]->getName());
        }
    }
}


//! @brief Helper function that distributes Q components equally over one vector per thread
//! Greedy algorithm is used. This does not guarantee an optimal solution, but is gives a 4/3-approximation.
//! @param rSplitQVector Reference to vector with vectors of components (one vector per thread)
//! @param nThreads Number of simulation threads
void ComponentSystem::distributeQcomponents(vector< vector<Component*> > &rSplitQVector, size_t nThreads)
{
    vector<double> timeVector;
    timeVector.resize(nThreads);
    for(size_t i=0; i<nThreads; ++i)
    {
        timeVector[i] = 0;
    }
    rSplitQVector.resize(nThreads);

    //Cycle components from largest to smallest
    for(size_t q=0; q<mComponentQptrs.size(); ++q)
    {
        //Find index of vector with smallest total time
        size_t smallestIndex=0;
        double smallestTime=timeVector.at(smallestIndex);
        for(size_t t=1; t<nThreads; ++t)
        {
            if(timeVector.at(t) < smallestTime)
            {
                smallestTime = timeVector.at(t);
                smallestIndex = t;
            }
        }

        //Insert current component to vector with smallest time
        rSplitQVector[smallestIndex].push_back(mComponentQptrs[q]);
        timeVector[smallestIndex] += mComponentQptrs[q]->getMeasuredTime();
    }

    for(size_t i=0; i<nThreads; ++i)
    {
        addDebugMessage("Creating Q-type thread vector, measured time = " + to_hstring(timeVector[i]*1000) + " ms", "qvector");
    }

        //Finally we sort each component vector, so that
        //signal components are simulated in correct order:
    for(size_t i=0; i<rSplitQVector.size(); ++i)
    {
        sortComponentVector(rSplitQVector[i]);

        for(size_t j=0; j<rSplitQVector[i].size(); ++j)
        {
            addDebugMessage("   "+rSplitQVector[i][j]->getName());
        }
    }
}


//! @brief Helper function that distributes signal components over one vector per thread.
//! @param rSplitSignalVector Reference to vector with vectors of components (one vector per thread)
//! @param nThreads Number of simulation threads
void ComponentSystem::distributeSignalcomponents(vector< vector<Component*> > &rSplitSignalVector, size_t nThreads)
{

        //First we want to divide the components into groups,
        //depending on who they are connected to.

    std::map<Component *, size_t> groupMap;     //Maps each component to a group number
    size_t curMax = 0;                          //Highest used group number

    //Loop through all signal components
    for(size_t s=0; s<mComponentSignalptrs.size(); ++s)
    {
        bool foundOneConnection=false;

        //Loop through all ports in each signal component
        for(size_t p=0; p<mComponentSignalptrs[s]->getPortPtrVector().size(); ++p)
        {
            //Loop through all connected ports to each port in each signal component
            for(size_t c=0; c<mComponentSignalptrs[s]->getPortPtrVector()[p]->getConnectedPorts().size(); ++c)
            {
                foundOneConnection=true;

                //Compare group number between current signal component and each connected component
                Component *A = mComponentSignalptrs[s];
                Component *B = mComponentSignalptrs[s]->getPortPtrVector()[p]->getConnectedPorts()[c]->getComponent();
                if(!groupMap.count(A) && !groupMap.count(B))        //Neither component has a number, so give current component a new number
                {
                    groupMap.insert(std::pair<Component *, size_t>(A, curMax));
                    ++curMax;
                }
                else if(!groupMap.count(A) && groupMap.count(B))    //Connected port has a number, so give current component same number
                {
                    groupMap.insert(std::pair<Component *, size_t>(A, groupMap.find(B)->second));
                }
                else if(groupMap.count(A) && groupMap.count(B))     //Both component have numbers, so merge current components group with the other one
                {
                    //Merge A's value with B's
                    size_t Aval = groupMap.find(A)->second;
                    size_t BVal = groupMap.find(B)->second;
                    std::map<Component *, size_t>::iterator it;
                    for(it=groupMap.begin(); it!=groupMap.end(); ++it)
                    {
                        if((*it).second == Aval)
                        {
                            (*it).second = BVal;
                        }
                    }
                }
            }
        }

        //If not connections were found, this is a lonely component, so add it to its own group
        if(!foundOneConnection)
        {
            groupMap.insert(std::pair<Component *, size_t>(mComponentSignalptrs[s], curMax));
            ++curMax;
        }
    }


        //Now we assign each component to a simulation thread vector.
        //We keep grouped components together, and always fill thread
        //with least measured time first.

    rSplitSignalVector.resize(nThreads);
    size_t i=0;                                             //Group number
    size_t currentVector=0;                                 //The vector to which we are currently adding components
    size_t nAddedComponents=0;                              //Total amount of added components
    std::vector<double> vectorTime;                           //Contains total measured time in each vector
    for(size_t j=0; j<rSplitSignalVector.size(); ++j)
    {
        vectorTime.push_back(0.0);
    }

    while(nAddedComponents < groupMap.size())               //Loop while there are still components to add
    {
        std::map<Component *, size_t>::iterator it;
        for(it=groupMap.begin(); it!=groupMap.end(); ++it)
        {
            if((*it).second == i)                           //Add all components with group number i to current vector
            {
                rSplitSignalVector[currentVector].push_back((*it).first);
                vectorTime[currentVector] += (*it).first->getMeasuredTime();
                ++nAddedComponents;
            }
        }

        //Find vector with smallest amount of data, to use next loop
        for(size_t j=0; j<vectorTime.size(); ++j)
        {
            if(vectorTime[j] < vectorTime[currentVector])
                currentVector = j;
        }
        ++i;
    }

//    // DEBUG
//    for(size_t i=0; i<vectorTime.size(); ++i)
//    {
//        std::stringstream ss;
//        ss << 1000*vectorTime[i];
//        addDebugMessage("Creating S-type thread vector, measured time = " + ss.str() + " ms", "svector");
//    }
//    // END DEBUG


        //Finally we sort each component vector, so that
        //signal components are simulated in correct order:
    for(size_t i=0; i<rSplitSignalVector.size(); ++i)
    {
        sortComponentVector(rSplitSignalVector[i]);
    }
}


//! @brief Helper function that distributes node pointers equally over one vector per thread
//! @param rSplitNodeVector Reference to vector with vectors of node pointers (one vector per thread)
//! @param nThreads Number of simulation threads
void ComponentSystem::distributeNodePointers(vector< vector<Node*> > &rSplitNodeVector, size_t nThreads)
{
    for(size_t c=0; c<nThreads; ++c)
    {
        vector<Node*> tempVector;
        rSplitNodeVector.push_back(tempVector);
    }
    size_t thread = 0;
    for(size_t n=0; n<mSubNodePtrs.size(); ++n)
    {
        rSplitNodeVector.at(thread).push_back(mSubNodePtrs.at(n));
        ++thread;
        if(thread>nThreads-1)
            thread = 0;
    }
}

void ComponentSystem::reschedule(size_t nThreads)
{
    mSplitCVector.clear();
    mSplitQVector.clear();
    mSplitSignalVector.clear();
    mSplitNodeVector.clear();

    simulateAndMeasureTime(10);                                //Measure time
    sortComponentVectorsByMeasuredTime();                       //Sort component vectors

    distributeCcomponents(mSplitCVector, nThreads);              //Distribute components and nodes
    distributeQcomponents(mSplitQVector, nThreads);
    distributeSignalcomponents(mSplitSignalVector, nThreads);
    distributeNodePointers(mSplitNodeVector, nThreads);
}

#else

        //This overrides the multi-threaded simulation call with a single-threaded simulation if TBB is not installed.
//! @brief Simulate function that overrides multi-threaded simulation call with a single-threaded call
//! In case multi-threaded support is not available
void ComponentSystem::simulateMultiThreaded(const double /*startT*/, const double stopT, const size_t /*nThreads*/, const bool /*noChanges*/, ParallelAlgorithmT /*algorithm*/)
{
    this->addErrorMessage("Multi-threaded simulation not available, TBB library is not present. Simulating single-threaded.");
    this->simulate(stopT);
}


bool ComponentSystem::simulateAndMeasureTime(const size_t /*steps*/)
{
    this->addErrorMessage("Unable to measure simulation time without TBB library.");
    return false;
}

double ComponentSystem::getTotalMeasuredTime()
{
    this->addErrorMessage("Time measurement results not available without TBB library.");
    return 0;
}


void SimulationHandler::sortSystemsByTotalMeasuredTime(std::vector<ComponentSystem*> &rSystemVector)
{
    if(rSystemVector.size() > 0)
    {
        rSystemVector[0]->addErrorMessage("Sorting systems by measured time is not possible without the TBB library.");
    }
    return;
}


void ComponentSystem::distributeCcomponents(vector< vector<Component*> > &/*rSplitCVector*/, size_t /*nThreads*/)
{
    addWarningMessage("Called distributeCcomponents(), but TBB is not avaialble.");
}


void ComponentSystem::distributeQcomponents(vector< vector<Component*> > &/*rSplitQVector*/, size_t /*nThreads*/)
{
    addWarningMessage("Called distributeQcomponents(), but TBB is not avaialble.");
}


void ComponentSystem::distributeSignalcomponents(vector< vector<Component*> > &/*rSplitSignalVector*/, size_t /*nThreads*/)
{
    addWarningMessage("Called distributeSignalcomponents(), but TBB is not avaialble.");
}


void ComponentSystem::distributeNodePointers(vector< vector<Node*> > &/*rSplitNodeVector*/, size_t /*nThreads*/)
{
    addWarningMessage("Called distributeNodePointers(), but TBB is not avaialble.");
}

#endif


////! @brief Simulate function for single-threaded simulations.
////! @param startT Start time of simulation
////! @param stopT Stop time of simulation
//void ComponentSystem::simulate(const double startT, const double stopT)
//{
//    mTime = startT;

//    //Simulate
//    double stopTsafe = stopT - mTimestep/2.0; //minus half a timestep is here to ensure that no numerical issues occurs

//    while ((mTime < stopTsafe) && (!mStopSimulation))
//    {
//        //! @todo maybe use iterators instead
//        //Signal components
//        for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
//        {
//            mComponentSignalptrs[s]->simulate(mTime, mTime+mTimestep);
//        }

//        //C components
//        for (size_t c=0; c < mComponentCptrs.size(); ++c)
//        {
//            mComponentCptrs[c]->simulate(mTime, mTime+mTimestep);
//        }
//        //! @todo this will log q and p from last Ts but c Zc from this Ts, this is strange
//        logTimeAndNodes(mTime); //MOVED HERE BECAUSE C-COMP ARE SETTING START TIMES

//        //Q components
//        for (size_t q=0; q < mComponentQptrs.size(); ++q)
//        {
//            mComponentQptrs[q]->simulate(mTime, mTime+mTimestep);
//        }

//        mTime += mTimestep;
//    }
//}

//! @brief Simulate function for single-threaded simulations.
//! @param[in] stopT Simulate from current time until stop time
void ComponentSystem::simulate(const double stopT)
{
    // Round to nearest, we may not get exactly the stop time that we want
    size_t numSimulationSteps = calcNumSimSteps(mTime, stopT); //Here mTime is the last time step since it is not updated yet

    //Simulate
    for (size_t i=0; i<numSimulationSteps; ++i)
    {
        if (mStopSimulation)
        {
            break;
        }

        mTime += mTimestep; //mTime is updated here before the simulation,
                            //mTime is the current time during the simulateOneTimestep

        //! @todo maybe use iterators instead
        //Signal components
        for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
        {
            mComponentSignalptrs[s]->simulate(mTime);
        }

        //C components
        for (size_t c=0; c < mComponentCptrs.size(); ++c)
        {
            mComponentCptrs[c]->simulate(mTime);
        }

        //Q components
        for (size_t q=0; q < mComponentQptrs.size(); ++q)
        {
            mComponentQptrs[q]->simulate(mTime);
        }

        ++mTotalTakenSimulationSteps;

        logTimeAndNodes(mTotalTakenSimulationSteps);
    }
}


//! @brief Simulates several systems sequentially until given stop time
//! @param[in] stopT Stop time for all systems
//! @param[in] rSystemVector Vector of pointers to component systems to simulate
//! @returns true if successful else false if simulation was aborted for some reason
bool SimulationHandler::simulateMultipleSystems(const double stopT, vector<ComponentSystem*> &rSystemVector)
{
    bool aborted = false;
    for(size_t i=0; i<rSystemVector.size(); ++i)
    {
        rSystemVector[i]->simulate(stopT);
        aborted = aborted && rSystemVector[i]->wasSimulationAborted(); //!< @todo this will give abort=true if one the systems fail, maybe we should abort entirely when one do
    }
    return !aborted;
}


//! @brief Finalizes a system component and all its contained components after a simulation.
void ComponentSystem::finalize()
{
    //Finalize
    //Signal components
    for (size_t s=0; s < mComponentSignalptrs.size(); ++s)
    {
        mComponentSignalptrs[s]->finalize();
    }

    //C components
    for (size_t c=0; c < mComponentCptrs.size(); ++c)
    {
        mComponentCptrs[c]->finalize();
    }

    //Q components
    for (size_t q=0; q < mComponentQptrs.size(); ++q)
    {
        mComponentQptrs[q]->finalize();
    }

    //loadStartValuesFromSimulation();
}

////! @brief This function will set the number of log data slots for preallocation and logDt based on a skip factor to the sample time
////! @param [in] factor The timestep skip factor, minimum 1.0, but if < 0 then disableLog
//void ComponentSystem::setLogSettingsSkipFactor(double factor, double start, double stop,  double sampletime)
//{
//    if (factor > 0)
//    {
//        enableLog();
//        //! @todo maybe only use integer factors
//        //make sure factor is not less then 1.0
//        factor = max(1.0, factor);
//        mLogTimeDt = sampletime * factor;
//        //mLastLogTime = start-mLogTimeDt;
//        mnLogSlots = (size_t)((stop-start)/mLogTimeDt+0.5); //Round to nearest
//        //! @todo FIXA /Peter
//    }
//    else
//    {
//        disableLog();
//    }
//}


////! @brief This function will set the number of log data slots for preallocation and logDt
////! @param [in] log_dt The desired log timestep, if log_dt < 0 then disableLog
//void ComponentSystem::setLogSettingsSampleTime(double log_dt, double start, double stop,  double sampletime)
//{
//    if (log_dt > 0)
//    {
//        enableLog();
//        // Make sure that we dont have log_dt lower than sampletime ( we cant log more then we calc)
//        log_dt = max(sampletime,log_dt);
//        mLogTimeDt = log_dt;
//        //mLastLogTime = start-mLogTimeDt;
//        mnLogSlots = size_t((stop-start)/log_dt+0.5); //Round to nearest
//        //! @todo FIXA /Peter
//    }
//    else
//    {
//        disableLog();
//    }
//}

//! @brief Enable node data logging
void ComponentSystem::enableLog()
{
    mEnableLogData = true;
}


//! @brief Disable node data logging
void ComponentSystem::disableLog()
{
    mEnableLogData = false;

    // If log disabled, then free memory if something has been previously allocated
    mTimeStorage.clear();
    mLogTheseTimeSteps.clear();

    mLogTimeDt = -1.0;
    //mLastLogTime = 0.0; //Initial value should not matter, will be overwritten when selecting log amount
    mnLogSlots = 0;
    mLogCtr = 0;
}

vector<double> *ComponentSystem::getLogTimeVector()
{
    return &mTimeStorage;
}

//! @brief Simulates a vector of component systems in parallel, by assigning one or more system to each simulation thread
//! @param startT Start time for all systems
//! @param stopT Stop time for all systems
//! @param nDesiredThreads Desired number of threads (may change due to hardware limitations)
//! @param rSystemVector Vector of pointers to systems to simulate
bool SimulationHandler::simulateMultipleSystemsMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, vector<ComponentSystem*> &rSystemVector, bool noChanges)
{
    HOPSAN_UNUSED(startT)
    HOPSAN_UNUSED(nDesiredThreads)
    HOPSAN_UNUSED(noChanges)
#ifdef USETBB
    size_t nThreads = determineActualNumberOfThreads(nDesiredThreads);              //Calculate how many threads to actually use

//    for(size_t i=0; i<rSystemVector.size(); ++i)                         //Loop through the systems, set start time, log nodes and measure simulation time
//    {
//        double *pTime = rSystemVector.at(i)->getTimePtr();
//        *pTime = startT;
////        rSystemVector.at(i)->logTimeAndNodes(*pTime);                        //Log the first time step
//        rSystemVector.at(i)->logTimeAndNodes(0);                        //Log the first time step
//    }

    if(!noChanges)
    {
        mSplitSystemVector.clear();
        for(size_t i=0; i<rSystemVector.size(); ++i)                     //Loop through the systems, set start time, log nodes and measure simulation time
        {
            rSystemVector.at(i)->simulateAndMeasureTime(5);              //Measure time
            rSystemVector.at(i)->initialize(startT, stopT);
        }
        sortSystemsByTotalMeasuredTime(rSystemVector);                   //Sort systems by total measured time
        mSplitSystemVector = distributeSystems(rSystemVector, nThreads); //Distribute systems evenly over split vectors
    }

    tbb::task_group *simTasks;                                          //Initialize TBB routines for parallel simulation
    simTasks = new tbb::task_group;
    for(size_t t=0; t < min(nThreads,mSplitSystemVector.size()); ++t)                                  //Execute simulation
    {
        simTasks->run(taskSimWholeSystems(mSplitSystemVector[t], stopT));
    }
    simTasks->wait();                                                   //Wait for all tasks to finish
    delete(simTasks);

    bool aborted=false;
    for(size_t i=0; i<rSystemVector.size(); ++i)
    {
        aborted = aborted && rSystemVector[i]->wasSimulationAborted();
    }
    return !aborted;
#else
    // Use single core simulation if no TBB support
    return simulateMultipleSystems(stopT, rSystemVector);
#endif
}

AliasHandler::AliasHandler(ComponentSystem *pSystem)
{
    mpSystem = pSystem;
}

HString AliasHandler::getVariableAlias(const HString &rCompName, const HString &rPortName, const HString &rVarName)
{
    Component *pComp = mpSystem->getSubComponent(rCompName);
    if (pComp)
    {
        Port *pPort = pComp->getPort(rPortName);
        if (pPort)
        {
            int id = pPort->getNodeDataIdFromName(rVarName);
            return pPort->getVariableAlias(id);
        }
    }
    return "";
}

//! @todo maybe this should be the default version, right now search comp/port twice
bool AliasHandler::setVariableAlias(const HString &rAlias, const HString &rCompName, const HString &rPortName, const HString &rVarName)
{
    Component *pComp = mpSystem->getSubComponent(rCompName);
    if (pComp)
    {
        Port *pPort = pComp->getPort(rPortName);
        if (pPort)
        {
            int id = pPort->getNodeDataIdFromName(rVarName);
            return setVariableAlias(rAlias, rCompName, rPortName, id);
        }
    }
    return false;
}

bool AliasHandler::setVariableAlias(const HString &rAlias, const HString &rCompName, const HString &rPortName, const int varId)
{
    if (varId<0)
    {
        mpSystem->addErrorMessage("Can not set alias for dataId < 0 (incorrect variable name)");
        return false;
    }

    if (!isNameValid(rAlias))
    {
        mpSystem->addErrorMessage("Invalid characters in requested alias name: "+rAlias);
        return false;
    }

    //! @todo must check if existing alias is set for the same component that already have it to avoid warning
    // Check if alias already exist
    if (hasAlias(rAlias))
    {
        HString comp,port;
        int var;
        getVariableFromAlias(rAlias,comp,port,var);
        if ( (comp==rCompName) && (port==rPortName) && (var==varId) )
        {
            // We are setting the same alias again, skip without warning
            return true;
        }
        else
        {
            // The alias already exist somewhere else
            mpSystem->addErrorMessage("Alias: "+rAlias+" already exist");
            return false;
        }
    }

    if (mpSystem->hasReservedUniqueName(rAlias))
    {
        mpSystem->addErrorMessage("The alias: " + rAlias + " is already used as some other name");
        return false;
    }

    // Set the alias for the given component port and var
    Component *pComp = mpSystem->getSubComponent(rCompName);
    if (pComp)
    {
        Port *pPort = pComp->getPort(rPortName);
        if (pPort)
        {
            // First unregister the old alias (if it exists)
            HString prevAlias = pPort->getVariableAlias(varId);
            if (!prevAlias.empty())
            {
                //! @todo the remove will search for port again all the way, maybe have a special remove to use when we know the port and id already
                removeAlias(prevAlias);
            }

            // If alias is non empty, set it
            if (!rAlias.empty())
            {
                //! @todo do we need to check if this is OK ??
                pPort->setVariableAlias(rAlias, varId);

                ParamOrVariableT data = {Variable, rCompName, rPortName};
                mAliasMap.insert(std::pair<HString, ParamOrVariableT>(rAlias, data));
                mpSystem->reserveUniqueName(rAlias);
            }
            return true;
        }
    }
    mpSystem->addErrorMessage("Component or Port not found when setting alias");
    return false;
}

bool AliasHandler::setParameterAlias(const HString & /*alias*/, const HString & /*compName*/, const HString & /*parameterName*/)
{
    mpSystem->addErrorMessage("AliasHandler::setParameterAlias has not been implemented");
    return false;
}

void AliasHandler::componentRenamed(const HString &rOldCompName, const HString &rNewCompName)
{
    std::map<HString, ParamOrVariableT>::iterator it=mAliasMap.begin();
    while(it!=mAliasMap.end())
    {
        if (it->second.componentName == rOldCompName)
        {
            HString alias = it->first;
            ParamOrVariableT data = it->second;
            mAliasMap.erase(it);
            data.componentName = rNewCompName;

            // Re-insert data (with new comp name)
            mAliasMap.insert(std::pair<HString, ParamOrVariableT>(alias, data));

            // Restart search for more components
            it = mAliasMap.begin();
        }
        else
        {
            ++it;
        }
    }
}

void AliasHandler::portRenamed(const HString & /*compName*/, const HString & /*oldPortName*/, const HString & /*newPortName*/)
{
    mpSystem->addErrorMessage("AliasHandler::portRenamed has not been implemented");
}

void AliasHandler::componentRemoved(const HString &rCompName)
{
    std::map<HString, ParamOrVariableT>::iterator it=mAliasMap.begin();
    while (it!=mAliasMap.end())
    {
        if (it->second.componentName == rCompName)
        {
            removeAlias(it->first);
            it = mAliasMap.begin(); //Restart search for more components
        }
        else
        {
            ++it;
        }
    }
}

void AliasHandler::portRemoved(const HString & /*compName*/, const HString & /*portName*/)
{
    mpSystem->addErrorMessage("AliasHandler::portRemoved has not been implemented");
}

bool AliasHandler::hasAlias(const HString &rAlias)
{
    if (mAliasMap.count(rAlias)>0)
    {
        return true;
    }
    return false;
}

bool AliasHandler::removeAlias(const HString &rAlias)
{
    std::map<HString, ParamOrVariableT>::iterator it = mAliasMap.find(rAlias);
    if (it != mAliasMap.end())
    {
        if (it->second.type == Variable)
        {
            Component *pComp = mpSystem->getSubComponent(it->second.componentName);
            if (pComp)
            {
                Port *pPort = pComp->getPort(it->second.name);
                if (pPort)
                {
                    int id = pPort->getVariableIdByAlias(rAlias);
                    pPort->setVariableAlias("",id); //Remove variable alias
                }
            }
        }
        mpSystem->unReserveUniqueName(rAlias); //We must unreserve before erasing the it, since rAlias may be a reference to data in it
        mAliasMap.erase(it);
        return true;
    }
    return false;
}

std::vector<HString> AliasHandler::getAliases() const
{
    vector<HString> aliasNames;
    aliasNames.reserve(mAliasMap.size());

    std::map<HString, ParamOrVariableT>::const_iterator it;
    for (it=mAliasMap.begin(); it!=mAliasMap.end(); ++it)
    {
        aliasNames.push_back(it->first);
    }
    return aliasNames;
}

void AliasHandler::getVariableFromAlias(const HString &rAlias, HString &rCompName, HString &rPortName, int &rVarId)
{
    // Clear return vars to indicate any failure
    rCompName.clear(); rPortName.clear(); rVarId=-1;

    // Search through map for specified alias
    std::map<HString, ParamOrVariableT>::iterator it;
    it = mAliasMap.find(rAlias);
    if (it != mAliasMap.end())
    {
        if (it->second.type == Variable)
        {
            rCompName = it->second.componentName;
            rPortName = it->second.name;

            // Lookup varName from port
            Component* pComp = mpSystem->getSubComponent(rCompName);
            if (pComp)
            {
                Port *pPort = pComp->getPort(rPortName);
                if (pPort)
                {
                    rVarId = pPort->getVariableIdByAlias(rAlias);
                }
            }
        }
    }
}

void AliasHandler::getVariableFromAlias(const HString &rAlias, HString &rCompName, HString &rPortName, HString &rVarName)
{
    // Clear return vars to indicate any failure
    rCompName.clear(); rPortName.clear(); rVarName.clear();

    // Search through map for specified alias
    AliasMapT::iterator it = mAliasMap.find(rAlias);
    if (it != mAliasMap.end())
    {
        if (it->second.type == Variable)
        {
            rCompName = it->second.componentName;
            rPortName = it->second.name;

            // Lookup varName from port
            Component* pComp = mpSystem->getSubComponent(rCompName);
            if (pComp)
            {
                Port *pPort = pComp->getPort(rPortName);
                if (pPort)
                {
                    int id = pPort->getVariableIdByAlias(rAlias);
                    const NodeDataDescription *pDataDesc = pPort->getNodeDataDescription(id);
                    if (pDataDesc)
                    {
                        rVarName = pDataDesc->name;
                    }
                }
            }
        }
    }
}

void AliasHandler::getParameterFromAlias(const HString & /*alias*/, HString &/*rCompName*/, HString &/*rParameterName*/)
{
    mpSystem->addErrorMessage("AliasHandler::getParameterFromAlias has not been implemented");
}


void ConditionalComponentSystem::configure()
{
    addInputVariable("on", "On/off condition, 1=on, 0=off", "", 0, &mpCondition);

    mAsleep = false;
}

void ConditionalComponentSystem::simulate(const double stopT)
{
    if((*mpCondition)>0)
    {
        if(mAsleep)
        {
            for(SubComponentMapT::iterator it = mSubComponentMap.begin(); it != mSubComponentMap.end(); ++it)
            {
                it->second->mTime = this->mTime;
            }
            mAsleep = false;
        }

        ComponentSystem::simulate(stopT);
    }
    else
    {
        // Round to nearest, we may not get exactly the stop time that we want
        size_t numSimulationSteps = calcNumSimSteps(mTime, stopT); //Here mTime is the last time step since it is not updated yet

        //Simulate
        for (size_t i=0; i<numSimulationSteps; ++i)
        {
            mTime += mTimestep; //mTime is updated here before the simulation,
                                //mTime is the current time during the simulateOneTimestep

            ++mTotalTakenSimulationSteps;

            logTimeAndNodes(mTotalTakenSimulationSteps);
        }

        mAsleep = true;
    }
}

void ConditionalComponentSystem::simulateMultiThreaded(const double startT, const double stopT, const size_t nDesiredThreads, const bool noChanges, ParallelAlgorithmT algorithm)
{
    ComponentSystem::simulateMultiThreaded(startT,  stopT, nDesiredThreads, noChanges, algorithm);
}
