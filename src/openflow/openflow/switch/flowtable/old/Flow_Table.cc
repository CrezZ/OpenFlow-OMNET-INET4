#include <omnetpp.h>
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "openflow/openflow/switch/flowtable/old/Flow_Table.h"
#include <sstream>

using namespace std;
using namespace inet;

namespace ofp{

Flow_Table::Flow_Table() {

}

inline
static int flow_fields_match(const oxm_basic_match &m1, const oxm_basic_match &m2, const uint32_t w){
#if OFP_VERSION_IN_USE == OFP_100
    return ((w & OFPFW_IN_PORT) || m1.in_port == m2.in_port) &&
            ((w & OFPFW_DL_TYPE) || m1.dl_type == m2.dl_type ) &&
            ((w & OFPFW_DL_SRC) || !m1.dl_src.compareTo(m2.dl_src)) &&
            ((w & OFPFW_DL_DST) || !m1.dl_dst.compareTo(m2.dl_dst)) &&
            ((w & OFPFW_DL_VLAN) || m1.dl_vlan == m2.dl_vlan ) &&
            ((w & OFPFW_DL_VLAN_PCP) || m1.dl_vlan_pcp == m2.dl_vlan_pcp ) &&
            ((w & OFPFW_NW_PROTO) || m1.nw_proto == m2.nw_proto ) &&
            ((w & OFPFW_NW_SRC_ALL) || m1.nw_src.equals(m2.nw_src) ) &&
            ((w & OFPFW_NW_DST_ALL) || m1.nw_dst.equals(m2.nw_dst) ) &&
            ((w & OFPFW_TP_SRC) || m1.tp_src == m2.tp_src ) &&
            ((w & OFPFW_TP_DST) || m1.tp_src == m2.tp_src );
#else
    //TODO fix wildcards and matches for OFP151
    return false;
#endif

}

void Flow_Table::addEntry(Flow_Table_Entry entry) {
    entryList.push_front(entry);
}



Flow_Table_Entry* Flow_Table::lookup(oxm_basic_match &match) {
    EV << "Looking through " << entryList.size() << " Flow Entries!" << '\n';

    for(auto iter =entryList.begin();iter != entryList.end();++iter){

        //check if flow has expired
        if ((*iter).getIdleTimeout() != 0 && (*iter).getExpiresAt() < simTime()){
            iter = entryList.erase(iter);
            continue;
        }


        if (flow_fields_match(match, (*iter).getMatch(), (*iter).getMatch().wildcards)){
            //adapt idle timer filed if neccessary
            if ((*iter).getIdleTimeout() != 0){
                (*iter).setExpiresAt((*iter).getIdleTimeout()+simTime());
            }
            return &(*iter);
        }
    }
    return NULL;
}

std::string Flow_Table::exportToXML() {
    std::ostringstream oss;
    string tab = "    ";

    oss << "<flowTable>" << endl; // start flow table

    for(auto iter =entryList.begin();iter != entryList.end();++iter){
        // begin flow entry
        oss << tab << "<flowEntry";
        oss << " cookie=\"" << (*iter).getCookie() << "\"";//        uint64_t cookie;
        oss << " flags=\"" << (*iter).getFlags() << "\"";//        uint32_t flags;
        oss << " priority=\"" << (*iter).getPriority() << "\"";//        int priority;
        oss << " >" << endl;

        //        oxm_basic_match match;
        uint32_t w = (*iter).getMatch().wildcards;
        oss << tab << tab << "<match ";
        if(!(w & OFPFW_IN_PORT))
            oss << " in_port=\"" << (int)(*iter).getMatch().in_port << "\""; //uint16_t in_port;
        if(!(*iter).getMatch().dl_dst.isUnspecified() && !(w & OFPFW_DL_DST))
            oss << " dl_dst=\"" << (*iter).getMatch().dl_dst.str() << "\""; //inet::MACAddress dl_src;
        if(!(*iter).getMatch().dl_src.isUnspecified() && !(w & OFPFW_DL_SRC))
            oss << " dl_src=\"" << (*iter).getMatch().dl_src.str() << "\""; //inet::MACAddress dl_dst;
        if(!(w & OFPFW_DL_VLAN))
            oss << " dl_vlan=\"" << (int)(*iter).getMatch().dl_vlan << "\""; //uint16_t dl_vlan;
        if(!(w & OFPFW_DL_VLAN_PCP))
            oss << " dl_vlan_pcp=\"" << (int)(*iter).getMatch().dl_vlan_pcp << "\""; //uint8_t dl_vlan_pcp;
        if(!(w & OFPFW_DL_TYPE))
            oss << " dl_type=\"" << (int)(*iter).getMatch().dl_type << "\""; //uint16_t dl_type;
        if(!(w & OFPFW_NW_PROTO))
            oss << " nw_proto=\"" << (int)(*iter).getMatch().nw_proto << "\""; //uint8_t nw_proto;
        if(!(*iter).getMatch().nw_src.isUnspecified() && !(w & OFPFW_NW_SRC_ALL))
            oss << " nw_src=\"" << (*iter).getMatch().nw_src.str(false) << "\""; //inet::IPv4Address nw_src;
        if(!(*iter).getMatch().nw_dst.isUnspecified() && !(w & OFPFW_NW_DST_ALL))
            oss << " nw_dst=\"" << (*iter).getMatch().nw_dst.str(false) << "\""; //inet::IPv4Address nw_dst;
        if(!(w & OFPFW_TP_SRC))
            oss << " tp_src=\"" << (int)(*iter).getMatch().tp_src << "\""; //uint16_t tp_src;
        if(!(w & OFPFW_TP_DST))
            oss << " tp_dst=\"" << (int)(*iter).getMatch().tp_dst << "\""; //uint16_t tp_dst;

        oss << " wildcards=\"" << (*iter).getMatch().wildcards << "\""; //uint32_t wildcards;
        oss << " />" << endl;
        //        std::vector<ofp_action_output> instructions;
        if(!(*iter).getInstructionsVector().empty()){
            oss << tab << tab << "<instructions>" << endl;
            for (uint32_t i=0;i<(*iter).getInstructionsVector().size();i++){
                int port = (*iter).getInstructionsVector()[i].port;
                oss << tab << tab << tab << "<action_output port=\"" << port << "\"/>" << endl;
            }
            oss << tab << tab << "</instructions>" << endl;
        }

        // end flow entry
        oss << tab << "</flowEntry>" << endl;
    }

    oss << "</flowTable>" << endl;//end flow table
    return oss.str();
}

void Flow_Table::importFromXML(cXMLElement* xmlDoc) {
    cXMLElementList xmlFlows = xmlDoc->getChildrenByTagName("flowEntry");
    for(size_t i=0; i<xmlFlows.size(); i++){
        Flow_Table_Entry entry;
        //get attributes
        if(const char* value = xmlFlows[i]->getAttribute("cookie"))
            entry.setCookie(atoi(value));//cookie
        if(const char* value = xmlFlows[i]->getAttribute("flags"))
            entry.setFlags(atoi(value));//flags
        if(const char* value = xmlFlows[i]->getAttribute("priority"))
            entry.setPriority(atoi(value));//priority
        if(const char* value = xmlFlows[i]->getAttribute("idleTimeout"))
            entry.setIdleTimeout(atoi(value));//idle timeout
        if(const char* value = xmlFlows[i]->getAttribute("hardTimeout"))
            entry.setHardTimeout(atoi(value));//hard timeout

        //get match
        oxm_basic_match match;
        if(cXMLElement* xmlMatch = xmlFlows[i]->getFirstChildWithTag("match")){
            if(const char* value = xmlMatch->getAttribute("in_port"))
                match.in_port = atoi(value);
            if(const char* value = xmlMatch->getAttribute("dl_src"))
                match.dl_src = MacAddress(value);
            if(const char* value = xmlMatch->getAttribute("dl_dst"))
                match.dl_dst = MacAddress(value);
            if(const char* value = xmlMatch->getAttribute("dl_vlan"))
                match.dl_vlan = atoi(value);
            if(const char* value = xmlMatch->getAttribute("dl_vlan_pcp"))
                match.dl_vlan_pcp = atoi(value);
            if(const char* value = xmlMatch->getAttribute("dl_type"))
                match.dl_type = atoi(value);
            if(const char* value = xmlMatch->getAttribute("nw_proto"))
                match.nw_proto = atoi(value);
            if(const char* value = xmlMatch->getAttribute("nw_src"))
                match.nw_src = Ipv4Address(value);
            if(const char* value = xmlMatch->getAttribute("nw_dst"))
                match.nw_dst = Ipv4Address(value);
            if(const char* value = xmlMatch->getAttribute("tp_src"))
                match.tp_src = atoi(value);
            if(const char* value = xmlMatch->getAttribute("tp_dst"))
                match.tp_dst = atoi(value);
            if(const char* value = xmlMatch->getAttribute("wildcards"))
                match.wildcards = atoi(value);
        }
        entry.setMatch(match);

        //get instructions
        cXMLElement* xmlInstructions = xmlFlows[i]->getFirstChildWithTag("instructions");
        cXMLElementList xmlInstructionList = xmlInstructions->getChildrenByTagName("action_output");
        std::vector<ofp_action_output> instructions;
        for (size_t i=0; i<xmlInstructionList.size(); i++){
            ofp_action_output action;
            action.port = atoi(xmlInstructionList[i]->getAttribute("port"));
            instructions.push_back(action);
        }
        entry.setInstructionsVector(instructions);

        entryList.push_back(entry);
    }
}

} /*end namespace ofp*/


