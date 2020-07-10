

package openflow.hyperflow;
import inet.linklayer.ethernet.EthernetInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.contract.IRoutingTable;
import inet.transportlayer.contract.ITcp;
<<<<<<< HEAD
import inet.common.MessageDispatcher;
=======

>>>>>>> f68a6a33e7756736202e7b0724f61a1dfd3c8d10

//
// TODO documentation
//
module Hyper_Flow_Synchronizer
{
    parameters:
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=device/server;bgb=582,213;i2=openflow/OpenFlow-Logo-small");

		//submodules
        string tcpType = default("TCP");  // tcp implementation (e.g. TCP, TCP_old, TCP_NSC)
        int numExtInterfaces = default(0);
        double serviceTime @unit("s") = default(0s);

        string networkLayerType = default("IPv4NetworkLayer");
        string routingTableType = default("IPv4RoutingTable");

        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        string routingFile = default("");

        routingTable.forwarding = forwarding;
        routingTable.multicastForwarding = multicastForwarding;   // for IPv4, IPv6, Generic
        *.routingTableModule = default(absPath(".routingTable"));
        *.interfaceTableModule = default(absPath(".interfaceTable"));
 
    gates:
        inout ethg[] @labels(EtherFrame-conn);

    submodules:
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=422,35");
        }
        tcp: <tcpType> like ITcp {
            parameters:
                @display("p=246,138");
        }
        routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
            parameters:
                @display("p=53,250;is=s");
        }
        networkLayer: <networkLayerType> like INetworkLayer {
            parameters:
                @display("p=355,278;q=queue");
            gates:
                ifIn;
                ifOut;
        }
        nl_bus: MessageDispatcher {
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=432,138,row,90;q=txQueue");
        }
        HyperFlowSynchronizer: HyperFlowSynchronizer {
            @display("p=89,138");
        }
    connections allowunconnected:
        tcp.ipOut --> networkLayer.transportIn; 
        tcp.ipIn <-- networkLayer.transportOut;

        for i=0..sizeof(ethg)-1 {
            ethg[i] <--> eth[i].phys;
            eth[i].upperLayerOut --> nl_bus.in++;
            networkLayer.ifIn <-- nl_bus.out++;
            eth[i].upperLayerIn <-- nl_bus.out++;
            networkLayer.ifOut -->nl_bus.in++;
        }

        tcp.appOut --> HyperFlowSynchronizer.tcpIn;
        HyperFlowSynchronizer.tcpOut --> tcp.appIn;
}


