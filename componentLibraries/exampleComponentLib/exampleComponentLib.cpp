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

// Include your component code code files here
// If you have lots of them you can include them in separate .h files and then include those files here instead.

#include "HydraulicComponents/MyExampleOrifice.hpp"
#include "HydraulicComponents/MyExampleConstantOrifice.hpp"
#include "HydraulicComponents/MyExampleVolume.hpp"
#include "HydraulicComponents/MyExampleVolume2.hpp"
#include "SignalComponents/MyExampleSignalSum.hpp"

// You need to include ComponentEssentials.h in order to gain access to the register function and the Factory types
// Also use the hopsan namespace

#include "ComponentEssentials.h"
using namespace hopsan;

// When you load your model into Hopsan, the register_contents() function bellow will be called
// It will register YOUR components into the Hopsan ComponentFactory

//! @brief A energy node
//! @ingroup NodeEnergy
class NodeEnergy :public Node
{
public:
    //! @brief The data variable indexes, DataLength is used internally
    //! @ingroup NodeEnergy
    enum DataIndexEnumT {mFlow, Pressure, Temperature, DataLength};
    static Node* CreatorFunction() {return new NodeEnergy;}

private:
    NodeEnergy() : Node(DataLength)
    {
        setNiceName("energy");
        setDataCharacteristics(mFlow, "mFlow", "mdot", "mFlow", FlowType);
        setDataCharacteristics(Pressure, "Pressure", "p", "Pressure", IntensityType);
        setDataCharacteristics(Temperature, "Temperature", "T", "Temperature");
    }
};

class MyEnergy1 : public ComponentSignal
{
private:
    // Private member variables
    //size_t mnInputs;
    Port *mpInPort, *mpOutPort;

public:
    // The creator function that is registered when a component lib is loaded into Hopsan
    // This static function is mandatory
    static Component *Creator()
    {
        return new MyEnergy1();
    }

    // The Configure function that is run ONCE when a new object of the class is created
    // Use this function to set initial member variable values and to register Ports, Parameters and Startvalues
    // This function is mandatory
    void configure()
    {
        // Add ports to the component
        mpInPort = addReadPort("in", "NodeEnergy", "", Port::NotRequired);
        mpOutPort = addOutputVariable("out", "The energy of input", "");
    }

    // The initialize function is called before simulation begins.
    // It may be called multiple times
    // In this function you can read or write from/to nodes
    // This function is optional but most likely needed
    void initialize()
    {
        // We assume that no-one will be disconnecting during simulation (that is not allowed)
        //mnInputs = mpMultiInPort->getNumPorts();
        // Simulate one timestep in order to initialize the output
        simulateOneTimestep();
    }

    // The simulateOneTimestep() function is called ONCE every time step
    // This function contains the actual component simulation equations
    // This function is mandatory
    void simulateOneTimestep()
    {
        // Initialize sum variable
        double sum = 0;

        // Sum all input ports
        // This index i, represents each subport in the multiport
        auto p = mpInPort->readNode(NodeEnergy::Pressure);
        auto q = mpInPort->readNode(NodeEnergy::mFlow);

        //Write value to output node
        mpOutPort->writeNode(NodeSignal::Value, p*q);
    }

    // The finalize function is called after simulation ends
    // It may be called multiple times
    // Use this function to clean up of any custom allocations made in Initialize (if needed)
    // This function is optional
    //void finalize()
    //{

    //}

    // The deconfigure function is called just before a component is deleted
    // Use it to clean up after any custom allocation in the configure function
    // This function is optional
    //void deconfigure()
    //{
        // Nothing to deconfigure
    //}
};

class MyEnergy2 : public ComponentSignal
{
private:
    // Private member variables
    Port *mpInQPort, *mpInPPort, *mpOutPort;

public:
    // The creator function that is registered when a component lib is loaded into Hopsan
    // This static function is mandatory
    static Component *Creator()
    {
        return new MyEnergy2();
    }

    // The Configure function that is run ONCE when a new object of the class is created
    // Use this function to set initial member variable values and to register Ports, Parameters and Startvalues
    // This function is mandatory
    void configure()
    {
        // Add ports to the component
        mpInPPort = addInputVariable("p", "","", 0);
        mpInQPort = addInputVariable("q", "","", 0);
        mpOutPort = addWritePort("out", "NodeEnergy", "", Port::NotRequired);
    }

    // The initialize function is called before simulation begins.
    // It may be called multiple times
    // In this function you can read or write from/to nodes
    // This function is optional but most likely needed
    void initialize()
    {
        // We assume that no-one will be disconnecting during simulation (that is not allowed)
        // Simulate one timestep in order to initialize the output
        simulateOneTimestep();
    }

    // The simulateOneTimestep() function is called ONCE every time step
    // This function contains the actual component simulation equations
    // This function is mandatory
    void simulateOneTimestep()
    {
        // This index i, represents each subport in the multiport
        auto p = mpInPPort->readNode(NodeSignal::Value);
        auto q = mpInQPort->readNode(NodeSignal::Value);

        //Write value to output node
        mpOutPort->writeNode(NodeEnergy::Pressure, p);
        mpOutPort->writeNode(NodeEnergy::mFlow, q);
    }

    // The finalize function is called after simulation ends
    // It may be called multiple times
    // Use this function to clean up of any custom allocations made in Initialize (if needed)
    // This function is optional
    //void finalize()
    //{

    //}

    // The deconfigure function is called just before a component is deleted
    // Use it to clean up after any custom allocation in the configure function
    // This function is optional
    //void deconfigure()
    //{
        // Nothing to deconfigure
    //}
};


extern "C" DLLEXPORT void register_contents(ComponentFactory* pComponentFactory, NodeFactory* pNodeFactory)
{
    // ========== Register Components ==========
    // Use the registerCreatorFunction(KeyValue, Function) in the component factory to register components
    // The KeyValue is a text string with the TypeName of the component.
    // This value must be unique for every component in Hopsan.
    // If a typename is already in use, your component will not be added.
    // Suggestion, let the KeyValue (TypeName) be the same as your Class name
    // If that name is already in use, use something similar

    pComponentFactory->registerCreatorFunction("MyExampleOrifice", MyExampleOrifice::Creator);
    pComponentFactory->registerCreatorFunction("MyExampleConstantOrifice", MyExampleConstantOrifice::Creator);
    pComponentFactory->registerCreatorFunction("MyExampleVolume", MyExampleVolume::Creator);
    pComponentFactory->registerCreatorFunction("MyExampleVolume2", MyExampleVolume2::Creator);
    pComponentFactory->registerCreatorFunction("MyExampleSignalSum", MyExampleSignalSum::Creator);

    pComponentFactory->registerCreatorFunction("MyEnergy1", MyEnergy1::Creator);
    pComponentFactory->registerCreatorFunction("MyEnergy2", MyEnergy2::Creator);

    // ========== Register Custom Nodes (if any) ==========
    // This is not yet supported
    pNodeFactory->registerCreatorFunction("NodeEnergy", NodeEnergy::CreatorFunction);
    HOPSAN_UNUSED(pNodeFactory)
}

// When you load your model into Hopsan, the get_hopsan_info() function bellow will be called
// This information is used to make sure that your component and the hopsan core have the same version

extern "C" DLLEXPORT void get_hopsan_info(HopsanExternalLibInfoT *pHopsanExternalLibInfo)
{
    // Change the name of the lib to something unique
    // You can include numbers in your name to indicate library version (if you want)
    pHopsanExternalLibInfo->libName = (char*)"HopsanExampleComponentLibrary";

    // Leave these two lines as they are
    pHopsanExternalLibInfo->hopsanCoreVersion = (char*)HOPSANCOREVERSION;
    pHopsanExternalLibInfo->libCompiledDebugRelease = (char*)HOPSAN_BUILD_TYPE_STR;

}
