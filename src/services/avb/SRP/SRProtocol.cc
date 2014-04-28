//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "SRProtocol.h"

#include "SRPFrame_m.h"

#include "AVBDefs.h"

#include "ExtendedIeee802Ctrl_m.h"
#include "EtherLLC.h"

#define ETHERAPP_BUFFER_SAP  0xe1

namespace CoRE4INET {

Define_Module(SRProtocol);

void SRProtocol::initialize()
{
    srpTable = (SRPTable *) getParentModule()->getSubmodule("srpTable");
    if (!srpTable)
    {
        throw cRuntimeError("srpTable module required for stream reservation");
    }
    srpTable->subscribe("talkerRegistered", this);
    srpTable->subscribe("listenerRegistered", this);
}

void SRProtocol::handleMessage(cMessage *msg)
{
    if (msg->arrivedOn("in"))
    {
        Ieee802Ctrl *etherctrl = dynamic_cast<Ieee802Ctrl *>(msg->removeControlInfo());
        if (!etherctrl)
        {
            error("packet `%s' from lower layer received without Ieee802Ctrl", msg->getName());
        }
        int arrivedOn = etherctrl->getSwitchPort();
        cModule *port = getParentModule()->getSubmodule("phy", arrivedOn);

        if (TalkerAdvertise* talkerAdvertise = dynamic_cast<TalkerAdvertise*>(msg))
        {
            //TODO Minor: try to get VLAN
            srpTable->updateTalkerWithStreamId(talkerAdvertise->getStreamID(), port, &etherctrl->getSrc(), SR_CLASS_A,
                    talkerAdvertise->getMaxFrameSize(), talkerAdvertise->getMaxIntervalFrames(), 0);
        }
        else if (ListenerReady* listenerReady = dynamic_cast<ListenerReady*>(msg))
        {
            unsigned long utilizedBandwidth = srpTable->getBandwidthForModule(port);
            //Add Higher Priority Bandwidth
            utilizedBandwidth += port->getSubmodule("shaper")->par("AVBHigherPriorityBandwidth").longValue();
            //TODO Minor: try to get VLAN
            unsigned long requiredBandwidth = srpTable->getBandwidthForStream(listenerReady->getStreamID(), 0);

            cGate *physOutGate = port->getSubmodule("mac")->gate("phys$o");
            cChannel *outChannel = physOutGate->findTransmissionChannel();

            unsigned long totalBandwidth = outChannel->getNominalDatarate();

            double reservableBandwidth = par("reservableBandwidth").doubleValue();

            if ((utilizedBandwidth + requiredBandwidth) <= (totalBandwidth * reservableBandwidth))
            {
                //TODO Minor: try to get VLAN
                srpTable->updateListenerWithStreamId(listenerReady->getStreamID(), port, 0);
                //TODO: Update Table if enough bandwidth
                EV_DETAIL << "Listener for stream " << listenerReady->getStreamID() << " registered on port "
                        << port->getFullName() << ". Utilized Bandwidth: "
                        << (utilizedBandwidth + requiredBandwidth) / (double) 1000000 << " of "
                        << (totalBandwidth * reservableBandwidth) / (double) 1000000 << " reservable Bandwidth of "
                        << totalBandwidth / (double) 1000000 << " MBit/s.";
            }
            else
            {
                bubble("ListenerReady Failed!");
                EV_DETAIL << "Listener for stream " << listenerReady->getStreamID()
                        << " could not be registered on port " << port->getFullName() << ". Required bandwidth: "
                        << requiredBandwidth / (double) 1000000 << "MBit/s, remaining bandwidth "
                        << ((totalBandwidth * reservableBandwidth) - utilizedBandwidth) / (double) 1000000 << "MBit/s.";
                SRPFrame *listenerReadyFailed = new ListenerReadyFailed("Listener Ready Failed", IEEE802CTRL_DATA);
                listenerReadyFailed->setStreamID(listenerReady->getStreamID());

                ExtendedIeee802Ctrl *answerEtherctrl = new ExtendedIeee802Ctrl();
                answerEtherctrl->setEtherType(MSRP_ETHERTYPE);
                answerEtherctrl->setDest(SRP_ADDRESS);
                answerEtherctrl->setSwitchPort(port->getIndex());
                listenerReadyFailed->setControlInfo(answerEtherctrl);
                send(listenerReadyFailed, gate("out"));
            }
        }
        else if (ListenerReadyFailed* listenerReadyFailed = dynamic_cast<ListenerReadyFailed*>(msg))
        {
            bubble("ListenerReady Failed!");
            std::list<cModule*> listeners = srpTable->getListenersForStreamId(listenerReadyFailed->getStreamID() ,0);
            for(std::list<cModule*>::iterator listener = listeners.begin(); listener != listeners.end(); listener++){
                srpTable->removeListenerWithStreamId(listenerReadyFailed->getStreamID(), (*listener), 0, true);
            }
        }
        delete etherctrl;
    }
    delete msg;
}

void SRProtocol::receiveSignal(cComponent *src, simsignal_t id, cObject *obj)
{
    Enter_Method_Silent
    ();
    if (id == registerSignal("talkerRegistered"))
    {
        SRPTable::TalkerEntry *tentry = (SRPTable::TalkerEntry*) obj;

        TalkerAdvertise *talkerAdvertise = new TalkerAdvertise("Talker Advertise", IEEE802CTRL_DATA);
        talkerAdvertise->setStreamID(tentry->streamId);
        talkerAdvertise->setMaxFrameSize(tentry->framesize);
        talkerAdvertise->setMaxIntervalFrames(tentry->intervalFrames);

        ExtendedIeee802Ctrl *etherctrl = new ExtendedIeee802Ctrl();
        etherctrl->setEtherType(MSRP_ETHERTYPE);
        etherctrl->setDest(SRP_ADDRESS);
        etherctrl->setSwitchPort(SWITCH_PORT_BROADCAST);
        talkerAdvertise->setControlInfo(etherctrl);

        //If talker was received from phy we have to exclude the incoming port
        if (strcmp(tentry->module->getName(), "phy") == 0)
        {
            etherctrl->setNotSwitchPort(tentry->module->getIndex());
        }

        send(talkerAdvertise, gate("out"));
    }
    else if (id == registerSignal("listenerRegistered"))
    {
        SRPTable::ListenerEntry *lentry = (SRPTable::ListenerEntry*) obj;

        //Get Talker Port
        SRPTable *srpTable = (SRPTable *) src;
        //TODO Minor: try to get VLAN
        cModule* talker = srpTable->getTalkerForStreamId(lentry->streamId, 0);
        //Send listener ready ony when talker is not a local application
        if (strcmp(talker->getName(), "phy") == 0)
        {
            SRPFrame *listenerReady = new ListenerReady("Listener Ready", IEEE802CTRL_DATA);
            listenerReady->setStreamID(lentry->streamId);

            ExtendedIeee802Ctrl *etherctrl = new ExtendedIeee802Ctrl();
            etherctrl->setEtherType(MSRP_ETHERTYPE);
            etherctrl->setDest(SRP_ADDRESS);
            etherctrl->setSwitchPort(talker->getIndex());
            listenerReady->setControlInfo(etherctrl);

            send(listenerReady, gate("out"));
        }

    }
}

} /* namespace CoRE4INET */