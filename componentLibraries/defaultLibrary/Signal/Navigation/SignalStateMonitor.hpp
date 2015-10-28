/*-----------------------------------------------------------------------------
 This source file is a part of Hopsan

 Copyright (c) 2009 to present year, Hopsan Group

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

 For license details and information about the Hopsan Group see the files
 GPLv3 and HOPSANGROUP in the Hopsan source code root directory

 For author and contributor information see the AUTHORS file
-----------------------------------------------------------------------------*/

#ifndef SIGNALSTATEMONITOR_HPP_INCLUDED
#define SIGNALSTATEMONITOR_HPP_INCLUDED

#include <iostream>
#include "ComponentEssentials.h"
#include "ComponentUtilities.h"
#include "math.h"

//!
//! @file SignalStateMonitor.hpp
//! @author Petter Krus <petter.krus@liu.se>
//  co-author/auditor **Not yet audited by a second person**
//! @date Tue 14 Apr 2015 16:48:38
//! @brief Check for steady state
//! @ingroup SignalComponents
//!
//==This code has been autogenerated using Compgen==
//from 
/*{, C:, HopsanTrunk, componentLibraries, defaultLibrary, Signal, \
Control}/SignalControlAero.nb*/

using namespace hopsan;

class SignalStateMonitor : public ComponentSignal
{
private:
     double y10;
     double y20;
     double y30;
     double thau;
     double delayParts1[9];
     double delayParts2[9];
     double delayParts3[9];
     double delayParts4[9];
     Matrix jacobianMatrix;
     Vec systemEquations;
     Matrix delayedPart;
     int i;
     int iter;
     int mNoiter;
     double jsyseqnweight[4];
     int order[4];
     int mNstep;
//==This code has been autogenerated using Compgen==
     //inputVariables
     double y1;
     double y2;
     double y3;
     double sOn;
     //outputVariables
     double s1;
     double y1f;
     double y2f;
     double y3f;
     //Delay declarations
//==This code has been autogenerated using Compgen==
     //inputVariables pointers
     double *mpy1;
     double *mpy2;
     double *mpy3;
     double *mpsOn;
     //inputParameters pointers
     double *mpy10;
     double *mpy20;
     double *mpy30;
     double *mpthau;
     //outputVariables pointers
     double *mps1;
     double *mpy1f;
     double *mpy2f;
     double *mpy3f;
     Delay mDelayedPart10;
     Delay mDelayedPart11;
     Delay mDelayedPart20;
     Delay mDelayedPart21;
     Delay mDelayedPart30;
     Delay mDelayedPart31;
     Delay mDelayedPart40;
     EquationSystemSolver *mpSolver;

public:
     static Component *Creator()
     {
        return new SignalStateMonitor();
     }

     void configure()
     {
//==This code has been autogenerated using Compgen==

        mNstep=9;
        jacobianMatrix.create(4,4);
        systemEquations.create(4);
        delayedPart.create(5,6);
        mNoiter=2;
        jsyseqnweight[0]=1;
        jsyseqnweight[1]=0.67;
        jsyseqnweight[2]=0.5;
        jsyseqnweight[3]=0.5;


        //Add ports to the component
        //Add inputVariables to the component
            addInputVariable("y1","monitored variable 1","",0.,&mpy1);
            addInputVariable("y2","monitored variable 2","",0.,&mpy2);
            addInputVariable("y3","monitored variable 3","",0.,&mpy3);
            addInputVariable("sOn","extra trigg signal","",0.,&mpsOn);

        //Add inputParammeters to the component
            addInputVariable("y10", "treashold variable 1", "", 0.02,&mpy10);
            addInputVariable("y20", "treashold variable 2", "", 0.02,&mpy20);
            addInputVariable("y30", "treashold variable 3", "", 0.02,&mpy30);
            addInputVariable("thau", "filter time constant 2", "sec", \
1.,&mpthau);
        //Add outputVariables to the component
            addOutputVariable("s1","One when varaibles in steady \
state","",0.,&mps1);
            addOutputVariable("y1f","filtered variable 1","",0.,&mpy1f);
            addOutputVariable("y2f","filtered variable 2","",0.,&mpy2f);
            addOutputVariable("y3f","filtered variable 3","",0.,&mpy3f);

//==This code has been autogenerated using Compgen==
        //Add constantParameters
        mpSolver = new EquationSystemSolver(this,4);
     }

    void initialize()
     {
        //Read port variable pointers from nodes

        //Read variables from nodes

        //Read inputVariables from nodes
        y1 = (*mpy1);
        y2 = (*mpy2);
        y3 = (*mpy3);
        sOn = (*mpsOn);

        //Read inputParameters from nodes
        y10 = (*mpy10);
        y20 = (*mpy20);
        y30 = (*mpy30);
        thau = (*mpthau);

        //Read outputVariables from nodes
        s1 = (*mps1);
        y1f = (*mpy1f);
        y2f = (*mpy2f);
        y3f = (*mpy3f);

//==This code has been autogenerated using Compgen==


        //Initialize delays
        delayParts1[1] = (mTimestep*y1f - 2*thau*y1f - \
mTimestep*Abs(y1))/(mTimestep + 2*thau);
        mDelayedPart11.initialize(mNstep,delayParts1[1]);
        delayParts2[1] = (mTimestep*y2f - 2*thau*y2f - \
mTimestep*Abs(y2))/(mTimestep + 2*thau);
        mDelayedPart21.initialize(mNstep,delayParts2[1]);
        delayParts3[1] = (mTimestep*y3f - 2*thau*y3f - \
mTimestep*Abs(y3))/(mTimestep + 2*thau);
        mDelayedPart31.initialize(mNstep,delayParts3[1]);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];

        simulateOneTimestep();

     }
    void simulateOneTimestep()
     {
        Vec stateVar(4);
        Vec stateVark(4);
        Vec deltaStateVar(4);

        //Read variables from nodes

        //Read inputVariables from nodes
        y1 = (*mpy1);
        y2 = (*mpy2);
        y3 = (*mpy3);
        sOn = (*mpsOn);

        //Read inputParameters from nodes
        y10 = (*mpy10);
        y20 = (*mpy20);
        y30 = (*mpy30);
        thau = (*mpthau);

        //LocalExpressions

        //Initializing variable vector for Newton-Raphson
        stateVark[0] = y1f;
        stateVark[1] = y2f;
        stateVark[2] = y3f;
        stateVark[3] = s1;

        //Iterative solution using Newton-Rapshson
        for(iter=1;iter<=mNoiter;iter++)
        {
         //StateMonitor
         //Differential-algebraic system of equation parts

          //Assemble differential-algebraic equations
          systemEquations[0] =y1f - (mTimestep*Abs(y1))/(mTimestep + 2*thau) \
+ delayedPart[1][1];
          systemEquations[1] =y2f - (mTimestep*Abs(y2))/(mTimestep + 2*thau) \
+ delayedPart[2][1];
          systemEquations[2] =y3f - (mTimestep*Abs(y3))/(mTimestep + 2*thau) \
+ delayedPart[3][1];
          systemEquations[3] =s1 - onPositive(-0.5 + sOn + \
onPositive(-Abs(y10) + Abs(y1f)) + onPositive(-Abs(y20) + Abs(y2f)) + \
onPositive(-Abs(y30) + Abs(y3f)));

          //Jacobian matrix
          jacobianMatrix[0][0] = 1;
          jacobianMatrix[0][1] = 0;
          jacobianMatrix[0][2] = 0;
          jacobianMatrix[0][3] = 0;
          jacobianMatrix[1][0] = 0;
          jacobianMatrix[1][1] = 1;
          jacobianMatrix[1][2] = 0;
          jacobianMatrix[1][3] = 0;
          jacobianMatrix[2][0] = 0;
          jacobianMatrix[2][1] = 0;
          jacobianMatrix[2][2] = 1;
          jacobianMatrix[2][3] = 0;
          jacobianMatrix[3][0] = 0;
          jacobianMatrix[3][1] = 0;
          jacobianMatrix[3][2] = 0;
          jacobianMatrix[3][3] = 1;
//==This code has been autogenerated using Compgen==

          //Solving equation using LU-faktorisation
          mpSolver->solve(jacobianMatrix, systemEquations, stateVark, iter);
          y1f=stateVark[0];
          y2f=stateVark[1];
          y3f=stateVark[2];
          s1=stateVark[3];
        }

        //Calculate the delayed parts
        delayParts1[1] = (mTimestep*y1f - 2*thau*y1f - \
mTimestep*Abs(y1))/(mTimestep + 2*thau);
        delayParts2[1] = (mTimestep*y2f - 2*thau*y2f - \
mTimestep*Abs(y2))/(mTimestep + 2*thau);
        delayParts3[1] = (mTimestep*y3f - 2*thau*y3f - \
mTimestep*Abs(y3))/(mTimestep + 2*thau);

        delayedPart[1][1] = delayParts1[1];
        delayedPart[2][1] = delayParts2[1];
        delayedPart[3][1] = delayParts3[1];
        delayedPart[4][1] = delayParts4[1];

        //Write new values to nodes
        //outputVariables
        (*mps1)=s1;
        (*mpy1f)=y1f;
        (*mpy2f)=y2f;
        (*mpy3f)=y3f;

        //Update the delayed variabels
        mDelayedPart11.update(delayParts1[1]);
        mDelayedPart21.update(delayParts2[1]);
        mDelayedPart31.update(delayParts3[1]);

     }
    void deconfigure()
    {
        delete mpSolver;
    }
};
#endif // SIGNALSTATEMONITOR_HPP_INCLUDED
