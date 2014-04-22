//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __CORE4INET_CTAPPLICATIONBASE_H_
#define __CORE4INET_CTAPPLICATIONBASE_H_

#include <omnetpp.h>
#include "base/ApplicationBase.h"

namespace CoRE4INET {

/**
 * @brief Base class for a real-time Ethernet-Application.
 *
 * Containes the mapping to Buffers and the ability to execute Callbacks
 *
 * @sa Buffer, Callback
 *
 * @ingroup Applications
 *
 * @author Till Steinbach
 */
class CTApplicationBase : public virtual ApplicationBase
{
    public:
        /**
         * @brief resets the bag on incoming RC-Frames (on RCin)
         *
         * This method should be called from subclasses unless the module
         * resets the bag on its own.
         *
         * @param msg Parameter must be forwarded from subclass
         */
        virtual void handleMessage(cMessage *msg);

};

} //namespace

#endif
