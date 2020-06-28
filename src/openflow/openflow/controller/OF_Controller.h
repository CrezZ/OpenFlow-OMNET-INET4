
#ifndef OPENFLOW_OPENFLOW_CONTROLLER_OF_CONTROLLER_H_
#define OPENFLOW_OPENFLOW_CONTROLLER_OF_CONTROLLER_H_

#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "openflow/messages/openflowprotocol/OFP_Message.h"
#include "openflow/openflow/controller/Switch_Info.h"
#include <omnetpp.h>

using namespace omnetpp;

namespace ofp{
    class AbstractControllerApp;
}

namespace ofp{

class OF_Controller: public cSimpleModule
{
public:
    OF_Controller();
    ~OF_Controller();
    virtual void finish();

    void sendPacketOut(OFP_Message *of_msg, inet::TcpSocket * socket);

    void registerApp(AbstractControllerApp * app);

    inet::TcpSocket *findSocketFor(cMessage *msg) const;
    Switch_Info *findSwitchInfoFor(cMessage *msg) ;
    inet::TcpSocket *findSocketForChassisId(std::string chassisId) const;

    std::vector<Switch_Info >* getSwitchesList() ;
    std::vector<AbstractControllerApp *>* getAppList() ;



protected:
    /**
     * Observer Signals
     */
    simsignal_t PacketInSignalId;
    simsignal_t PacketOutSignalId;
    simsignal_t PacketHelloSignalId;
    simsignal_t PacketFeatureRequestSignalId;
    simsignal_t PacketFeatureReplySignalId;
    simsignal_t BootedSignalId;
    simsignal_t ExperimenterSignalID;

    /**
     * Statistics
     */
    simsignal_t queueSize;
    simsignal_t waitingTime;
    long numPacketIn;
    std::map<int,int> packetsPerSecond;

    int lastQueueSize;
    double lastChangeTime;
    std::map<int,double> avgQueueSize;

    /**
     * Message Processing
     */
    bool _booted = false;
    bool _busy;
    double _serviceTime;
    std::list<cMessage *> _msgList;

    /**
     * Network and Controller State
     */
    std::vector<Switch_Info > _switchesList;
    std::vector<AbstractControllerApp *> _apps;

    /**
     * Connection
     */
    inet::TcpSocket _socket;

    virtual void initialize();
    virtual void handleMessage(cMessage *msg);
    void processQueuedMsg(cMessage *data_msg);
    void calcAvgQueueSize(int size);
    void sendHello(OFP_Message *msg);
    void registerConnection(OFP_Message *msg);
    void sendFeatureRequest(cMessage *msg);
    virtual void handleFeaturesReply(OFP_Message *of_msg);
    virtual void handlePacketIn(OFP_Message *of_msg);

private:
    void handleExperimenterMsg(OFP_Message* of_msg);
};

} /*end namespace ofp*/

#endif /* OPENFLOW_OPENFLOW_CONTROLLER_OF_CONTROLLER_H_ */
