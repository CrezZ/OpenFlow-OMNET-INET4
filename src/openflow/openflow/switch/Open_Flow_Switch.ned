

package openflow.openflow.switch;

import openflow.openflow.switch.engine.OF_Switch;

<<<<<<< HEAD
import inet.linklayer.contract.IWiredInterface;
=======
import inet.common.MessageDispatcher;

import inet.linklayer.contract.IEthernetInterface;
>>>>>>> f68a6a33e7756736202e7b0724f61a1dfd3c8d10
import inet.linklayer.ethernet.EthernetInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.contract.IRoutingTable;
import inet.transportlayer.contract.ITcp;
<<<<<<< HEAD
import inet.common.MessageDispatcher;
=======

>>>>>>> f68a6a33e7756736202e7b0724f61a1dfd3c8d10

module Open_Flow_Switch
{
    parameters:
        @networkNode();
        @OF_Switch();
        @labels(node,ethernet-node);
        @display("i=device/switch;i2=openflow/OpenFlow-Logo-small");
        bool sendCompletePacket = default(false);
        double serviceTime @unit("s") = default(0s);
        bool csmacdSupport = default(true);  // by default use CSMA/CD
        string macType = default(csmacdSupport ? "EtherMAC" : "EtherMACFullDuplex"); // ~EtherMAC or ~EtherMACFullDuplex

        string networkLayerType = default("IPv4NetworkLayer");
        string routingTableType = default("IPv4RoutingTable");

        //submodules
        string tcpType = default("TCP");  // tcp implementation (e.g. TCP, TCP_old, TCP_NSC)
        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        string routingFile = default("");

        routingTable.forwarding = forwarding;
        routingTable.multicastForwarding = multicastForwarding;   // for IPv4, IPv6, Generic
        *.routingTableModule = default(absPath(".routingTable"));
        *.interfaceTableModule = default(absPath(".interfaceTable"));
        etherMAC[*].encapType = "EtherEncapDummy";
        etherMAC[*].csmacdSupport = csmacdSupport;
        etherMAC[*].macType = macType;
        networkLayer.proxyARP = false; 

    gates:
        inout gateControlPlane[] @labels(ControlPlane);
        inout gateDataPlane[] @labels(DataPlane);

    submodules:

        interfaceTable: InterfaceTable {
            parameters:
                @display("p=446,203");
        }
        tcp: <tcpType> like ITcp {
            parameters:
                @display("p=315,90");
        }
        routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
            parameters:
                @display("p=200,203");
        }
        l7: MessageDispatcher {
            parameters:
                @display("p=750,446;b=1000,5,,,,1");
        }
        l4: MessageDispatcher {
            parameters:
                @display("p=750,446;b=1000,5,,,,1");
        }
        l2: MessageDispatcher {
            parameters:
                @display("p=750,446;b=1000,5,,,,1");
        }
        
        networkLayer: <networkLayerType> like INetworkLayer {
            parameters:
                @display("p=200,90;q=queue");
<<<<<<< HEAD
            gates:
                ifIn;
                ifOut;
=======
>>>>>>> f68a6a33e7756736202e7b0724f61a1dfd3c8d10
        }
        eth[sizeof(gateControlPlane)]: EthernetInterface {
            parameters:
                @display("p=75,90,row,90;q=txQueue");
        }
<<<<<<< HEAD
        etherMAC[sizeof(gateDataPlane)]: <default("EthernetInterface")> like IWiredInterface {
=======
        etherMAC[sizeof(gateDataPlane)]: <default("EthernetInterface")> like IEthernetInterface {
>>>>>>> f68a6a33e7756736202e7b0724f61a1dfd3c8d10
            parameters:
                mac.promiscuous = true;
                @display("p=633,90,c,70");
        }

        OF_Switch: OF_Switch {
            @display("p=446,90");
            sendCompletePacket = sendCompletePacket;
            gates:
                dataPlaneOut[sizeof(gateDataPlane)];
                dataPlaneIn[sizeof(gateDataPlane)];
        }
        nl_bus: MessageDispatcher {
        }
		tl_bus: MessageDispatcher {
        }

    connections allowunconnected:
        for i=0..sizeof(gateControlPlane)-1 {
            eth[i].phys <--> gateControlPlane[i];
<<<<<<< HEAD
            eth[i].upperLayerOut --> nl_bus.in++;
            networkLayer.ifIn <-- nl_bus.out++;
            eth[i].upperLayerIn <-- nl_bus.out++;
            networkLayer.ifOut -->nl_bus.in++;
=======
            eth[i].upperLayerOut --> l2.in++ ;
            networkLayer.ifIn <-- l2.out++;
            eth[i].upperLayerIn <-- l2.out++;
            networkLayer.ifOut -->l2.in++;
>>>>>>> f68a6a33e7756736202e7b0724f61a1dfd3c8d10
        }

        for i=0..sizeof(gateDataPlane)-1 {
            etherMAC[i].phys <--> gateDataPlane[i];
            OF_Switch.dataPlaneOut[i] --> etherMAC[i].upperLayerIn;
            etherMAC[i].upperLayerOut --> OF_Switch.dataPlaneIn[i];
        }
<<<<<<< HEAD
        tcp.ipOut --> networkLayer.transportIn;
        tcp.ipIn <-- networkLayer.transportOut;

        tcp.appOut --> OF_Switch.controlPlaneIn;
        OF_Switch.controlPlaneOut --> tcp.appIn;
=======
        tcp.ipOut --> l4.in++;
        networkLayer.transportIn <-- l4.out++;
        tcp.ipIn <-- l4.out++;
        networkLayer.transportOut --> l4.in++;

        tcp.appOut --> l7.in++;
        OF_Switch.controlPlaneIn <--l7.out++;
        OF_Switch.controlPlaneOut --> l7.in++;
        tcp.appIn <-- l7.out++;
>>>>>>> f68a6a33e7756736202e7b0724f61a1dfd3c8d10


}
