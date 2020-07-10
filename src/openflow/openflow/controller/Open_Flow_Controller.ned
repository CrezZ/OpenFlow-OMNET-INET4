

package openflow.openflow.controller;
import inet.applications.contract.IApp;
import inet.linklayer.ethernet.EthernetInterface;
import inet.networklayer.common.InterfaceTable;
import inet.networklayer.contract.INetworkLayer;
import inet.networklayer.contract.IRoutingTable;
import inet.transportlayer.contract.ITcp;
import openflow.controllerApps.AbstractControllerApp;
import inet.common.MessageDispatcher;

//
// TODO documentation
//
module Open_Flow_Controller
{
    parameters:
        @networkNode();
        @labels(node,ethernet-node);
        @display("i=device/server;bgb=795,445;i2=openflow/OpenFlow-Logo-small");
        int numControllerApps = default(0);
        int numTcpControllerApps = default(0);

        string networkLayerType = default("IPv4NetworkLayer");
        string routingTableType = default("IPv4RoutingTable");

		//submodules
        string tcpType = default("TCP");  // tcp implementation (e.g. TCP, TCP_old, TCP_NSC)
        bool forwarding = default(false);
        bool multicastForwarding = default(false);
        string routingFile = default("");
        int numExtInterfaces = default(0);
        double serviceTime @unit("s") = default(0s);

        routingTable.forwarding = forwarding;
        routingTable.multicastForwarding = multicastForwarding;   // for IPv4, IPv6, Generic
        *.routingTableModule = default(absPath(".routingTable"));
        *.interfaceTableModule = default(absPath(".interfaceTable"));

    gates:
        inout ethg[] @labels(EtherFrame-conn);
    submodules:

        controllerApps[numControllerApps]: <> like AbstractControllerApp {
            parameters:
                @display("p=519,141,row,60");
        }

        tcpControllerApps[numTcpControllerApps]: <> like IApp {
            parameters:
                @display("p=200,141,row,60");
        }
        tcp: <tcpType> like ITcp {
            parameters:
                @display("p=200,278");
        }
        routingTable: <routingTableType> like IRoutingTable if routingTableType != "" {
            parameters:
                @display("p=733,333");
        }
        networkLayer: <networkLayerType> like INetworkLayer {
            parameters:
                @display("p=355,278;q=queue");
            gates:
                ifIn;
                ifOut;
        }
        eth[sizeof(ethg)]: EthernetInterface {
            parameters:
                @display("p=519,278,row,90;q=txQueue");
        }
        interfaceTable: InterfaceTable {
            parameters:
                @display("p=733,226");
        }
        nl_bus: MessageDispatcher {
        }
		tl_bus: MessageDispatcher {
        }

        OF_Controller: OF_Controller {
            @display("p=56,278");
            serviceTime = serviceTime;
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

        for i=0..numTcpControllerApps-1 {
            tcpControllerApps[i].socketOut --> tcp.appIn;
            tcpControllerApps[i].socketIn <-- tcp.appOut;
        }

        tcp.appOut --> tl_bus.in++;
        tl_bus.out++ --> OF_Controller.tcpIn;
        OF_Controller.tcpOut --> tl_bus.in++;
        tl_bus.out++ --> tcp.appIn;
}


