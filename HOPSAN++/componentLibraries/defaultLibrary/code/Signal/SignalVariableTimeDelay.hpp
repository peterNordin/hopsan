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
//! @file   SignalVariableTimeDelay.hpppp
//! @author Peter Nordin <peter.nordin@liu.se>
//! @date   2013-02-22
//!
//! @brief Contains a Signal Time Delay Component
//!
//$Id: SignalTimeDelay.hpp 4915 2013-01-18 12:15:46Z bjoer $

#ifndef SIGNALVARIABLETIMEDELAY_HPP_INCLUDED
#define SIGNALVARIABLETIMEDELAY_HPP_INCLUDED

#include "ComponentEssentials.h"
#include "ComponentUtilities.h"

namespace hopsan {

    //!
    //! @brief
    //! @ingroup SignalComponents
    //!
    class SignalVariableTimeDelay : public ComponentSignal
    {

    private:
        double mTimeDelay;
        Delay *mpDelay;
        double *mpND_in, *mpND_out;
        Port *mpIn, *mpOut;

    public:
        static Component *Creator()
        {
            return new SignalVariableTimeDelay();
        }

        void configure()
        {
            mTimeDelay = 1.0;
            mpDelay = 0;

            registerParameter("dT", "Time delay", "[s]", mTimeDelay);

            mpIn = addReadPort("in", "NodeSignal");
            mpOut = addWritePort("out", "NodeSignal", Port::NOTREQUIRED);
        }


        void initialize()
        {
            mpND_in = getSafeNodeDataPtr(mpIn, NodeSignal::VALUE);
            mpND_out = getSafeNodeDataPtr(mpOut, NodeSignal::VALUE);

            if (mTimeDelay < 0)
            {
                addErrorMessage("Can not have timedelay < 0 s");
                stopSimulation();
                return;
            }

            mpDelay = new Delay;
            mpDelay->initialize(mTimeDelay, mTimestep, (*mpND_in));
            (*mpND_out) = (*mpND_in);
        }


        void simulateOneTimestep()
        {
            // Check if delay have changed, useing int truncation and + 0.5  to round to nearest int
            const size_t nSamps = int(mTimeDelay/mTimestep+0.5);

            if ( nSamps != mpDelay->getSize())
            {
                // Create a new empty delay
                Delay *pNewDelay = new Delay();

                // Check if it should be populated
                if (nSamps != 0)
                {
                    pNewDelay->initialize(nSamps,0);

                    // Copy old data, depending on longer or shorter delay
                    if (nSamps < mpDelay->getSize())
                    {
                        // Keep the oldest values truncate teh newest ones
                        for (size_t i=0; i<nSamps; ++i)
                        {
                            pNewDelay->update(mpDelay->getOldIdx(i));
                        }
                    }
                    else
                    {
                        // Keep all old values, fill with the newest value
                        for (size_t i=0; i<mpDelay->getSize(); ++i)
                        {
                            pNewDelay->update(mpDelay->getOldIdx(i));
                        }
                        for (size_t i=mpDelay->getSize(); i<nSamps; ++i)
                        {
                            if (mpDelay->getSize() > 0)
                            {
                                pNewDelay->update(mpDelay->getNewest());
                            }
                            else
                            {
                                pNewDelay->update(*mpND_in);
                            }
                        }
                    }
                }

                delete mpDelay;
                mpDelay = pNewDelay;
            }

            // If delay is populated, use it , else not
            if (mpDelay->getSize() == 0)
            {
                *mpND_out = *mpND_in;
            }
            else
            {
                (*mpND_out) =  mpDelay->update(*mpND_in);
            }
        }

        void finalize()
        {
            if (mpDelay)
            {
                delete mpDelay;
            }
        }
    };
}

#endif
