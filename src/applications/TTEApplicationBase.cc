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

#include "TTEApplicationBase.h"

namespace TTEthernetModel {

Define_Module(TTEApplicationBase);

void TTEApplicationBase::initialize()
{
    // TODO - Generated method body
}

void TTEApplicationBase::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}

/*void TTEApplicationBase::addIncoming(uint16 ctID, Incoming *incoming)
{
    incomings[ctID].push_back(incoming);
}*/

void TTEApplicationBase::addBuffer(uint16 ctID, Buffer *buffer)
{
    buffers[ctID].push_back(buffer);
}

void TTEApplicationBase::executeCallback(Callback *cb){
    Enter_Method("executeCallback(cb)");
    cb->executeCallback();
}

} //namespace
