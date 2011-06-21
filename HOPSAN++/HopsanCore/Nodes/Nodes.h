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
//! @file   Nodes.h
//! @author FluMeS
//! @date   2010-01-08
//! @brief Contains all built in node types
//!
//$Id$

#ifndef NODES_H_INCLUDED
#define NODES_H_INCLUDED

#include "../Node.h"

#include <iostream>

namespace hopsan {

    DLLIMPORTEXPORT void register_nodes(NodeFactory* nfampND_ct);

    //!
    //! @class NodeSignal
    //! @brief A signal node
    //! @ingroup SignalNode
    //!
    class NodeSignal :public Node
    {
    public:
        enum {VALUE, DATALENGTH};
        static Node* CreatorFunction() {return new NodeSignal;}

    private:
        NodeSignal() : Node(DATALENGTH)
        {
            setDataCharacteristics(VALUE, "Value", "-");
        }
    };


    //!
    //! @class NodeHydraulic
    //! @brief A hydraulic node
    //! @ingroup HydraulicNode
    //!
    class NodeHydraulic :public Node
    {
    public:
        enum {FLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP, HEATFLOW, DATALENGTH};
        static Node* CreatorFunction() {return new NodeHydraulic;}

    private:
        NodeHydraulic() : Node(DATALENGTH)
        {
            setDataCharacteristics(FLOW, "Flow", "m^3/s");
            setDataCharacteristics(PRESSURE, "Pressure", "Pa");
            setDataCharacteristics(TEMPERATURE, "Temperature", "K", Node::NOPLOT);
            setDataCharacteristics(WAVEVARIABLE, "WaveVariable", "Pa", Node::NOPLOT);
            setDataCharacteristics(CHARIMP, "CharImp", "?", Node::NOPLOT);
            setDataCharacteristics(HEATFLOW, "HeatFlow", "?", Node::NOPLOT);

//            setData(NodeHydraulic::PRESSURE, 1.0e5);
//            setData(NodeHydraulic::FLOW, 0.0);
        }

        virtual void setSpecialStartValues(Node *pNode)
        {
            for(int i=0; i<mDataNames.size(); ++i)
            {
                if(WAVEVARIABLE==i)
                {
                    pNode->setData(i, mDataVector[PRESSURE]);
                    std::cout << "SpecialStartValue: Name: " << mDataNames[i] << "  Value: " << mDataVector[i] << "  Unit: " << mDataUnits[i] << std::endl;
                }
                //! todo Maybe also write CHARIMP?
            }
        }
    };


    //!
    //! @class NodePneumatic
    //! @brief A pneumatic node
    //! @ingroup PneumaticNode
    //!
    class NodePneumatic :public Node
    {
    public:
        enum {MASSFLOW, ENERGYFLOW, PRESSURE, TEMPERATURE, WAVEVARIABLE, CHARIMP, HEATFLOW, DATALENGTH};
        static Node* CreatorFunction() {return new NodePneumatic;}

    private:
        NodePneumatic() : Node(DATALENGTH)
        {
            setDataCharacteristics(MASSFLOW, "MassFlow", "kg/s");
            setDataCharacteristics(PRESSURE, "Pressure", "Pa");
            setDataCharacteristics(TEMPERATURE, "Temperature", "K", Node::NOPLOT);
            setDataCharacteristics(WAVEVARIABLE, "WaveVariable", "Pa", Node::NOPLOT);
            setDataCharacteristics(CHARIMP, "CharImp", "?", Node::NOPLOT);
            setDataCharacteristics(ENERGYFLOW, "EnergyFlow", "J/s", Node::NOPLOT);

//            setData(NodeENERGYFLOW::PRESSURE, 1.0e5);
//            setData(NodeENERGYFLOW::FLOW, 0.0);
        }

        virtual void setSpecialStartValues(Node *pNode)
        {
            for(size_t i=0; i<mDataNames.size(); ++i)
            {
                if(WAVEVARIABLE==i)
                {
                    pNode->setData(i, mDataVector[PRESSURE]);
                    std::cout << "SpecialStartValue: Name: " << mDataNames[i] << "  Value: " << mDataVector[i] << "  Unit: " << mDataUnits[i] << std::endl;
                }
                //! todo Maybe also write CHARIMP?
            }
        }
    };

    //!
    //! @class NodeMechanic
    //! @brief A mechanic node
    //! @ingroup MechanicalNode
    //!
    class NodeMechanic :public Node
    {
    public:
        enum {VELOCITY, FORCE, POSITION, WAVEVARIABLE, CHARIMP, EQMASS, DATALENGTH};
        static Node* CreatorFunction() {return new NodeMechanic;}

    private:
        NodeMechanic() : Node(DATALENGTH)
        {
            setDataCharacteristics(VELOCITY, "Velocity", "m/s");
            setDataCharacteristics(FORCE, "Force", "N");
            setDataCharacteristics(POSITION, "Position", "m");
            setDataCharacteristics(WAVEVARIABLE, "WaveVariable", "N", Node::NOPLOT);
            setDataCharacteristics(CHARIMP, "CharImp", "N s/m", Node::NOPLOT);
            setDataCharacteristics(EQMASS, "EquivalentMass", "kg");
        }

        virtual void setSpecialStartValues(Node *pNode)
        {
            for(int i=0; i<mDataNames.size(); ++i)
            {
                if(WAVEVARIABLE==i)
                {
                    pNode->setData(i, mDataVector[FORCE]);
                }
                //! todo Maybe also write CHARIMP?
            }
        }
    };

    //!
    //! @class NodeMechanicRotational
    //! @brief A rotational mechanic node
    //! @ingroup RotationalMechanicalNode
    //!
    class NodeMechanicRotational :public Node
    {
    public:
        enum {ANGULARVELOCITY, TORQUE, ANGLE, WAVEVARIABLE, CHARIMP, EQINERTIA, DATALENGTH};
        static Node* CreatorFunction() {return new NodeMechanicRotational;}

    private:
        NodeMechanicRotational() : Node(DATALENGTH)
        {
            setDataCharacteristics(ANGULARVELOCITY, "Angular Velocity", "rad/s");
            setDataCharacteristics(TORQUE, "Torque", "Nm");
            setDataCharacteristics(ANGLE, "Angle", "rad");
            setDataCharacteristics(WAVEVARIABLE, "WaveVariable", "Nm", Node::NOPLOT);
            setDataCharacteristics(CHARIMP, "CharImp", "?", Node::NOPLOT);
            setDataCharacteristics(EQINERTIA, "Equivalent Inertia", "kgm^2");
        }

        virtual void setSpecialStartValues(Node *pNode)
        {
            for(int i=0; i<mDataNames.size(); ++i)
            {
                if(WAVEVARIABLE==i)
                {
                    pNode->setData(i, mDataVector[TORQUE]);
                }
                //! todo Maybe also write CHARIMP?
            }
        }
    };



    //!
    //! @class NodeElectric
    //! @brief An electric node
    //! @ingroup ElectricNode
    //!
    class NodeElectric :public Node
    {
    public:
        enum {VOLTAGE, CURRENT, WAVEVARIABLE, CHARIMP, DATALENGTH};
        static Node* CreatorFunction() {return new NodeElectric;}

    private:
        NodeElectric() : Node(DATALENGTH)
        {
            setDataCharacteristics(VOLTAGE, "Voltage", "V");
            setDataCharacteristics(CURRENT, "Current", "A");
            setDataCharacteristics(WAVEVARIABLE, "WaveVariable", "V", Node::NOPLOT);
            setDataCharacteristics(CHARIMP, "CharImp", "V/A", Node::NOPLOT);
        }

        virtual void setSpecialStartValues(Node *pNode)
        {
            for(int i=0; i<mDataNames.size(); ++i)
            {
                if(WAVEVARIABLE==i)
                {
                    pNode->setData(i, mDataVector[VOLTAGE]);
                }
                //! todo Maybe also write CHARIMP?
            }
        }
    };
}

#endif // NODES_H_INCLUDED
