#ifndef MECHANICM3LOAD1D_HPP_INCLUDED
#define MECHANICM3LOAD1D_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file MechanicM3load1D.hpp
//! @author Petter Krus <petter.krus@liu.se>, Martin Hochwallner \
<martin.hochwallner@liu.se
//! @date Tue 14 Apr 2015 15:39:54
//! @brief An inertia load with spring and damper
//! @ingroup MechanicComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, data, Hopsan, hopsan, componentLibraries, defaultLibrary, Special, \
MechanicB}/MechanicM3load1D.nb*/

using namespace hopsan;

class MechanicM3load1D : public ComponentQ
{
private:
     double m1Mass;
     double m2Mass;
     double m3Mass;
     double m1FrictionViscousCoeff;
     double m2FrictionViscousCoeff;
     double m3FrictionViscousCoeff;
     double m12FrictionViscousCoeff;
     double m13FrictionViscousCoeff;
     double bfc;
     double m1PositionLimitNeg;
     double m1PositionLimitPos;
     double m1PositionOffset;
     double m1Direction;
     double m2PositionLimitNeg;
     double m2PositionLimitPos;
     double m2PositionOffset;
     double m2Direction;
     double m3PositionLimitNeg;
     double m3PositionLimitPos;
     double m3PositionOffset;
     double m3Direction;
     Port *mpPm1;
     Port *mpPm2;
     Port *mpPm3;
     double delayParts1[9];
     double delayParts2[9];
     double delayParts3[9];
     double delayParts4[9];
     double delayParts5[9];
     double delayParts6[9];
     double delayParts7[9];
     double delayParts8[9];
     double delayParts9[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     double jsyseqnweight[4];
     int order[9];
     int mNstep;
     //Port Pm1 variable
     double fm1;
     double xm1;
     double vm1;
     double cm1;
     double Zcm1;
     double eqMassm1;
     //Port Pm2 variable
     double fm2;
     double xm2;
     double vm2;
     double cm2;
     double Zcm2;
     double eqMassm2;
     //Port Pm3 variable
     double fm3;
     double xm3;
     double vm3;
     double cm3;
     double Zcm3;
     double eqMassm3;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double m1FrictionCoulomb;
     double m2FrictionCoulomb;
     double m3FrictionCoulomb;
     double m12FrictionCoulomb;
     double m13FrictionCoulomb;
     //outputVariables
     double m1Position;
     double m2Position;
     double m3Position;
     double m1Velocity;
     double m2Velocity;
     double m3Velocity;
     double m1FrictionForce;
     double m2FrictionForce;
     double m3FrictionForce;
     double m12FrictionForce;
     double m13FrictionForce;
     //Expressions variables
     //Port Pm1 pointer
     double *mpND_fm1;
     double *mpND_xm1;
     double *mpND_vm1;
     double *mpND_cm1;
     double *mpND_Zcm1;
     double *mpND_eqMassm1;
     //Port Pm2 pointer
     double *mpND_fm2;
     double *mpND_xm2;
     double *mpND_vm2;
     double *mpND_cm2;
     double *mpND_Zcm2;
     double *mpND_eqMassm2;
     //Port Pm3 pointer
     double *mpND_fm3;
     double *mpND_xm3;
     double *mpND_vm3;
     double *mpND_cm3;
     double *mpND_Zcm3;
     double *mpND_eqMassm3;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mpm1FrictionCoulomb;
     double *mpm2FrictionCoulomb;
     double *mpm3FrictionCoulomb;
     double *mpm12FrictionCoulomb;
     double *mpm13FrictionCoulomb;
     //inputParameters pointers
     double *mpm1Mass;
     double *mpm2Mass;
     double *mpm3Mass;
     double *mpm1FrictionViscousCoeff;
     double *mpm2FrictionViscousCoeff;
     double *mpm3FrictionViscousCoeff;
     double *mpm12FrictionViscousCoeff;
     double *mpm13FrictionViscousCoeff;
     double *mpbfc;
     double *mpm1PositionLimitNeg;
     double *mpm1PositionLimitPos;
     double *mpm1PositionOffset;
     double *mpm1Direction;
     double *mpm2PositionLimitNeg;
     double *mpm2PositionLimitPos;
     double *mpm2PositionOffset;
     double *mpm2Direction;
     double *mpm3PositionLimitNeg;
     double *mpm3PositionLimitPos;
     double *mpm3PositionOffset;
     double *mpm3Direction;
     //outputVariables pointers
     double *mpm1Position;
     double *mpm2Position;
     double *mpm3Position;
     double *mpm1Velocity;
     double *mpm2Velocity;
     double *mpm3Velocity;
     double *mpm1FrictionForce;
     double *mpm2FrictionForce;
     double *mpm3FrictionForce;
     double *mpm12FrictionForce;
     double *mpm13FrictionForce;
     Delay mDelayedPart10;
     Delay mDelayedPart11;
     Delay mDelayedPart20;
     Delay mDelayedPart21;
     Delay mDelayedPart30;
     Delay mDelayedPart31;
     Delay mDelayedPart40;
     Delay mDelayedPart41;
     Delay mDelayedPart50;
     Delay mDelayedPart51;
     Delay mDelayedPart60;
     Delay mDelayedPart61;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new MechanicM3load1D();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(9,9);
        systemEquations.create(9);
        delayedPart.create(10,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        mpPm1=addPowerPort("Pm1","NodeMechanic");
        mpPm2=addPowerPort("Pm2","NodeMechanic");
        mpPm3=addPowerPort("Pm3","NodeMechanic");
        //Add inputVariables to the component
            addInputVariable("m1FrictionCoulomb","Dry friction between M1 and \
inertial frame of reference (+/-)","N",10.,&mpm1FrictionCoulomb);
            addInputVariable("m2FrictionCoulomb","Dry friction between M2 and \
inertial frame of reference (+/-)","N",10.,&mpm2FrictionCoulomb);
            addInputVariable("m3FrictionCoulomb","Dry friction between M3 and \
inertial frame of reference (+/-)","N",10.,&mpm3FrictionCoulomb);
            addInputVariable("m12FrictionCoulomb","Dry friction between M1 \
and M2 (+/-)","N",10.,&mpm12FrictionCoulomb);
            addInputVariable("m13FrictionCoulomb","Dry friction between M1 \
and M3 (+/-)","N",10.,&mpm13FrictionCoulomb);

        //Add inputParammeters to the component
            addInputVariable("m1Mass", "Inertia of M1", "kg", 10.,&mpm1Mass);
            addInputVariable("m2Mass", "Inertia of M2", "kg", 1.,&mpm2Mass);
            addInputVariable("m3Mass", "Inertia of M3", "kg", 1.,&mpm3Mass);
            addInputVariable("m1FrictionViscousCoeff", "Visc. friction coeff. \
between M1 and inertial frame of reference", "Ns/m", \
10.,&mpm1FrictionViscousCoeff);
            addInputVariable("m2FrictionViscousCoeff", "Visc. friction coeff. \
between M2 and inertial frame of reference", "Ns/m", \
10.,&mpm2FrictionViscousCoeff);
            addInputVariable("m3FrictionViscousCoeff", "Visc. friction coeff. \
between M3 and inertial frame of reference", "Ns/m", \
10.,&mpm3FrictionViscousCoeff);
            addInputVariable("m12FrictionViscousCoeff", "Visc. friction \
coeff. between M1 and M2", "Ns/m", 10.,&mpm12FrictionViscousCoeff);
            addInputVariable("m13FrictionViscousCoeff", "Visc. friction \
coeff. between M1 and M3", "Ns/m", 10.,&mpm13FrictionViscousCoeff);
            addInputVariable("bfc", "Numerical friction factor.", "", \
1.,&mpbfc);
            addInputVariable("m1PositionLimitNeg", "Limitation on stroke M1", \
"m", -1000.,&mpm1PositionLimitNeg);
            addInputVariable("m1PositionLimitPos", "Limitation on stroke M1", \
"m", 1000.,&mpm1PositionLimitPos);
            addInputVariable("m1PositionOffset", "Offset for the position \
output of M1", "m", 0.,&mpm1PositionOffset);
            addInputVariable("m1Direction", "Direction 1 / -1 for the outputs \
of M1", "1", 1.,&mpm1Direction);
            addInputVariable("m2PositionLimitNeg", "Limitation on stroke M2", \
"m", -1000.,&mpm2PositionLimitNeg);
            addInputVariable("m2PositionLimitPos", "Limitation on stroke M2", \
"m", 1000.,&mpm2PositionLimitPos);
            addInputVariable("m2PositionOffset", "Offset for the position \
output of M2", "m", 0.,&mpm2PositionOffset);
            addInputVariable("m2Direction", "Direction 1 / -1 for the outputs \
of M2", "1", 1.,&mpm2Direction);
            addInputVariable("m3PositionLimitNeg", "Limitation on stroke M3", \
"m", -1000.,&mpm3PositionLimitNeg);
            addInputVariable("m3PositionLimitPos", "Limitation on stroke M3", \
"m", 1000.,&mpm3PositionLimitPos);
            addInputVariable("m3PositionOffset", "Offset for the position \
output of M3", "m", 0.,&mpm3PositionOffset);
            addInputVariable("m3Direction", "Direction 1 / -1 for the outputs \
of M3", "1", 1.,&mpm3Direction);
        //Add outputVariables to the component
            addOutputVariable("m1Position","Position of \
M1","m",0.,&mpm1Position);
            addOutputVariable("m2Position","Position of \
M2","m",0.,&mpm2Position);
            addOutputVariable("m3Position","Position of \
M3","m",0.,&mpm3Position);
            addOutputVariable("m1Velocity","Velocity of \
M1","m/s",0.,&mpm1Velocity);
            addOutputVariable("m2Velocity","Velocity of \
M2","m/s",0.,&mpm2Velocity);
            addOutputVariable("m3Velocity","Velocity of \
M3","m/s",0.,&mpm3Velocity);
            addOutputVariable("m1FrictionForce","friction force between M1 \
and inertial frame of reference","N",0.,&mpm1FrictionForce);
            addOutputVariable("m2FrictionForce","friction force between M2 \
and inertial frame of reference","N",0.,&mpm2FrictionForce);
            addOutputVariable("m3FrictionForce","friction force between M3 \
and inertial frame of reference","N",0.,&mpm3FrictionForce);
            addOutputVariable("m12FrictionForce","friction force between M1 \
and M2","N",0.,&mpm12FrictionForce);
            addOutputVariable("m13FrictionForce","friction force between M1 \
and M3","N",0.,&mpm13FrictionForce);

//==This code has been autogenerated using Compgen==
        //Add constantParameters
        mpSolver = new EquationSystemSolver(this,9);
     }

    void initialize()
     {
        //Read port variable pointers from nodes
        //Port Pm1
        mpND_fm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::Force);
        mpND_xm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::Position);
        mpND_vm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::Velocity);
        mpND_cm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::WaveVariable);
        mpND_Zcm1=getSafeNodeDataPtr(mpPm1, NodeMechanic::CharImpedance);
        mpND_eqMassm1=getSafeNodeDataPtr(mpPm1, \
NodeMechanic::EquivalentMass);
        //Port Pm2
        mpND_fm2=getSafeNodeDataPtr(mpPm2, NodeMechanic::Force);
        mpND_xm2=getSafeNodeDataPtr(mpPm2, NodeMechanic::Position);
        mpND_vm2=getSafeNodeDataPtr(mpPm2, NodeMechanic::Velocity);
        mpND_cm2=getSafeNodeDataPtr(mpPm2, NodeMechanic::WaveVariable);
        mpND_Zcm2=getSafeNodeDataPtr(mpPm2, NodeMechanic::CharImpedance);
        mpND_eqMassm2=getSafeNodeDataPtr(mpPm2, \
NodeMechanic::EquivalentMass);
        //Port Pm3
        mpND_fm3=getSafeNodeDataPtr(mpPm3, NodeMechanic::Force);
        mpND_xm3=getSafeNodeDataPtr(mpPm3, NodeMechanic::Position);
        mpND_vm3=getSafeNodeDataPtr(mpPm3, NodeMechanic::Velocity);
        mpND_cm3=getSafeNodeDataPtr(mpPm3, NodeMechanic::WaveVariable);
        mpND_Zcm3=getSafeNodeDataPtr(mpPm3, NodeMechanic::CharImpedance);
        mpND_eqMassm3=getSafeNodeDataPtr(mpPm3, \
NodeMechanic::EquivalentMass);

        //Read variables from nodes
        //Port Pm1
        fm1 = (*mpND_fm1);
        xm1 = (*mpND_xm1);
        vm1 = (*mpND_vm1);
        cm1 = (*mpND_cm1);
        Zcm1 = (*mpND_Zcm1);
        eqMassm1 = (*mpND_eqMassm1);
        //Port Pm2
        fm2 = (*mpND_fm2);
        xm2 = (*mpND_xm2);
        vm2 = (*mpND_vm2);
        cm2 = (*mpND_cm2);
        Zcm2 = (*mpND_Zcm2);
        eqMassm2 = (*mpND_eqMassm2);
        //Port Pm3
        fm3 = (*mpND_fm3);
        xm3 = (*mpND_xm3);
        vm3 = (*mpND_vm3);
        cm3 = (*mpND_cm3);
        Zcm3 = (*mpND_Zcm3);
        eqMassm3 = (*mpND_eqMassm3);

        //Read inputVariables from nodes
        m1FrictionCoulomb = (*mpm1FrictionCoulomb);
        m2FrictionCoulomb = (*mpm2FrictionCoulomb);
        m3FrictionCoulomb = (*mpm3FrictionCoulomb);
        m12FrictionCoulomb = (*mpm12FrictionCoulomb);
        m13FrictionCoulomb = (*mpm13FrictionCoulomb);

        //Read inputParameters from nodes
        m1Mass = (*mpm1Mass);
        m2Mass = (*mpm2Mass);
        m3Mass = (*mpm3Mass);
        m1FrictionViscousCoeff = (*mpm1FrictionViscousCoeff);
        m2FrictionViscousCoeff = (*mpm2FrictionViscousCoeff);
        m3FrictionViscousCoeff = (*mpm3FrictionViscousCoeff);
        m12FrictionViscousCoeff = (*mpm12FrictionViscousCoeff);
        m13FrictionViscousCoeff = (*mpm13FrictionViscousCoeff);
        bfc = (*mpbfc);
        m1PositionLimitNeg = (*mpm1PositionLimitNeg);
        m1PositionLimitPos = (*mpm1PositionLimitPos);
        m1PositionOffset = (*mpm1PositionOffset);
        m1Direction = (*mpm1Direction);
        m2PositionLimitNeg = (*mpm2PositionLimitNeg);
        m2PositionLimitPos = (*mpm2PositionLimitPos);
        m2PositionOffset = (*mpm2PositionOffset);
        m2Direction = (*mpm2Direction);
        m3PositionLimitNeg = (*mpm3PositionLimitNeg);
        m3PositionLimitPos = (*mpm3PositionLimitPos);
        m3PositionOffset = (*mpm3PositionOffset);
        m3Direction = (*mpm3Direction);

        //Read outputVariables from nodes
        m1Position = (*mpm1Position);
        m2Position = (*mpm2Position);
        m3Position = (*mpm3Position);
        m1Velocity = (*mpm1Velocity);
        m2Velocity = (*mpm2Velocity);
        m3Velocity = (*mpm3Velocity);
        m1FrictionForce = (*mpm1FrictionForce);
        m2FrictionForce = (*mpm2FrictionForce);
        m3FrictionForce = (*mpm3FrictionForce);
        m12FrictionForce = (*mpm12FrictionForce);
        m13FrictionForce = (*mpm13FrictionForce);

//==This code has been autogenerated using Compgen==


        //Initialize delays
        delayParts1[1] = (fm1*mTimestep - 2*m1Mass*vm1 + \
m12FrictionViscousCoeff*mTimestep*vm1 + m13FrictionViscousCoeff*mTimestep*vm1 \
+ m1FrictionViscousCoeff*mTimestep*vm1 - \
m12FrictionViscousCoeff*mTimestep*vm2 - m13FrictionViscousCoeff*mTimestep*vm3 \
- \
mTimestep*limit((-2*bfc*m1Mass*vm1)/mTimestep,-m1FrictionCoulomb,m1FrictionCo\
ulomb) - mTimestep*limit((-2*bfc*m1Mass*m2Mass*(vm1 - vm2))/((m1Mass + \
m2Mass)*mTimestep),-m12FrictionCoulomb,m12FrictionCoulomb) - \
mTimestep*limit((-2*bfc*m1Mass*m3Mass*(vm1 - vm3))/((m1Mass + \
m3Mass)*mTimestep),-m13FrictionCoulomb,m13FrictionCoulomb))/(2*m1Mass + \
m12FrictionViscousCoeff*mTimestep + m13FrictionViscousCoeff*mTimestep + \
m1FrictionViscousCoeff*mTimestep);
        mDelayedPart11.initialize(mNstep,delayParts1[1]);
        delayParts2[1] = (fm2*mTimestep - \
m12FrictionViscousCoeff*mTimestep*vm1 - 2*m2Mass*vm2 + \
m12FrictionViscousCoeff*mTimestep*vm2 + m2FrictionViscousCoeff*mTimestep*vm2 \
+ mTimestep*limit((-2*bfc*m1Mass*m2Mass*(vm1 - vm2))/((m1Mass + \
m2Mass)*mTimestep),-m12FrictionCoulomb,m12FrictionCoulomb) - \
mTimestep*limit((-2*bfc*m2Mass*vm2)/mTimestep,-m2FrictionCoulomb,m2FrictionCo\
ulomb))/(2*m2Mass + m12FrictionViscousCoeff*mTimestep + \
m2FrictionViscousCoeff*mTimestep);
        mDelayedPart21.initialize(mNstep,delayParts2[1]);
        delayParts3[1] = (fm3*mTimestep - \
m13FrictionViscousCoeff*mTimestep*vm1 - 2*m3Mass*vm3 + \
m13FrictionViscousCoeff*mTimestep*vm3 + m3FrictionViscousCoeff*mTimestep*vm3 \
+ mTimestep*limit((-2*bfc*m1Mass*m3Mass*(vm1 - vm3))/((m1Mass + \
m3Mass)*mTimestep),-m13FrictionCoulomb,m13FrictionCoulomb) - \
mTimestep*limit((-2*bfc*m3Mass*vm3)/mTimestep,-m3FrictionCoulomb,m3FrictionCo\
ulomb))/(2*m3Mass + m13FrictionViscousCoeff*mTimestep + \
m3FrictionViscousCoeff*mTimestep);
        mDelayedPart31.initialize(mNstep,delayParts3[1]);
        delayParts4[1] = (-(mTimestep*vm1) - 2*xm1)/2.;
        mDelayedPart41.initialize(mNstep,delayParts4[1]);
        delayParts5[1] = (-(mTimestep*vm2) - 2*xm2)/2.;
        mDelayedPart51.initialize(mNstep,delayParts5[1]);
        delayParts6[1] = (-(mTimestep*vm3) - 2*xm3)/2.;
        mDelayedPart61.initialize(mNstep,delayParts6[1]);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];
        delayedPart[6][1] = delayParts6[1];
        delayedPart[7][1] = delayParts7[1];
        delayedPart[8][1] = delayParts8[1];
        delayedPart[9][1] = delayParts9[1];
     }
    void simulateOneTimestep()
     {
        Vec stateVar(9);
        Vec stateVark(9);
        Vec deltaStateVar(9);

        //Read variables from nodes
        //Port Pm1
        cm1 = (*mpND_cm1);
        Zcm1 = (*mpND_Zcm1);
        //Port Pm2
        cm2 = (*mpND_cm2);
        Zcm2 = (*mpND_Zcm2);
        //Port Pm3
        cm3 = (*mpND_cm3);
        Zcm3 = (*mpND_Zcm3);

        //Read inputVariables from nodes
        m1FrictionCoulomb = (*mpm1FrictionCoulomb);
        m2FrictionCoulomb = (*mpm2FrictionCoulomb);
        m3FrictionCoulomb = (*mpm3FrictionCoulomb);
        m12FrictionCoulomb = (*mpm12FrictionCoulomb);
        m13FrictionCoulomb = (*mpm13FrictionCoulomb);

        //Read inputParameters from nodes
        m1Mass = (*mpm1Mass);
        m2Mass = (*mpm2Mass);
        m3Mass = (*mpm3Mass);
        m1FrictionViscousCoeff = (*mpm1FrictionViscousCoeff);
        m2FrictionViscousCoeff = (*mpm2FrictionViscousCoeff);
        m3FrictionViscousCoeff = (*mpm3FrictionViscousCoeff);
        m12FrictionViscousCoeff = (*mpm12FrictionViscousCoeff);
        m13FrictionViscousCoeff = (*mpm13FrictionViscousCoeff);
        bfc = (*mpbfc);
        m1PositionLimitNeg = (*mpm1PositionLimitNeg);
        m1PositionLimitPos = (*mpm1PositionLimitPos);
        m1PositionOffset = (*mpm1PositionOffset);
        m1Direction = (*mpm1Direction);
        m2PositionLimitNeg = (*mpm2PositionLimitNeg);
        m2PositionLimitPos = (*mpm2PositionLimitPos);
        m2PositionOffset = (*mpm2PositionOffset);
        m2Direction = (*mpm2Direction);
        m3PositionLimitNeg = (*mpm3PositionLimitNeg);
        m3PositionLimitPos = (*mpm3PositionLimitPos);
        m3PositionOffset = (*mpm3PositionOffset);
        m3Direction = (*mpm3Direction);

        //LocalExpressions

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = vm1;
        stateVark[1] = vm2;
        stateVark[2] = vm3;
        stateVark[3] = xm1;
        stateVark[4] = xm2;
        stateVark[5] = xm3;
        stateVark[6] = fm1;
        stateVark[7] = fm2;
        stateVark[8] = fm3;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //M3load1D
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =vm1 - dxLimit(limit((mTimestep*vm1)/2. - \
delayedPart[4][1],m1PositionLimitNeg,m1PositionLimitPos),m1PositionLimitNeg,m\
1PositionLimitPos)*((mTimestep*(-fm1 + m12FrictionViscousCoeff*vm2 + \
m13FrictionViscousCoeff*vm3 + \
limit((-2*bfc*m1Mass*vm1)/mTimestep,-m1FrictionCoulomb,m1FrictionCoulomb) + \
limit((-2*bfc*m1Mass*m2Mass*(vm1 - vm2))/((m1Mass + \
m2Mass)*mTimestep),-m12FrictionCoulomb,m12FrictionCoulomb) + \
limit((-2*bfc*m1Mass*m3Mass*(vm1 - vm3))/((m1Mass + \
m3Mass)*mTimestep),-m13FrictionCoulomb,m13FrictionCoulomb)))/(2*m1Mass + \
(m12FrictionViscousCoeff + m13FrictionViscousCoeff + \
m1FrictionViscousCoeff)*mTimestep) - delayedPart[1][1]);
          systemEquations[1] =vm2 - dxLimit(limit((mTimestep*vm2)/2. - \
delayedPart[5][1],m2PositionLimitNeg,m2PositionLimitPos),m2PositionLimitNeg,m\
2PositionLimitPos)*(-((mTimestep*(fm2 - m12FrictionViscousCoeff*vm1 + \
limit((-2*bfc*m1Mass*m2Mass*(vm1 - vm2))/((m1Mass + \
m2Mass)*mTimestep),-m12FrictionCoulomb,m12FrictionCoulomb) - \
limit((-2*bfc*m2Mass*vm2)/mTimestep,-m2FrictionCoulomb,m2FrictionCoulomb)))/(\
2*m2Mass + (m12FrictionViscousCoeff + m2FrictionViscousCoeff)*mTimestep)) - \
delayedPart[2][1]);
          systemEquations[2] =vm3 - dxLimit(limit((mTimestep*vm3)/2. - \
delayedPart[6][1],m3PositionLimitNeg,m3PositionLimitPos),m3PositionLimitNeg,m\
3PositionLimitPos)*(-((mTimestep*(fm3 - m13FrictionViscousCoeff*vm1 + \
limit((-2*bfc*m1Mass*m3Mass*(vm1 - vm3))/((m1Mass + \
m3Mass)*mTimestep),-m13FrictionCoulomb,m13FrictionCoulomb) - \
limit((-2*bfc*m3Mass*vm3)/mTimestep,-m3FrictionCoulomb,m3FrictionCoulomb)))/(\
2*m3Mass + (m13FrictionViscousCoeff + m3FrictionViscousCoeff)*mTimestep)) - \
delayedPart[3][1]);
          systemEquations[3] =xm1 - limit((mTimestep*vm1)/2. - \
delayedPart[4][1],m1PositionLimitNeg,m1PositionLimitPos);
          systemEquations[4] =xm2 - limit((mTimestep*vm2)/2. - \
delayedPart[5][1],m2PositionLimitNeg,m2PositionLimitPos);
          systemEquations[5] =xm3 - limit((mTimestep*vm3)/2. - \
delayedPart[6][1],m3PositionLimitNeg,m3PositionLimitPos);
          systemEquations[6] =-cm1 + fm1 - vm1*Zcm1;
          systemEquations[7] =-cm2 + fm2 - vm2*Zcm2;
          systemEquations[8] =-cm3 + fm3 - vm3*Zcm3;

          //Jacobian matrix
          jacobianMatrix[0][0] = 1 - \
(mTimestep*((-2*bfc*m1Mass*dxLimit((-2*bfc*m1Mass*vm1)/mTimestep,-m1FrictionC\
oulomb,m1FrictionCoulomb))/mTimestep - \
(2*bfc*m1Mass*m2Mass*dxLimit((-2*bfc*m1Mass*m2Mass*(vm1 - vm2))/((m1Mass + \
m2Mass)*mTimestep),-m12FrictionCoulomb,m12FrictionCoulomb))/((m1Mass + \
m2Mass)*mTimestep) - (2*bfc*m1Mass*m3Mass*dxLimit((-2*bfc*m1Mass*m3Mass*(vm1 \
- vm3))/((m1Mass + \
m3Mass)*mTimestep),-m13FrictionCoulomb,m13FrictionCoulomb))/((m1Mass + \
m3Mass)*mTimestep))*dxLimit(limit((mTimestep*vm1)/2. - \
delayedPart[4][1],m1PositionLimitNeg,m1PositionLimitPos),m1PositionLimitNeg,m\
1PositionLimitPos))/(2*m1Mass + (m12FrictionViscousCoeff + \
m13FrictionViscousCoeff + m1FrictionViscousCoeff)*mTimestep);
          jacobianMatrix[0][1] = -((mTimestep*(m12FrictionViscousCoeff + \
(2*bfc*m1Mass*m2Mass*dxLimit((-2*bfc*m1Mass*m2Mass*(vm1 - vm2))/((m1Mass + \
m2Mass)*mTimestep),-m12FrictionCoulomb,m12FrictionCoulomb))/((m1Mass + \
m2Mass)*mTimestep))*dxLimit(limit((mTimestep*vm1)/2. - \
delayedPart[4][1],m1PositionLimitNeg,m1PositionLimitPos),m1PositionLimitNeg,m\
1PositionLimitPos))/(2*m1Mass + (m12FrictionViscousCoeff + \
m13FrictionViscousCoeff + m1FrictionViscousCoeff)*mTimestep));
          jacobianMatrix[0][2] = -((mTimestep*(m13FrictionViscousCoeff + \
(2*bfc*m1Mass*m3Mass*dxLimit((-2*bfc*m1Mass*m3Mass*(vm1 - vm3))/((m1Mass + \
m3Mass)*mTimestep),-m13FrictionCoulomb,m13FrictionCoulomb))/((m1Mass + \
m3Mass)*mTimestep))*dxLimit(limit((mTimestep*vm1)/2. - \
delayedPart[4][1],m1PositionLimitNeg,m1PositionLimitPos),m1PositionLimitNeg,m\
1PositionLimitPos))/(2*m1Mass + (m12FrictionViscousCoeff + \
m13FrictionViscousCoeff + m1FrictionViscousCoeff)*mTimestep));
          jacobianMatrix[0][3] = 0;
          jacobianMatrix[0][4] = 0;
          jacobianMatrix[0][5] = 0;
          jacobianMatrix[0][6] = (mTimestep*dxLimit(limit((mTimestep*vm1)/2. \
- \
delayedPart[4][1],m1PositionLimitNeg,m1PositionLimitPos),m1PositionLimitNeg,m\
1PositionLimitPos))/(2*m1Mass + (m12FrictionViscousCoeff + \
m13FrictionViscousCoeff + m1FrictionViscousCoeff)*mTimestep);
          jacobianMatrix[0][7] = 0;
          jacobianMatrix[0][8] = 0;
          jacobianMatrix[1][0] = (mTimestep*(-m12FrictionViscousCoeff - \
(2*bfc*m1Mass*m2Mass*dxLimit((-2*bfc*m1Mass*m2Mass*(vm1 - vm2))/((m1Mass + \
m2Mass)*mTimestep),-m12FrictionCoulomb,m12FrictionCoulomb))/((m1Mass + \
m2Mass)*mTimestep))*dxLimit(limit((mTimestep*vm2)/2. - \
delayedPart[5][1],m2PositionLimitNeg,m2PositionLimitPos),m2PositionLimitNeg,m\
2PositionLimitPos))/(2*m2Mass + (m12FrictionViscousCoeff + \
m2FrictionViscousCoeff)*mTimestep);
          jacobianMatrix[1][1] = 1 + \
(mTimestep*((2*bfc*m1Mass*m2Mass*dxLimit((-2*bfc*m1Mass*m2Mass*(vm1 - \
vm2))/((m1Mass + \
m2Mass)*mTimestep),-m12FrictionCoulomb,m12FrictionCoulomb))/((m1Mass + \
m2Mass)*mTimestep) + \
(2*bfc*m2Mass*dxLimit((-2*bfc*m2Mass*vm2)/mTimestep,-m2FrictionCoulomb,m2Fric\
tionCoulomb))/mTimestep)*dxLimit(limit((mTimestep*vm2)/2. - \
delayedPart[5][1],m2PositionLimitNeg,m2PositionLimitPos),m2PositionLimitNeg,m\
2PositionLimitPos))/(2*m2Mass + (m12FrictionViscousCoeff + \
m2FrictionViscousCoeff)*mTimestep);
          jacobianMatrix[1][2] = 0;
          jacobianMatrix[1][3] = 0;
          jacobianMatrix[1][4] = 0;
          jacobianMatrix[1][5] = 0;
          jacobianMatrix[1][6] = 0;
          jacobianMatrix[1][7] = (mTimestep*dxLimit(limit((mTimestep*vm2)/2. \
- \
delayedPart[5][1],m2PositionLimitNeg,m2PositionLimitPos),m2PositionLimitNeg,m\
2PositionLimitPos))/(2*m2Mass + (m12FrictionViscousCoeff + \
m2FrictionViscousCoeff)*mTimestep);
          jacobianMatrix[1][8] = 0;
          jacobianMatrix[2][0] = (mTimestep*(-m13FrictionViscousCoeff - \
(2*bfc*m1Mass*m3Mass*dxLimit((-2*bfc*m1Mass*m3Mass*(vm1 - vm3))/((m1Mass + \
m3Mass)*mTimestep),-m13FrictionCoulomb,m13FrictionCoulomb))/((m1Mass + \
m3Mass)*mTimestep))*dxLimit(limit((mTimestep*vm3)/2. - \
delayedPart[6][1],m3PositionLimitNeg,m3PositionLimitPos),m3PositionLimitNeg,m\
3PositionLimitPos))/(2*m3Mass + (m13FrictionViscousCoeff + \
m3FrictionViscousCoeff)*mTimestep);
          jacobianMatrix[2][1] = 0;
          jacobianMatrix[2][2] = 1 + \
(mTimestep*((2*bfc*m1Mass*m3Mass*dxLimit((-2*bfc*m1Mass*m3Mass*(vm1 - \
vm3))/((m1Mass + \
m3Mass)*mTimestep),-m13FrictionCoulomb,m13FrictionCoulomb))/((m1Mass + \
m3Mass)*mTimestep) + \
(2*bfc*m3Mass*dxLimit((-2*bfc*m3Mass*vm3)/mTimestep,-m3FrictionCoulomb,m3Fric\
tionCoulomb))/mTimestep)*dxLimit(limit((mTimestep*vm3)/2. - \
delayedPart[6][1],m3PositionLimitNeg,m3PositionLimitPos),m3PositionLimitNeg,m\
3PositionLimitPos))/(2*m3Mass + (m13FrictionViscousCoeff + \
m3FrictionViscousCoeff)*mTimestep);
          jacobianMatrix[2][3] = 0;
          jacobianMatrix[2][4] = 0;
          jacobianMatrix[2][5] = 0;
          jacobianMatrix[2][6] = 0;
          jacobianMatrix[2][7] = 0;
          jacobianMatrix[2][8] = (mTimestep*dxLimit(limit((mTimestep*vm3)/2. \
- \
delayedPart[6][1],m3PositionLimitNeg,m3PositionLimitPos),m3PositionLimitNeg,m\
3PositionLimitPos))/(2*m3Mass + (m13FrictionViscousCoeff + \
m3FrictionViscousCoeff)*mTimestep);
          jacobianMatrix[3][0] = -(mTimestep*dxLimit((mTimestep*vm1)/2. - \
delayedPart[4][1],m1PositionLimitNeg,m1PositionLimitPos))/2.;
          jacobianMatrix[3][1] = 0;
          jacobianMatrix[3][2] = 0;
          jacobianMatrix[3][3] = 1;
          jacobianMatrix[3][4] = 0;
          jacobianMatrix[3][5] = 0;
          jacobianMatrix[3][6] = 0;
          jacobianMatrix[3][7] = 0;
          jacobianMatrix[3][8] = 0;
          jacobianMatrix[4][0] = 0;
          jacobianMatrix[4][1] = -(mTimestep*dxLimit((mTimestep*vm2)/2. - \
delayedPart[5][1],m2PositionLimitNeg,m2PositionLimitPos))/2.;
          jacobianMatrix[4][2] = 0;
          jacobianMatrix[4][3] = 0;
          jacobianMatrix[4][4] = 1;
          jacobianMatrix[4][5] = 0;
          jacobianMatrix[4][6] = 0;
          jacobianMatrix[4][7] = 0;
          jacobianMatrix[4][8] = 0;
          jacobianMatrix[5][0] = 0;
          jacobianMatrix[5][1] = 0;
          jacobianMatrix[5][2] = -(mTimestep*dxLimit((mTimestep*vm3)/2. - \
delayedPart[6][1],m3PositionLimitNeg,m3PositionLimitPos))/2.;
          jacobianMatrix[5][3] = 0;
          jacobianMatrix[5][4] = 0;
          jacobianMatrix[5][5] = 1;
          jacobianMatrix[5][6] = 0;
          jacobianMatrix[5][7] = 0;
          jacobianMatrix[5][8] = 0;
          jacobianMatrix[6][0] = -Zcm1;
          jacobianMatrix[6][1] = 0;
          jacobianMatrix[6][2] = 0;
          jacobianMatrix[6][3] = 0;
          jacobianMatrix[6][4] = 0;
          jacobianMatrix[6][5] = 0;
          jacobianMatrix[6][6] = 1;
          jacobianMatrix[6][7] = 0;
          jacobianMatrix[6][8] = 0;
          jacobianMatrix[7][0] = 0;
          jacobianMatrix[7][1] = -Zcm2;
          jacobianMatrix[7][2] = 0;
          jacobianMatrix[7][3] = 0;
          jacobianMatrix[7][4] = 0;
          jacobianMatrix[7][5] = 0;
          jacobianMatrix[7][6] = 0;
          jacobianMatrix[7][7] = 1;
          jacobianMatrix[7][8] = 0;
          jacobianMatrix[8][0] = 0;
          jacobianMatrix[8][1] = 0;
          jacobianMatrix[8][2] = -Zcm3;
          jacobianMatrix[8][3] = 0;
          jacobianMatrix[8][4] = 0;
          jacobianMatrix[8][5] = 0;
          jacobianMatrix[8][6] = 0;
          jacobianMatrix[8][7] = 0;
          jacobianMatrix[8][8] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          vm1=stateVark[0];
          vm2=stateVark[1];
          vm3=stateVark[2];
          xm1=stateVark[3];
          xm2=stateVark[4];
          xm3=stateVark[5];
          fm1=stateVark[6];
          fm2=stateVark[7];
          fm3=stateVark[8];
          //Expressions
          m1Position = m1PositionOffset - m1Direction*xm1;
          m2Position = m2PositionOffset - m2Direction*xm2;
          m3Position = m3PositionOffset - m3Direction*xm3;
          m1Velocity = -(m1Direction*vm1);
          m2Velocity = -(m2Direction*vm2);
          m3Velocity = -(m3Direction*vm3);
          m1FrictionForce = m1Direction*(m1FrictionViscousCoeff*vm1 - \
limit((-2*bfc*m1Mass*vm1)/mTimestep,-m1FrictionCoulomb,m1FrictionCoulomb));
          m2FrictionForce = m2Direction*(m2FrictionViscousCoeff*vm2 - \
limit((-2*bfc*m2Mass*vm2)/mTimestep,-m2FrictionCoulomb,m2FrictionCoulomb));
          m3FrictionForce = m3Direction*(m3FrictionViscousCoeff*vm3 - \
limit((-2*bfc*m3Mass*vm3)/mTimestep,-m3FrictionCoulomb,m3FrictionCoulomb));
          m12FrictionForce = -(m12FrictionViscousCoeff*(vm1 - vm2)) + \
limit((-2*bfc*m1Mass*m2Mass*(vm1 - vm2))/((m1Mass + \
m2Mass)*mTimestep),-m12FrictionCoulomb,m12FrictionCoulomb);
          m13FrictionForce = -(m13FrictionViscousCoeff*(vm1 - vm3)) + \
limit((-2*bfc*m1Mass*m3Mass*(vm1 - vm3))/((m1Mass + \
m3Mass)*mTimestep),-m13FrictionCoulomb,m13FrictionCoulomb);
          eqMassm1 = m1Mass;
          eqMassm2 = m2Mass;
          eqMassm3 = m3Mass;
        }

        //Calculate the delayed parts
        delayParts1[1] = (fm1*mTimestep - 2*m1Mass*vm1 + \
m12FrictionViscousCoeff*mTimestep*vm1 + m13FrictionViscousCoeff*mTimestep*vm1 \
+ m1FrictionViscousCoeff*mTimestep*vm1 - \
m12FrictionViscousCoeff*mTimestep*vm2 - m13FrictionViscousCoeff*mTimestep*vm3 \
- \
mTimestep*limit((-2*bfc*m1Mass*vm1)/mTimestep,-m1FrictionCoulomb,m1FrictionCo\
ulomb) - mTimestep*limit((-2*bfc*m1Mass*m2Mass*(vm1 - vm2))/((m1Mass + \
m2Mass)*mTimestep),-m12FrictionCoulomb,m12FrictionCoulomb) - \
mTimestep*limit((-2*bfc*m1Mass*m3Mass*(vm1 - vm3))/((m1Mass + \
m3Mass)*mTimestep),-m13FrictionCoulomb,m13FrictionCoulomb))/(2*m1Mass + \
m12FrictionViscousCoeff*mTimestep + m13FrictionViscousCoeff*mTimestep + \
m1FrictionViscousCoeff*mTimestep);
        delayParts2[1] = (fm2*mTimestep - \
m12FrictionViscousCoeff*mTimestep*vm1 - 2*m2Mass*vm2 + \
m12FrictionViscousCoeff*mTimestep*vm2 + m2FrictionViscousCoeff*mTimestep*vm2 \
+ mTimestep*limit((-2*bfc*m1Mass*m2Mass*(vm1 - vm2))/((m1Mass + \
m2Mass)*mTimestep),-m12FrictionCoulomb,m12FrictionCoulomb) - \
mTimestep*limit((-2*bfc*m2Mass*vm2)/mTimestep,-m2FrictionCoulomb,m2FrictionCo\
ulomb))/(2*m2Mass + m12FrictionViscousCoeff*mTimestep + \
m2FrictionViscousCoeff*mTimestep);
        delayParts3[1] = (fm3*mTimestep - \
m13FrictionViscousCoeff*mTimestep*vm1 - 2*m3Mass*vm3 + \
m13FrictionViscousCoeff*mTimestep*vm3 + m3FrictionViscousCoeff*mTimestep*vm3 \
+ mTimestep*limit((-2*bfc*m1Mass*m3Mass*(vm1 - vm3))/((m1Mass + \
m3Mass)*mTimestep),-m13FrictionCoulomb,m13FrictionCoulomb) - \
mTimestep*limit((-2*bfc*m3Mass*vm3)/mTimestep,-m3FrictionCoulomb,m3FrictionCo\
ulomb))/(2*m3Mass + m13FrictionViscousCoeff*mTimestep + \
m3FrictionViscousCoeff*mTimestep);
        delayParts4[1] = (-(mTimestep*vm1) - 2*xm1)/2.;
        delayParts5[1] = (-(mTimestep*vm2) - 2*xm2)/2.;
        delayParts6[1] = (-(mTimestep*vm3) - 2*xm3)/2.;

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[5][1] = delayParts5[1];
        delayedPart[6][1] = delayParts6[1];
        delayedPart[7][1] = delayParts7[1];
        delayedPart[8][1] = delayParts8[1];
        delayedPart[9][1] = delayParts9[1];

        //Write new values to nodes
        //Port Pm1
        (*mpND_fm1)=fm1;
        (*mpND_xm1)=xm1;
        (*mpND_vm1)=vm1;
        (*mpND_eqMassm1)=eqMassm1;
        //Port Pm2
        (*mpND_fm2)=fm2;
        (*mpND_xm2)=xm2;
        (*mpND_vm2)=vm2;
        (*mpND_eqMassm2)=eqMassm2;
        //Port Pm3
        (*mpND_fm3)=fm3;
        (*mpND_xm3)=xm3;
        (*mpND_vm3)=vm3;
        (*mpND_eqMassm3)=eqMassm3;
        //outputVariables
        (*mpm1Position)=m1Position;
        (*mpm2Position)=m2Position;
        (*mpm3Position)=m3Position;
        (*mpm1Velocity)=m1Velocity;
        (*mpm2Velocity)=m2Velocity;
        (*mpm3Velocity)=m3Velocity;
        (*mpm1FrictionForce)=m1FrictionForce;
        (*mpm2FrictionForce)=m2FrictionForce;
        (*mpm3FrictionForce)=m3FrictionForce;
        (*mpm12FrictionForce)=m12FrictionForce;
        (*mpm13FrictionForce)=m13FrictionForce;

        //Update the delayed variabels
        mDelayedPart11.update(delayParts1[1]);
        mDelayedPart21.update(delayParts2[1]);
        mDelayedPart31.update(delayParts3[1]);
        mDelayedPart41.update(delayParts4[1]);
        mDelayedPart51.update(delayParts5[1]);
        mDelayedPart61.update(delayParts6[1]);

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // MECHANICM3LOAD1D_HPP_INCLUDED
