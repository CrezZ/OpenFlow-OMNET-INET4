#include "openflow/controllerApps/AbstractControllerApp.h"
//#include "openflow/messages/openflowprotocol/OFP_Message.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "openflow/openflow/util/ofmessagefactory/OFMessageFactory.h"

using namespace inet;



namespace ofp{

Define_Module(AbstractControllerApp);

AbstractControllerApp::AbstractControllerApp()
{

}

AbstractControllerApp::~AbstractControllerApp()
{

}


void AbstractControllerApp::initialize(){
    //register signals
    PacketInSignalId =registerSignal("PacketIn");
    PacketOutSignalId =registerSignal("PacketOut");
    PacketFeatureRequestSignalId = registerSignal("PacketFeatureRequest");
    PacketFeatureReplySignalId = registerSignal("PacketFeatureReply");
    BootedSignalId= registerSignal("Booted");
    ExperimenterSignalID = registerSignal("Experimenter");

    getParentModule()->subscribe("PacketIn",this);
    getParentModule()->subscribe("PacketOut",this);
    getParentModule()->subscribe("PacketFeatureRequest",this);
    getParentModule()->subscribe("PacketFeatureReply",this);
    getParentModule()->subscribe("Booted",this);
    getParentModule()->subscribe("Experimenter",this);

    packetsFlooded=0;
    packetsDropped=0;
    numPacketOut=0;
    numFlowMod=0;

    controller= NULL;
}


void AbstractControllerApp::receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details) {

    //register at controller
    if(id == BootedSignalId){
        EV << "ARPResponder::Booted" << '\n';
        if (dynamic_cast<OF_Controller *>(obj) != NULL) {
            OF_Controller *cntrl = (OF_Controller *) obj;
            this->controller = cntrl;
            controller->registerApp(this);
        }

    }
}

void AbstractControllerApp::floodPacket(OFP_Packet_In *packet_in_msg){
    EV << "floodPacket" << '\n';
    packetsFlooded++;

    TcpSocket *socket = controller->findSocketFor(packet_in_msg);
    controller->sendPacketOut(createFloodPacketFromPacketIn(packet_in_msg), socket);
}

void AbstractControllerApp::dropPacket(OFP_Packet_In *packet_in_msg){
    EV << "dropPacket" << '\n';
    packetsDropped++;

    TcpSocket *socket = controller->findSocketFor(packet_in_msg);
    controller->sendPacketOut(createDropPacketFromPacketIn(packet_in_msg), socket);
}


void AbstractControllerApp::sendPacket(OFP_Packet_In *packet_in_msg, uint32_t outport){
    EV << "sendPacket" << '\n';
    numPacketOut++;

    TcpSocket *socket = controller->findSocketFor(packet_in_msg);
    socket->send(createPacketOutFromPacketIn(packet_in_msg,outport));
}

void AbstractControllerApp::sendFlowModMessage(ofp_flow_mod_command mod_com, const oxm_basic_match &match, int outport, TcpSocket * socket, int idleTimeOut =1 , int hardTimeOut=0){
    EV << "sendFlowModMessage" << '\n';
    numFlowMod++;

    uint32_t out = (uint32_t)outport;
    OFP_Flow_Mod* flow_mod_msg = OFMessageFactory::instance()->createFlowModMessage(mod_com, match, 0, &out, 1, idleTimeOut, hardTimeOut);

    socket->send(flow_mod_msg);
}

void AbstractControllerApp::finish(){
    // record statistics
    recordScalar("numPacketOut", numPacketOut);
    recordScalar("numFlowMod", numFlowMod);
    recordScalar("packetsDropped", packetsDropped);
    recordScalar("packetsFlooded", packetsFlooded);

}


OFP_Packet_Out * AbstractControllerApp::createPacketOutFromPacketIn(OFP_Packet_In *packet_in_msg, uint32_t outport){

    OFP_Packet_Out *packetOut = nullptr;


    if (packet_in_msg->getBuffer_id() == OFP_NO_BUFFER){

        EthernetIIFrame *frame =  dynamic_cast<EthernetIIFrame *>(packet_in_msg->getEncapsulatedPacket());
        if(frame){
            packetOut = OFMessageFactory::instance()->createPacketOut(&outport, 1, frame->getArrivalGate()->getIndex(), packet_in_msg->getBuffer_id(), frame);
        } else {
            throw cRuntimeError("AbstractControllerApp::createPacketOutFromPacketIn: OFP_NO_BUFFER was set but no frame was provided in packet in");
        }
    } else {
        packetOut = OFMessageFactory::instance()->createPacketOut(&outport, 1, packet_in_msg->getMatch().in_port, packet_in_msg->getBuffer_id());
    }

    return packetOut;
}



OFP_Packet_Out * AbstractControllerApp::createFloodPacketFromPacketIn(OFP_Packet_In *packet_in_msg){

    OFP_Packet_Out *packetOut = nullptr;
    uint32_t outport = OFPP_FLOOD;

    if (packet_in_msg->getBuffer_id() == OFP_NO_BUFFER){

        EthernetIIFrame *frame =  dynamic_cast<EthernetIIFrame *>(packet_in_msg->getEncapsulatedPacket());
        if(frame){
            packetOut = OFMessageFactory::instance()->createPacketOut(&outport, 1, frame->getArrivalGate()->getIndex(), packet_in_msg->getBuffer_id(), frame);
        } else {
            throw cRuntimeError("AbstractControllerApp::createPacketOutFromPacketIn: OFP_NO_BUFFER was set but no frame was provided in packet in");
        }
    } else {
        packetOut = OFMessageFactory::instance()->createPacketOut(&outport, 1, packet_in_msg->getMatch().in_port, packet_in_msg->getBuffer_id());
    }

    return packetOut;
}

OFP_Packet_Out * AbstractControllerApp::createDropPacketFromPacketIn(OFP_Packet_In *packet_in_msg){
    OFP_Packet_Out *packetOut = nullptr;

    if (packet_in_msg->getBuffer_id() == OFP_NO_BUFFER){

        EthernetIIFrame *frame =  dynamic_cast<EthernetIIFrame *>(packet_in_msg->getEncapsulatedPacket());
        if(frame){
            packetOut = OFMessageFactory::instance()->createPacketOut(nullptr, 0, frame->getArrivalGate()->getIndex(), packet_in_msg->getBuffer_id(), frame);
        } else {
            throw cRuntimeError("AbstractControllerApp::createPacketOutFromPacketIn: OFP_NO_BUFFER was set but no frame was provided in packet in");
        }
    } else {
        packetOut = OFMessageFactory::instance()->createPacketOut(nullptr, 0, packet_in_msg->getMatch().in_port, packet_in_msg->getBuffer_id());
    }

    return packetOut;
}

CommonHeaderFields AbstractControllerApp::extractCommonHeaderFields(OFP_Packet_In *packet_in_msg){
    CommonHeaderFields headerFields = CommonHeaderFields();

    headerFields.buffer_id = packet_in_msg->getBuffer_id();
    headerFields.swInfo = controller->findSwitchInfoFor(packet_in_msg);


    // packet is encapsulated in packet-in message
    if (headerFields.buffer_id == OFP_NO_BUFFER){
        headerFields.inport = packet_in_msg->getEncapsulatedPacket()->getArrivalGate()->getIndex();
        headerFields.src_mac = dynamic_cast<EthernetIIFrame *>(packet_in_msg->getEncapsulatedPacket())->getSrc();
        headerFields.dst_mac = dynamic_cast<EthernetIIFrame *>(packet_in_msg->getEncapsulatedPacket())->getDest();
        headerFields.eth_type = dynamic_cast<EthernetIIFrame *>(packet_in_msg->getEncapsulatedPacket())->getEtherType();

        if(headerFields.eth_type ==ETHERTYPE_ARP){
            ARPPacket *arpPacket = check_and_cast<ARPPacket *>((dynamic_cast<EthernetIIFrame *>(packet_in_msg->getEncapsulatedPacket()))->getEncapsulatedPacket());
            headerFields.arp_src_adr = arpPacket->getSrcIPAddress();
            headerFields.arp_dst_adr = arpPacket->getDestIPAddress();
            headerFields.arp_op = arpPacket->getOpcode();
        }
    }else{
        headerFields.inport = packet_in_msg->getMatch().in_port;
        headerFields.src_mac = packet_in_msg->getMatch().dl_src;
        headerFields.dst_mac = packet_in_msg->getMatch().dl_dst;
        headerFields.eth_type = packet_in_msg->getMatch().dl_type;
        headerFields.arp_src_adr = packet_in_msg->getMatch().nw_src;
        headerFields.arp_dst_adr = packet_in_msg->getMatch().nw_dst;
        headerFields.arp_op = packet_in_msg->getMatch().nw_proto;
    }

    return headerFields;

}


} /*end namespace ofp*/

