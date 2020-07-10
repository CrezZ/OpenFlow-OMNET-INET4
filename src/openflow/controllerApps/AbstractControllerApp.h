
#ifndef ABSTRACTCONTROLLERAPP_H_
#define ABSTRACTCONTROLLERAPP_H_

#include <omnetpp.h>
#include <openflow/openflow/protocol/OpenFlow.h>
#include <list>
#include "openflow/openflow/controller/OF_Controller.h"
#include "inet/transportlayer/contract/tcp/TcpSocket.h"
#include "openflow/messages/openflowprotocol/OFP_Packet_In.h"
#include "openflow/messages/openflowprotocol/OFP_Packet_Out.h"
#include "openflow/messages/openflowprotocol/OFP_Flow_Mod.h"

//INET
#include "inet/networklayer/contract/ipv4/Ipv4Address.h"
#include "inet/linklayer/common/MacAddress.h"

using namespace omnetpp;

namespace ofp{

struct CommonHeaderFields{
    uint32_t buffer_id;
    Switch_Info * swInfo;
    uint32_t inport;
    inet::MacAddress src_mac;
    inet::MacAddress dst_mac;
    int eth_type;
    inet::Ipv4Address arp_src_adr;
    inet::Ipv4Address arp_dst_adr;
    int arp_op;
};

class AbstractControllerApp:public cSimpleModule, public cListener {

protected:
    simsignal_t PacketInSignalId;
    simsignal_t PacketOutSignalId;
    simsignal_t PacketFeatureRequestSignalId;
    simsignal_t PacketFeatureReplySignalId;
    simsignal_t BootedSignalId;
    simsignal_t ExperimenterSignalID;

    long packetsFlooded;
    long packetsDropped;
    long numPacketOut;
    long numFlowMod;

    OF_Controller * controller;


    virtual void initialize();
    virtual void receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details) override;

    virtual OFP_Packet_Out * createFloodPacketFromPacketIn(OFP_Packet_In *packet_in_msg);
    virtual OFP_Packet_Out * createDropPacketFromPacketIn(OFP_Packet_In *packet_in_msg);
    virtual OFP_Packet_Out * createPacketOutFromPacketIn(OFP_Packet_In *packet_in_msg, uint32_t outport);

    virtual CommonHeaderFields extractCommonHeaderFields(OFP_Packet_In *packet_in_msg);

    virtual void floodPacket(OFP_Packet_In *packet_in_msg);
    virtual void dropPacket(OFP_Packet_In *packet_in_msg);
    virtual void sendPacket(OFP_Packet_In *packet_in_msg, uint32_t outport);
    virtual void sendFlowModMessage(ofp_flow_mod_command mod_com, const oxm_basic_match &match, int outport, inet::TcpSocket * socket, int idleTimeOut, int hardTimeOut);

public:
    AbstractControllerApp();
    ~AbstractControllerApp();
    virtual void finish();

};

} /*end namespace ofp*/


#endif
