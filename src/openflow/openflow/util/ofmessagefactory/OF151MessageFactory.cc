//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
//
// c Timo Haeckel, for HAW Hamburg
//


#include "openflow/openflow/util/ofmessagefactory/OF151MessageFactory.h"
#include <openflow/openflow/protocol/OpenFlow.h>
#include <openflow/messages/openflowprotocol/OFP_Features_Reply.h>
#include <openflow/messages/openflowprotocol/OFP_Features_Request.h>

#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "inet/linklayer/ethernet/EtherFrame_m.h"

using namespace inet;

namespace ofp {

OF151MessageFactory::OF151MessageFactory() {
    // TODO Auto-generated constructor stub

}

OF151MessageFactory::~OF151MessageFactory() {
    // TODO Auto-generated destructor stub
}

OFP_Features_Reply* OF151MessageFactory::createFeaturesReply(std::string dpid,
        uint32_t n_buffers, uint8_t n_tables, uint32_t capabilities,
        uint32_t n_ports) {

    //TODO use OFP151 Message

    //get new message
    OFP_Features_Reply* msg = new OFP_Features_Reply("FeaturesReply");

    //set ofp header
    msg->getHeader().version = OFP_VERSION;
    msg->getHeader().type = OFPT_FEATURES_REPLY;

    //set data fields
    msg->setDatapath_id(dpid.c_str());
    msg->setN_buffers(n_buffers);
    msg->setN_tables(n_tables);
    msg->setCapabilities(capabilities);
    msg->setPortsArraySize(n_ports);

    // set message params
    msg->setByteLength(32);

    return msg;
}

OFP_Features_Request* OF151MessageFactory::createFeatureRequest() {

    OFP_Features_Request *featuresRequest = new OFP_Features_Request("FeaturesRequest");
    featuresRequest->getHeader().version = OFP_VERSION;
    featuresRequest->getHeader().type = OFPT_FEATURES_REQUEST;
    featuresRequest->setByteLength(8);

    return featuresRequest;
}

OFP_Flow_Mod* OF151MessageFactory::createFlowModMessage(ofp_flow_mod_command mod_com,const oxm_basic_match& match, int pritority, int* outports, int n_outports, uint32_t idleTimeOut, uint32_t hardTimeOut) {
    return nullptr;
}

OFP_Hello* OF151MessageFactory::createHello() {
    return nullptr;
}

OFP_Packet_In* OF151MessageFactory::createPacketIn(ofp_packet_in_reason reason, Packet *packet, uint32_t buffer_id, bool sendFullFrame) {
    return nullptr;
}

OFP_Packet_Out* OF151MessageFactory::createPacketOut(uint32_t* outports, int n_outports, int in_port, uint32_t buffer_id, Packet *packet) {
    return nullptr;
}

} /* namespace ofp */
