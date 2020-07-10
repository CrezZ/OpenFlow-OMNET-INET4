#include "openflow/controllerApps/LearningSwitch.h"
#include "openflow/openflow/controller/Switch_Info.h"

using namespace inet;

namespace ofp{

Define_Module(LearningSwitch);

LearningSwitch::LearningSwitch(){

}

LearningSwitch::~LearningSwitch(){

}

void LearningSwitch::initialize(){
    AbstractControllerApp::initialize();
    idleTimeout = par("flowModIdleTimeOut");
    hardTimeout = par("flowModHardTimeOut");
}

void LearningSwitch::receiveSignal(cComponent *src, simsignal_t id, cObject *obj, cObject *details) {
    AbstractControllerApp::receiveSignal(src,id,obj,details);

    if(id == PacketInSignalId){
        EV << "LearningSwitch::PacketIn" << endl;
        if (dynamic_cast<OFP_Packet_In *>(obj) != NULL) {
            OFP_Packet_In *packet_in = (OFP_Packet_In *) obj;
            doSwitching(packet_in);
        }
    }
}


void LearningSwitch::doSwitching(OFP_Packet_In *packet_in_msg){

    CommonHeaderFields headerFields = extractCommonHeaderFields(packet_in_msg);

    //search map for source mac address and enter
    if(lookupTable.count(headerFields.swInfo)<=0){
        lookupTable[headerFields.swInfo]= std::map<MacAddress,uint32_t>();
        lookupTable[headerFields.swInfo][headerFields.src_mac] = headerFields.inport;
    } else {
        if(lookupTable[headerFields.swInfo].count(headerFields.src_mac)<=0){
            lookupTable[headerFields.swInfo][headerFields.src_mac] = headerFields.inport;
        }
    }


    if(lookupTable.count(headerFields.swInfo)<=0){
        floodPacket(packet_in_msg);
    } else {
        if(lookupTable[headerFields.swInfo].count(headerFields.dst_mac)<=0){
            floodPacket(packet_in_msg);
        } else {
            uint32_t outport = lookupTable[headerFields.swInfo][headerFields.dst_mac];

            oxm_basic_match match = oxm_basic_match();
            match.dl_dst = headerFields.dst_mac;
            match.dl_type = headerFields.eth_type;
            match.dl_src = headerFields.src_mac;
            match.in_port = headerFields.inport;

            match.wildcards= 0;
            //TODO fix wildcards for OFP151!
#if OFP_VERSION_IN_USE == OFP_100
//            match.wildcards |= OFPFW_IN_PORT;
//            match.wildcards |= OFPFW_DL_TYPE;
//            match.wildcards |= OFPFW_DL_SRC;
//            match.wildcards |= OFPFW_DL_DST;
            match.wildcards |= OFPFW_DL_VLAN;
            match.wildcards |=  OFPFW_DL_VLAN_PCP;
            match.wildcards |= OFPFW_NW_PROTO;
            match.wildcards |= OFPFW_NW_SRC_ALL;
            match.wildcards |= OFPFW_NW_DST_ALL;
            match.wildcards |= OFPFW_TP_SRC;
            match.wildcards |= OFPFW_TP_DST;

#endif

            TcpSocket * socket = controller->findSocketFor(packet_in_msg);
            sendFlowModMessage(OFPFC_ADD, match, outport, socket,idleTimeout,hardTimeout);
            sendPacket(packet_in_msg, outport);
        }
    }
}


} /*end namespace ofp*/






