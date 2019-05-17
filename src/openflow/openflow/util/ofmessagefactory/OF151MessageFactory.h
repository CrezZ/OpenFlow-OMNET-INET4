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


#ifndef OPENFLOW_OPENFLOW_UTIL_OF151MESSAGEFACTORY_H_
#define OPENFLOW_OPENFLOW_UTIL_OF151MESSAGEFACTORY_H_

#include "openflow/openflow/util/ofmessagefactory/AOFMessageFactory.h"

namespace ofp {

/**
 * Implementing the openflow message factory interface for OpenFlow Version 1.5.1
 * TODO implement!!!!
 *
 * @author Timo Haeckel, for HAW Hamburg
 */
class OF151MessageFactory: public AOFMessageFactory {
public:
    OF151MessageFactory();
    virtual ~OF151MessageFactory();

    virtual OFP_Features_Reply* createFeaturesReply(std::string dpid, uint32_t n_buffers, uint8_t n_tables, uint32_t capabilities, uint32_t n_ports = 0);
    virtual OFP_Features_Request* createFeatureRequest();
    virtual OFP_Flow_Mod* createFlowModMessage(ofp_flow_mod_command mod_com,const oxm_basic_match& match, int pritority, int* outports, int n_outports, uint32_t idleTimeOut=1 , uint32_t hardTimeOut=0);
    virtual OFP_Hello* createHello();
    virtual OFP_Packet_In* createPacketIn(ofp_packet_in_reason reason, inet::EthernetIIFrame *frame, uint32_t buffer_id = OFP_NO_BUFFER, bool sendFullFrame = true);
    virtual OFP_Packet_Out* createPacketOut(uint32_t* outports, int n_outports, int in_port, uint32_t buffer_id = OFP_NO_BUFFER, inet::EthernetIIFrame *frame = nullptr);
};

} /* namespace ofp */

#endif /* OPENFLOW_OPENFLOW_UTIL_OF151MESSAGEFACTORY_H_ */
