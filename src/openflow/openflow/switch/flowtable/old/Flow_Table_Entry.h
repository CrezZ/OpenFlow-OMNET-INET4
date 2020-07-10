

#ifndef OPENFLOW_OPENFLOW_SWITCH_FLOW_TABLE_ENTRY_H_
#define OPENFLOW_OPENFLOW_SWITCH_FLOW_TABLE_ENTRY_H_

#include <openflow/openflow/protocol/OpenFlow.h>
#include "openflow/messages/openflowprotocol/OFP_Flow_Mod.h"
#include <vector>


namespace ofp{

struct flow_table_counters {

};

struct ltmatch{
    bool operator() (const oxm_basic_match m1, const oxm_basic_match m2) const{
        if(m1.in_port == m2.in_port){
            return false;
        }else if(m1.dl_type == m2.dl_type){
            return false;
        }else if(m1.dl_src == m2.dl_src){
            return false;
        }else if(m1.dl_dst == m2.dl_dst){
            return false;
        }else if (m1.wildcards != m2.wildcards){
            return false;
        }else{
            return true;
        }
    }
};


class Flow_Table_Entry {
    public:
        Flow_Table_Entry();
        Flow_Table_Entry(OFP_Flow_Mod *flowModMsg);

        uint64_t getCookie() const;
        flow_table_counters getCounters() const;
        uint16_t getFlags() const;
        double getHardTimeout() const;
        double getIdleTimeout() const;
        inet::SimTime getExpiresAt() const;
        ofp_action_output getInstructions() const;
        const std::vector<ofp_action_output>& getInstructionsVector() const;
        oxm_basic_match getMatch() const;
        int getPriority() const;

        void setCookie(uint64_t cookie);
        void setCounters(flow_table_counters counters);
        void setFlags(uint16_t flags);
        void setHardTimeout(double hardTimeout);
        void setIdleTimeout(double idleTimeout);
        void setExpiresAt(inet::SimTime expiresAt);
        void setInstructions(ofp_action_output instructions[1]);
        void setInstructionsVector(std::vector<ofp_action_output> instructions);
        void setMatch(oxm_basic_match match);
        void setPriority(int priority);

    protected:
        oxm_basic_match match;
        flow_table_counters counters;
        std::vector<ofp_action_output> instructions;
        uint64_t cookie;
        uint32_t flags;
        int priority;
        double idleTimeout;
        double hardTimeout;
        inet::SimTime expiresAt;
};

} /*end namespace ofp*/

#endif /* OPENFLOW_OPENFLOW_SWITCH_FLOW_TABLE_ENTRY_H_ */
