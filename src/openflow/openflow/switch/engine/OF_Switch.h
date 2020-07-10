

#ifndef OPENFLOW_OPENFLOW_SWITCH_OF_SWITCH_H_
#define OPENFLOW_OPENFLOW_SWITCH_OF_SWITCH_H_

#include <omnetpp.h>
#include "inet/transportlayer/contract/tcp/TCPSocket.h"

#include "openflow/openflow/switch/buffer/Buffer.h"
#include "openflow/messages/openflowprotocol/OFP_Message.h"
#include "openflow/openflow/switch/flowtable/old/Flow_Table.h"
#include <vector>
#include "inet/common/packet/Packet.h"

namespace ofp{

/**
 * Openflow Switching Engine.
 * TODO use new FlowTable module.
 */
class OF_Switch: public ::omnetpp::cSimpleModule
{
public:
    OF_Switch();
    ~OF_Switch();
    void disablePorts(std::vector<int> ports);
    virtual void finish();

protected:
    double flowTimeoutPollInterval;
    double serviceTime;
    bool busy;
    bool sendCompletePacket;

    long controlPlanePacket;
    long dataPlanePacket;
    long flowTableHit;
    long flowTableMiss;

    //stats
    ::omnetpp::simsignal_t dpPingPacketHash;
    ::omnetpp::simsignal_t cpPingPacketHash;
    ::omnetpp::simsignal_t queueSize;
    ::omnetpp::simsignal_t bufferSize;
    ::omnetpp::simsignal_t waitingTime;

    std::list<::omnetpp::cMessage *> msgList;
    std::vector<ofp_port> portVector;


    Buffer buffer;
    Flow_Table flowTable;
    inet::TcpSocket socket;

    virtual void initialize();
    virtual void handleMessage(::omnetpp::cMessage *msg);
    void connect(const char *connectToAddress);

    void processQueuedMsg(::omnetpp::cMessage *data_msg);
    void handleFeaturesRequestMessage(OFP_Message *of_msg);
    void handleFlowModMessage(OFP_Message *of_msg);
    void handlePacketOutMessage(OFP_Message *of_msg);
    void executePacketOutAction(ofp_action_output *action, inet::Packet *packet, uint32_t inport);
    void processFrame(inet::Packet *packet);
    void handleMissMatchedPacket(inet::Packet *packet);
};

} /*end namespace ofp*/

#endif /* OPENFLOW_OPENFLOW_SWITCH_OF_SWITCH_H_ */
