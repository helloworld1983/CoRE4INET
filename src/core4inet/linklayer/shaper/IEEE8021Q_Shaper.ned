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

package core4inet.linklayer.shaper;

import core4inet.linklayer.shaper.base.IShaper;

//A Shaper for IEEE802.1Q-tagged messages.
//
//The IEEE8021Q Shaper handles Q-tagged and best-effort traffic
//
// @see ~IShaper
//
// @author Till Steinbach, Philipp Meyer
//
simple IEEE8021Q_Shaper like IShaper
{
    parameters:
        @display("i=block/classifier");
        @class(IEEE8021Q_Shaper);
        
        //Signal emitted when queue length of the IEEE802.1Q queue changes, contains new queue length
        @signal[q*QueueLength](type=unsigned long; unit=packets);
        //Statistic of the queue length of IEEE802.1Q frames
        @statisticTemplate[qQueueLength](title="Q-Queue Length ($name)"; unit=packets; record=vector,stats; interpolationmode=sample-hold);
        //Signal emitted when queue size of the IEEE802.1Q queue changes, contains new queue size in bytes
        @signal[q*QueueSize](type=unsigned long; unit=B);
        //Statistic of the queue size ofIEEE802.1Q frames in bytes
        @statisticTemplate[qQueueSize](title="Q-Queue Size ($name)"; unit=B; record=vector,stats; interpolationmode=sample-hold);
		
		//Oscillator of the timed module (must be set to the name of an oscillator module in the Node) when left empty: oscillator
		string oscillator = default("^.^.scheduler.oscillator");
		//Timer of the timed module (must be set to the name of an timer module in the Node) when left empty: timer
        string timer = default("^.^.scheduler.timer");
        //Untagged VLAN. Outgoing frames with this VLAN get untagged
        int untaggedVID = default(0);
        //Interface allows frames with this VLAN to leave the port
        string taggedVIDs = default("0");
         //Default Priority. Frames without Q-tag will be treated with this priority
        int defaultPriority = default(0);
        
    gates:
        //Input for IEEE802.1Q frames from the upper layer (EtherLLC or IMacRelayUnit)
        input in @labels(EtherFrame);
        //Output to the lower layer (MAC layer)
        output out @labels(EtherFrame);
}
