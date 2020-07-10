#include "openflow/controllerApps/ARPResponder.h"
#include <algorithm>
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "openflow/openflow/util/ofmessagefactory/OFMessageFactory.h"

using namespace inet;

namespace ofp{

Define_Module(ARPResponder);

#define MSGKIND_ARPRESPONDERBOOTED 801


ARPResponder::ARPResponder(){

}

ARPResponder::~ARPResponder(){

}

void ARPResponder::initialize(){
    AbstractControllerApp::initialize();
    //stats
    floodedArp = 0l;
    answeredArp = 0l;
}


void ARPResponder::handleMessage(cMessage *msg){
    delete msg;
}


bool ARPResponder::addEntry(std::string srcIp, MacAddress srcMac){
    if(macToIp.count(srcMac) <= 0){
        //add him to our table
        macToIp[srcMac] = srcIp;
        ipToMac[srcIp] = srcMac;
        return true;
    }
    return false;
}


void ARPResponder::handlePacketIn(OFP_Packet_In * packet_in_msg){

    CommonHeaderFields headerFields = extractCommonHeaderFields(packet_in_msg);

    //check if it is an arp packet
    if(headerFields.eth_type == ETHERTYPE_ARP){

            //add entry if not existent
            addEntry(headerFields.arp_src_adr.str(),headerFields.src_mac);

            //check arp type
            if(headerFields.arp_op == ARP_REQUEST){

                //can we give the reply directly
                if(ipToMac.count(headerFields.arp_dst_adr.str()) >0){

                    //drop the orginal packet
                    dropPacket(packet_in_msg);

                    //encap the arp reply
                    inet::Packet* arpReply = createArpReply(
                        headerFields.arp_dst_adr, headerFields.arp_src_adr,
                        ipToMac[headerFields.arp_dst_adr.str()],
                        headerFields.src_mac);

                    OFP_Packet_Out* packetOut = OFMessageFactory::instance()->createPacketOut(&headerFields.inport, 1, -1, OFP_NO_BUFFER, arpReply);

                    //send the packet
                    answeredArp++;
                    controller->sendPacketOut(packetOut,headerFields.swInfo->getSocket());
                    delete arpReply;
                } else {
                    //we need to flood the packet
                    floodedArp++;
                    floodPacket(packet_in_msg);
                }
            }

    }

}


void ARPResponder::receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details) {
    AbstractControllerApp::receiveSignal(src,id,obj,details);

    if(id == PacketInSignalId){
        EV << "ARPResponder::PacketIn" << '\n';
        if (dynamic_cast<OFP_Packet_In *>(obj) != NULL) {
            OFP_Packet_In *packet_in_msg = (OFP_Packet_In *) obj;
            handlePacketIn(packet_in_msg);
        }
    }
}

Packet * ARPResponder::createArpReply(Ipv4Address srcIp, Ipv4Address dstIp, MacAddress srcMac,MacAddress dstMac){
    ArpPacket *arpReply = new ArpPacket("controllerArpReply");
    arpReply->setOpcode(ARP_REPLY);
    arpReply->setName("arpReply");
    arpReply->setSrcIPAddress(srcIp);
    arpReply->setSrcMACAddress(srcMac);
    arpReply->setDestIPAddress(dstIp);
    arpReply->setDestMACAddress(dstMac);
    arpReply->setByteLength(28);
    delete arpReply->removeControlInfo();


    Packet *frame = NULL;
    Packet *eth2Frame = new Packet(arpReply->getName());
    eth2Frame->setSrc(arpReply->getSrcMACAddress());  // if blank, will be filled in by MAC
    eth2Frame->setDest(arpReply->getDestMACAddress());
    eth2Frame->setEtherType(ETHERTYPE_ARP);
    eth2Frame->setByteLength(ETHER_MAC_FRAME_BYTES);

    frame = eth2Frame;
    frame->encapsulate(arpReply);
    if (frame->getByteLength() < MIN_ETHERNET_FRAME_BYTES)
        frame->setByteLength(MIN_ETHERNET_FRAME_BYTES);  // "padding"

    frame->addByteLength(PREAMBLE_BYTES+SFD_BYTES);

    return frame;
}


void ARPResponder::finish(){
    AbstractControllerApp::finish();
    // record statistics
    recordScalar("arpFlooded", floodedArp);
    recordScalar("arpAnswered", answeredArp);
}

} /*end namespace ofp*/

