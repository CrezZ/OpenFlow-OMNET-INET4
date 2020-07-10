#include <omnetpp.h>
#include <openflow/openflow/protocol/OpenFlow.h>
#include "openflow/openflow/controller/OF_Controller.h"

#include "inet/applications/tcpapp/GenericAppMsg_m.h"
#include "openflow/messages/openflowprotocol/OFP_Message.h"
#include "openflow/messages/openflowprotocol/OFP_Packet_Out.h"
#include "inet/transportlayer/contract/tcp/TCPCommand_m.h"
#include "openflow/messages/openflowprotocol/OFP_Flow_Mod.h"
#include "openflow/messages/openflowprotocol/OFP_Features_Request.h"
#include "openflow/messages/openflowprotocol/OFP_Features_Reply.h"
#include "inet/transportlayer/tcp/TCPConnection.h"
#include "openflow/controllerApps/AbstractControllerApp.h"
#include "openflow/messages/openflowprotocol/OFP_Hello.h"
#include "openflow/openflow/util/ofmessagefactory/OFMessageFactory.h"

using namespace std;
using namespace inet;


namespace ofp{

Define_Module(OF_Controller);

#define MSGKIND_BOOTED 100



OF_Controller::OF_Controller(){

}

OF_Controller::~OF_Controller(){
    for(auto&& msg : _msgList) {
      delete msg;
    }
    _msgList.clear();
}

void OF_Controller::initialize(){
    //register signals
    PacketInSignalId =registerSignal("PacketIn");
    PacketOutSignalId =registerSignal("PacketOut");
    PacketHelloSignalId =registerSignal("PacketHello");
    PacketFeatureRequestSignalId = registerSignal("PacketFeatureRequest");
    PacketFeatureReplySignalId = registerSignal("PacketFeatureReply");
    BootedSignalId = registerSignal("Booted");
    ExperimenterSignalID = registerSignal("Experimenter");

    //stats
    queueSize = registerSignal("queueSize");
    waitingTime = registerSignal("waitingTime");
    numPacketIn=0;

    lastQueueSize =0;
    lastChangeTime=0.0;

    //parameters
    _serviceTime = par("serviceTime");
    _busy = false;

    // TCP socket; listen on incoming connections
    const char *address = par("address");
    int port = par("port");
    _socket.setOutputGate(gate("tcpOut"));
    //_socket.setDataTransferMode(TCP_TRANSFER_OBJECT);
    _socket.bind(address[0] ? L3Address(address) : L3Address(), port);
    _socket.listen();

    //schedule booted message
    cMessage *booted = new cMessage("Booted");
    booted->setKind(MSGKIND_BOOTED);
    scheduleAt(simTime() + par("bootTime").doubleValue(), booted);
}


void OF_Controller::handleMessage(cMessage *msg){
    if (msg->isSelfMessage())
    {   // this is a self message
        if (msg->getKind()==MSGKIND_BOOTED)
        { // we are now ready to handle messages
            _booted = true;
            emit(BootedSignalId, this);
        }
        else
        {   //This is message which has been scheduled due to service time
            //Get the Original message
            cMessage *data_msg = (cMessage *) msg->getContextPointer();
            emit(waitingTime,(simTime()-data_msg->getArrivalTime()-_serviceTime));
            processQueuedMsg(data_msg);

            //delete the processed msg
            delete data_msg;

            //Trigger next service time
            if (_msgList.empty()){
                _busy = false;
            } else {
                cMessage *msgfromlist = _msgList.front();
                _msgList.pop_front();
                cMessage *event = new cMessage("event");
                event->setContextPointer(msgfromlist);
                scheduleAt(simTime()+_serviceTime, event);
            }
            calcAvgQueueSize(_msgList.size());
        }

        // delete self message after handling it.
        delete msg;
    }
    else if (_booted)
    {   // we are booted schedule message according to service time
        if (_busy)
        { //there are scheduled messages so append this to the list.
            _msgList.push_back(msg);
        }
        else
        { // queue is empty so schedule it with service time.
            _busy = true; // mark that we are now busy
            cMessage *event = new cMessage("event");
            event->setContextPointer(msg);
            scheduleAt(simTime()+_serviceTime, event);
        }

        //do statistic stuff.
        if(packetsPerSecond.count(floor(simTime().dbl())) <=0){
            packetsPerSecond.insert(pair<int,int>(floor(simTime().dbl()),1));
        } else {
            packetsPerSecond[floor(simTime().dbl())]++;
        }
        calcAvgQueueSize(_msgList.size());
        emit(queueSize,static_cast<unsigned long>(_msgList.size()));
    }
    else
    {   // this is not a self message and we are not yet booted
        // ignore it.
        delete msg;
    }
}

void OF_Controller::calcAvgQueueSize(int size){
    if(lastQueueSize != size){
        double timeDiff = simTime().dbl() - lastChangeTime;
        if(avgQueueSize.count(floor(simTime().dbl())) <=0){
            avgQueueSize.insert(pair<int,double>(floor(simTime().dbl()),lastQueueSize*timeDiff));
        } else {
            avgQueueSize[floor(simTime().dbl())] += lastQueueSize*timeDiff;
        }
            lastChangeTime = simTime().dbl();
            lastQueueSize = size;
        }
}

void OF_Controller::processQueuedMsg(cMessage *data_msg){
    if (OFP_Message *of_msg = dynamic_cast<OFP_Message *>(data_msg))
    {   // this is a openflow message
        ofp_type type = (ofp_type) of_msg->getHeader().type;

        switch (type) {
            case OFPT_HELLO:
                        registerConnection(of_msg);
                        sendHello(of_msg);
                        sendFeatureRequest(data_msg);
                        break;

            case OFPT_FEATURES_REPLY:
                handleFeaturesReply(of_msg);
                break;

            case OFPT_PACKET_IN:
                EV << "packet-in message from switch\n";
                handlePacketIn(of_msg);
                break;

#if OFP_VERSION_IN_USE == OFP_100
            case OFPT_VENDOR:
                // the controller apps might want to implement vendor specific features so forward them.
                handleExperimenterMsg(of_msg);
                break;

#elif  OFP_VERSION_IN_USE == OFP_151
            case OFPT_EXPERIMENTER:
                // the controller apps might want to implement experimental features so forward them.
                handleExperimenterMsg(of_msg);
                break;
#endif

            default:
                break;
        }
    }
}


void OF_Controller::sendHello(OFP_Message *msg){
    OFP_Hello* hello = OFMessageFactory::instance()->createHello();

    emit(PacketHelloSignalId,hello);
    findSocketFor(msg)->send(hello);
}

void OF_Controller::sendFeatureRequest(cMessage *msg){

    OFP_Features_Request *featuresRequest = OFMessageFactory::instance()->createFeatureRequest();

    emit(PacketFeatureRequestSignalId,featuresRequest);
    findSocketFor(msg)->send(featuresRequest);
}

void OF_Controller::handleFeaturesReply(OFP_Message *of_msg){
    EV << "OFA_controller::handleFeaturesReply" << endl;
    Switch_Info *swInfo= findSwitchInfoFor(of_msg);
    if(dynamic_cast<OFP_Features_Reply *>(of_msg) != NULL){
        //TODO use OFProtoParser for Feature Reply -> Switch Info
        OFP_Features_Reply * castMsg = (OFP_Features_Reply *)of_msg;
        swInfo->setMacAddress(castMsg->getDatapath_id());
        swInfo->setNumOfPorts(castMsg->getPortsArraySize());
        emit(PacketFeatureReplySignalId,castMsg);
    }
}

void OF_Controller::handlePacketIn(OFP_Message *of_msg){
    EV << "OFA_controller::handlePacketIn" << endl;
    numPacketIn++;
    emit(PacketInSignalId,of_msg);
}

void OF_Controller::handleExperimenterMsg(OFP_Message* of_msg) {
    EV << "OFA_controller::handleExperimenterMsg" << endl;
    emit(ExperimenterSignalID, of_msg);
}


void OF_Controller::sendPacketOut(OFP_Message *of_msg, TcpSocket *socket){
    Enter_Method_Silent();
    take(of_msg);
    EV << "OFA_controller::sendPacketOut" << endl;
    emit(PacketOutSignalId,of_msg);
    socket->send(of_msg);
}



void OF_Controller::registerConnection(OFP_Message *msg){
    TcpSocket *socket = findSocketFor(msg);
    if(!socket){
        socket = new TcpSocket(msg);
        socket->setOutputGate(gate("tcpOut"));
        Switch_Info swInfo = Switch_Info();
        swInfo.setSocket(socket);
        swInfo.setConnId(socket->getSocketId());
        swInfo.setMacAddress("");
        swInfo.setNumOfPorts(-1);
        swInfo.setVersion(msg->getHeader().version);
        _switchesList.push_back(swInfo);
    }
}


TcpSocket *OF_Controller::findSocketFor(cMessage *msg) const{
    TcpCommand *ind = dynamic_cast<TcpCommand *>(msg->getControlInfo());
    if (!ind)
        throw cRuntimeError("TCPSocketMap: findSocketFor(): no TCPCommand control info in message (not from TCP?)");

//    int connId = ind->getConnId();
      int connId = ind->getUserId();
    for(auto i=_switchesList.begin(); i != _switchesList.end(); ++i) {
        if((*i).getConnId() == connId){
            return (*i).getSocket();
        }
    }
    return NULL;
}


Switch_Info *OF_Controller::findSwitchInfoFor(cMessage *msg) {
    TcpCommand *ind = dynamic_cast<TcpCommand *>(msg->getControlInfo());
    if (!ind)
        return NULL;
    //    int connId = ind->getConnId();
    int connId = ind->getUserId();
    for(auto i=_switchesList.begin(); i != _switchesList.end(); ++i) {
        if((*i).getConnId() == connId){
            return &(*i);
        }
    }
    return NULL;
}

TcpSocket *OF_Controller::findSocketForChassisId(std::string chassisId) const{
    for(auto i=_switchesList.begin(); i != _switchesList.end(); ++i) {
        if(strcmp((*i).getMacAddress().c_str(),chassisId.c_str())==0){
            return (*i).getSocket();
        }
    }
    return NULL;
}

void OF_Controller::registerApp(AbstractControllerApp *app){
    _apps.push_back(app);
}

std::vector<Switch_Info >* OF_Controller::getSwitchesList() {
    return &_switchesList;
}

std::vector<AbstractControllerApp *>* OF_Controller::getAppList() {
    return &_apps;
}

void OF_Controller::finish(){
    // record statistics
    recordScalar("numPacketIn", numPacketIn);

    std::map<int,int>::iterator iterMap;
    for(iterMap = packetsPerSecond.begin(); iterMap != packetsPerSecond.end(); iterMap++){
        stringstream name;
        name << "packetsPerSecondAt-" << iterMap->first;
        recordScalar(name.str().c_str(),iterMap->second);
    }

    std::map<int,double>::iterator iterMap2;
    for(iterMap2 = avgQueueSize.begin(); iterMap2 != avgQueueSize.end(); iterMap2++){
        stringstream name;
        name << "avgQueueSizeAt-" << iterMap2->first;
        recordScalar(name.str().c_str(),(iterMap2->second/1.0));
    }
}

} /*end namespace ofp*/

