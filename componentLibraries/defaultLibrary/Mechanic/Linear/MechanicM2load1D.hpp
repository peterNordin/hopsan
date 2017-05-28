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

#ifndef MECHANICM2LOAD1D_HPP_INCLUDED
#define MECHANICM2LOAD1D_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file MechanicM2load1D.hpp
//! @author Petter Krus <petter.krus@liu.se>
//  co-author/auditor **Not yet audited by a second person**
//! @date Mon 22 Jun 2015 11:48:29
//! @brief An inertia load with spring and damper
//! @ingroup MechanicComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, componentLibraries, defaultLibrary, Mechanic, \
Linear}/MechanicM2load1D.nb*/

using namespace hopsan;

class MechanicM2load1D : public ComponentQ
{
private:
     double m1;
     double m2;
     double bL;
     double fc;
     double bfc;
     double xpmin;
     double xpmax;
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
     //outputVariables
     double vt;
     double xt;
     //InitialExpressions variables
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
     //inputParameters pointers
     double *mpm1;
     double *mpm2;
     double *mpbL;
     double *mpfc;
     double *mpbfc;
     double *mpxpmin;
     double *mpxpmax;
     //outputVariables pointers
     double *mpvt;
     double *mpxt;
     Delay mDelayedPart10;
     Delay mDelayedPart20;
     Delay mDelayedPart30;
     Delay mDelayedPart31;
     Delay mDelayedPart40;
     Delay mDelayedPart41;
     Delay mDelayedPart42;
     Delay mDelayedPart50;
     Delay mDelayedPart51;
     Delay mDelayedPart60;
     Delay mDelayedPart61;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new MechanicM2load1D();
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

        //Add inputParammeters to the component
            addInputVariable("m1", "Inertia1", "kg", 1000.,&mpm1);
            addInputVariable("m2", "Inertia2", "kg", 1000.,&mpm2);
            addInputVariable("bL", "Visc. friction coeff.", "Ns/m", \
10.,&mpbL);
            addInputVariable("fc", "Dry friction (+/-)", "N", 10.,&mpfc);
            addInputVariable("bfc", "Numerical friction factor.", "", \
1.,&mpbfc);
            addInputVariable("xpmin", "Limitation on stroke xp (-x3)", "m", \
0.,&mpxpmin);
            addInputVariable("xpmax", "Limitation on stroke xp (-x3)", "m", \
1.,&mpxpmax);
        //Add outputVariables to the component
            addOutputVariable("vt","cg speed","m/s",0.,&mpvt);
            addOutputVariable("xt","cg position","m",0.,&mpxt);

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

        //Read inputParameters from nodes
        m1 = (*mpm1);
        m2 = (*mpm2);
        bL = (*mpbL);
        fc = (*mpfc);
        bfc = (*mpbfc);
        xpmin = (*mpxpmin);
        xpmax = (*mpxpmax);

        //Read outputVariables from nodes
        vt = (*mpvt);
        xt = (*mpxt);

//==This code has been autogenerated using Compgen==
        //InitialExpressions
        xm1 = -xm2;


        //Initialize delays
        delayParts3[1] = (-(fm2*m1*mTimestep) + fm3*m1*mTimestep - \
fm1*m2*mTimestep + fm3*m2*mTimestep - 2*m1*m2*vm3 + \
m1*mTimestep*limit((2*bfc*m1*m2*vm3)/((m1 + m2)*mTimestep),-fc,fc) + \
m2*mTimestep*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc))/(2.*m1*m2);
        mDelayedPart31.initialize(mNstep,delayParts3[1]);
        delayParts4[1] = (-2*fm2*m1*Power(mTimestep,2) + \
2*fm3*m1*Power(mTimestep,2) - 2*fm1*m2*Power(mTimestep,2) + \
2*fm3*m2*Power(mTimestep,2) - 8*m1*m2*xm3 + \
2*m1*Power(mTimestep,2)*limit((2*bfc*m1*m2*vm3)/((m1 + m2)*mTimestep),-fc,fc) \
+ 2*m2*Power(mTimestep,2)*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc))/(4.*m1*m2);
        mDelayedPart41.initialize(mNstep,delayParts4[1]);
        delayParts4[2] = (-(fm2*m1*Power(mTimestep,2)) + \
fm3*m1*Power(mTimestep,2) - fm1*m2*Power(mTimestep,2) + \
fm3*m2*Power(mTimestep,2) + 4*m1*m2*xm3 + \
m1*Power(mTimestep,2)*limit((2*bfc*m1*m2*vm3)/((m1 + m2)*mTimestep),-fc,fc) + \
m2*Power(mTimestep,2)*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc))/(4.*m1*m2);
        mDelayedPart42.initialize(mNstep,delayParts4[2]);
        delayParts5[1] = (-(fm1*mTimestep) + fm2*mTimestep - 2*m1*vt - \
2*m2*vt)/(2*m1 + 2*m2);
        mDelayedPart51.initialize(mNstep,delayParts5[1]);
        delayParts6[1] = (-(mTimestep*vt) - 2*xt)/2.;
        mDelayedPart61.initialize(mNstep,delayParts6[1]);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[4][2] = mDelayedPart42.getIdx(1);
        delayedPart[5][1] = delayParts5[1];
        delayedPart[6][1] = delayParts6[1];
        delayedPart[7][1] = delayParts7[1];
        delayedPart[8][1] = delayParts8[1];
        delayedPart[9][1] = delayParts9[1];

        simulateOneTimestep();

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

        //Read inputParameters from nodes
        m1 = (*mpm1);
        m2 = (*mpm2);
        bL = (*mpbL);
        fc = (*mpfc);
        bfc = (*mpbfc);
        xpmin = (*mpxpmin);
        xpmax = (*mpxpmax);

        //LocalExpressions

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = vm1;
        stateVark[1] = vm2;
        stateVark[2] = vm3;
        stateVark[3] = xm3;
        stateVark[4] = vt;
        stateVark[5] = xt;
        stateVark[6] = fm1;
        stateVark[7] = fm2;
        stateVark[8] = fm3;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //M2load1D
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =vm1 + (m1*vt + m2*(vm3 + vt))/(m1 + m2);
          systemEquations[1] =vm2 + (m1*vm3 - m1*vt - m2*vt)/(m1 + m2);
          systemEquations[2] =vm3 - \
dxLimit(limit(-(Power(mTimestep,2)*(-(fm2*m1) + fm3*m1 - fm1*m2 + fm3*m2 + \
(m1 + m2)*limit((2*bfc*m1*m2*vm3)/((m1 + m2)*mTimestep),-fc,fc)))/(4.*m1*m2) \
- delayedPart[4][1] - \
delayedPart[4][2],-xpmax,-xpmin),-xpmax,-xpmin)*(-(mTimestep*(-(fm2*m1) + \
fm3*m1 - fm1*m2 + fm3*m2 + (m1 + m2)*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc)))/(2.*m1*m2) - delayedPart[3][1]);
          systemEquations[3] =xm3 - limit(-(Power(mTimestep,2)*(-(fm2*m1) + \
fm3*m1 - fm1*m2 + fm3*m2 + (m1 + m2)*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc)))/(4.*m1*m2) - delayedPart[4][1] - \
delayedPart[4][2],-xpmax,-xpmin);
          systemEquations[4] =((-fm1 + fm2)*mTimestep)/(2.*(m1 + m2)) + vt + \
delayedPart[5][1];
          systemEquations[5] =-(mTimestep*vt)/2. + xt + delayedPart[6][1];
          systemEquations[6] =-cm1 + fm1 + vm2*Zcm1;
          systemEquations[7] =-cm2 + fm2 - vm2*Zcm2;
          systemEquations[8] =-cm3 + fm3 - vm3*Zcm3;

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = 0;
          jacobianMatrix[0][2] = m2/(m1 + m2);
          jacobianMatrix[0][3] = 0;
          jacobianMatrix[0][4] = 1;
          jacobianMatrix[0][5] = 0;
          jacobianMatrix[0][6] = 0;
          jacobianMatrix[0][7] = 0;
          jacobianMatrix[0][8] = 0;
          jacobianMatrix[1][0] = 0;
          jacobianMatrix[1][1] = 1;
          jacobianMatrix[1][2] = m1/(m1 + m2);
          jacobianMatrix[1][3] = 0;
          jacobianMatrix[1][4] = (-m1 - m2)/(m1 + m2);
          jacobianMatrix[1][5] = 0;
          jacobianMatrix[1][6] = 0;
          jacobianMatrix[1][7] = 0;
          jacobianMatrix[1][8] = 0;
          jacobianMatrix[2][0] = 0;
          jacobianMatrix[2][1] = 0;
          jacobianMatrix[2][2] = 1 + bfc*dxLimit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc)*dxLimit(limit(-(Power(mTimestep,2)*(-(fm2*m1) + fm3*m1 \
- fm1*m2 + fm3*m2 + (m1 + m2)*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc)))/(4.*m1*m2) - delayedPart[4][1] - \
delayedPart[4][2],-xpmax,-xpmin),-xpmax,-xpmin);
          jacobianMatrix[2][3] = 0;
          jacobianMatrix[2][4] = 0;
          jacobianMatrix[2][5] = 0;
          jacobianMatrix[2][6] = \
-(mTimestep*dxLimit(limit(-(Power(mTimestep,2)*(-(fm2*m1) + fm3*m1 - fm1*m2 + \
fm3*m2 + (m1 + m2)*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc)))/(4.*m1*m2) - delayedPart[4][1] - \
delayedPart[4][2],-xpmax,-xpmin),-xpmax,-xpmin))/(2.*m1);
          jacobianMatrix[2][7] = \
-(mTimestep*dxLimit(limit(-(Power(mTimestep,2)*(-(fm2*m1) + fm3*m1 - fm1*m2 + \
fm3*m2 + (m1 + m2)*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc)))/(4.*m1*m2) - delayedPart[4][1] - \
delayedPart[4][2],-xpmax,-xpmin),-xpmax,-xpmin))/(2.*m2);
          jacobianMatrix[2][8] = ((m1 + \
m2)*mTimestep*dxLimit(limit(-(Power(mTimestep,2)*(-(fm2*m1) + fm3*m1 - fm1*m2 \
+ fm3*m2 + (m1 + m2)*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc)))/(4.*m1*m2) - delayedPart[4][1] - \
delayedPart[4][2],-xpmax,-xpmin),-xpmax,-xpmin))/(2.*m1*m2);
          jacobianMatrix[3][0] = 0;
          jacobianMatrix[3][1] = 0;
          jacobianMatrix[3][2] = \
(bfc*mTimestep*dxLimit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc)*dxLimit(-(Power(mTimestep,2)*(-(fm2*m1) + fm3*m1 - \
fm1*m2 + fm3*m2 + (m1 + m2)*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc)))/(4.*m1*m2) - delayedPart[4][1] - \
delayedPart[4][2],-xpmax,-xpmin))/2.;
          jacobianMatrix[3][3] = 1;
          jacobianMatrix[3][4] = 0;
          jacobianMatrix[3][5] = 0;
          jacobianMatrix[3][6] = \
-(Power(mTimestep,2)*dxLimit(-(Power(mTimestep,2)*(-(fm2*m1) + fm3*m1 - \
fm1*m2 + fm3*m2 + (m1 + m2)*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc)))/(4.*m1*m2) - delayedPart[4][1] - \
delayedPart[4][2],-xpmax,-xpmin))/(4.*m1);
          jacobianMatrix[3][7] = \
-(Power(mTimestep,2)*dxLimit(-(Power(mTimestep,2)*(-(fm2*m1) + fm3*m1 - \
fm1*m2 + fm3*m2 + (m1 + m2)*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc)))/(4.*m1*m2) - delayedPart[4][1] - \
delayedPart[4][2],-xpmax,-xpmin))/(4.*m2);
          jacobianMatrix[3][8] = ((m1 + \
m2)*Power(mTimestep,2)*dxLimit(-(Power(mTimestep,2)*(-(fm2*m1) + fm3*m1 - \
fm1*m2 + fm3*m2 + (m1 + m2)*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc)))/(4.*m1*m2) - delayedPart[4][1] - \
delayedPart[4][2],-xpmax,-xpmin))/(4.*m1*m2);
          jacobianMatrix[4][0] = 0;
          jacobianMatrix[4][1] = 0;
          jacobianMatrix[4][2] = 0;
          jacobianMatrix[4][3] = 0;
          jacobianMatrix[4][4] = 1;
          jacobianMatrix[4][5] = 0;
          jacobianMatrix[4][6] = -mTimestep/(2.*(m1 + m2));
          jacobianMatrix[4][7] = mTimestep/(2.*(m1 + m2));
          jacobianMatrix[4][8] = 0;
          jacobianMatrix[5][0] = 0;
          jacobianMatrix[5][1] = 0;
          jacobianMatrix[5][2] = 0;
          jacobianMatrix[5][3] = 0;
          jacobianMatrix[5][4] = -mTimestep/2.;
          jacobianMatrix[5][5] = 1;
          jacobianMatrix[5][6] = 0;
          jacobianMatrix[5][7] = 0;
          jacobianMatrix[5][8] = 0;
          jacobianMatrix[6][0] = 0;
          jacobianMatrix[6][1] = Zcm1;
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
          xm3=stateVark[3];
          vt=stateVark[4];
          xt=stateVark[5];
          fm1=stateVark[6];
          fm2=stateVark[7];
          fm3=stateVark[8];
          //Expressions
          xm1 = -((m2*xm3 + (m1 + m2)*xt)/(m1 + m2));
          xm2 = -((m1*xm3 - (m1 + m2)*xt)/(m1 + m2));
          eqMassm1 = m1;
          eqMassm2 = m2;
        }

        //Calculate the delayed parts
        delayParts3[1] = (-(fm2*m1*mTimestep) + fm3*m1*mTimestep - \
fm1*m2*mTimestep + fm3*m2*mTimestep - 2*m1*m2*vm3 + \
m1*mTimestep*limit((2*bfc*m1*m2*vm3)/((m1 + m2)*mTimestep),-fc,fc) + \
m2*mTimestep*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc))/(2.*m1*m2);
        delayParts4[1] = (-2*fm2*m1*Power(mTimestep,2) + \
2*fm3*m1*Power(mTimestep,2) - 2*fm1*m2*Power(mTimestep,2) + \
2*fm3*m2*Power(mTimestep,2) - 8*m1*m2*xm3 + \
2*m1*Power(mTimestep,2)*limit((2*bfc*m1*m2*vm3)/((m1 + m2)*mTimestep),-fc,fc) \
+ 2*m2*Power(mTimestep,2)*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc))/(4.*m1*m2);
        delayParts4[2] = (-(fm2*m1*Power(mTimestep,2)) + \
fm3*m1*Power(mTimestep,2) - fm1*m2*Power(mTimestep,2) + \
fm3*m2*Power(mTimestep,2) + 4*m1*m2*xm3 + \
m1*Power(mTimestep,2)*limit((2*bfc*m1*m2*vm3)/((m1 + m2)*mTimestep),-fc,fc) + \
m2*Power(mTimestep,2)*limit((2*bfc*m1*m2*vm3)/((m1 + \
m2)*mTimestep),-fc,fc))/(4.*m1*m2);
        delayParts5[1] = (-(fm1*mTimestep) + fm2*mTimestep - 2*m1*vt - \
2*m2*vt)/(2*m1 + 2*m2);
        delayParts6[1] = (-(mTimestep*vt) - 2*xt)/2.;

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];
        delayedPart[4][2] = mDelayedPart42.getIdx(0);
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
        (*mpvt)=vt;
        (*mpxt)=xt;

        //Update the delayed variabels
        mDelayedPart31.update(delayParts3[1]);
        mDelayedPart41.update(delayParts4[1]);
        mDelayedPart42.update(delayParts4[2]);
        mDelayedPart51.update(delayParts5[1]);
        mDelayedPart61.update(delayParts6[1]);

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // MECHANICM2LOAD1D_HPP_INCLUDED
