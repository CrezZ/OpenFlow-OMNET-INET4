#include <omnetpp.h>
#include "inet/linklayer/ethernet/EtherFrame_m.h"
#include "inet/linklayer/common/MACAddress.h"
#include "inet/networklayer/arp/ipv4/ARPPacket_m.h"
#include "Flow_Table_Entry.h"

using namespace std;

namespace ofp{

Flow_Table_Entry::Flow_Table_Entry(){
    idleTimeout = 0.;
    hardTimeout = 0.;
}

Flow_Table_Entry::Flow_Table_Entry(OFP_Flow_Mod *flowModMsg){
    match = flowModMsg->getMatch();

    for(unsigned int i = 0; i < flowModMsg->getActionsArraySize(); i++){
        instructions.push_back(flowModMsg->getActions(i));
    }
    cookie = flowModMsg->getCookie();
    flags = flowModMsg->getFlags();
    priority = flowModMsg->getPriority();
    hardTimeout = flowModMsg->getHard_timeout();
    idleTimeout = flowModMsg->getIdle_timeout();

    if(idleTimeout != 0){
        expiresAt = idleTimeout+omnetpp::simTime();
    } else {
        expiresAt = hardTimeout+omnetpp::simTime();
    }
}

uint64_t Flow_Table_Entry::getCookie() const{
    return cookie;
}

flow_table_counters Flow_Table_Entry::getCounters() const{
    return counters;
}

uint16_t Flow_Table_Entry::getFlags() const{
    return flags;
}

double Flow_Table_Entry::getHardTimeout() const{
    return hardTimeout;
}

double Flow_Table_Entry::getIdleTimeout() const{
    return idleTimeout;
}

inet::SimTime Flow_Table_Entry::getExpiresAt() const{
    return expiresAt;
}

ofp_action_output Flow_Table_Entry::getInstructions() const{
    return instructions[0];
}

const std::vector<ofp_action_output>& Flow_Table_Entry::getInstructionsVector() const{
    return instructions;
}

oxm_basic_match Flow_Table_Entry::getMatch() const{
    return match;
}

int Flow_Table_Entry::getPriority() const{
    return priority;
}

void Flow_Table_Entry::setCookie(uint64_t cookie){
    this->cookie = cookie;
}

void Flow_Table_Entry::setCounters(flow_table_counters counters){
    this->counters = counters;
}

void Flow_Table_Entry::setFlags(uint16_t flags){
    this->flags = flags;
}

void Flow_Table_Entry::setHardTimeout(double hardTimeout){
    this->hardTimeout = hardTimeout;
}

void Flow_Table_Entry::setIdleTimeout(double idleTimeout){
    this->idleTimeout = idleTimeout;
}

void Flow_Table_Entry::setExpiresAt(inet::SimTime expiresAt){
    this->expiresAt = expiresAt;
}

void Flow_Table_Entry::setInstructions(ofp_action_output instructions[1]){
    this->instructions[0] = instructions[0];
}

void Flow_Table_Entry::setMatch(oxm_basic_match match){
    this->match = match;
}

void Flow_Table_Entry::setPriority(int priority){
    this->priority = priority;
}

} /*end namespace ofp*/

void ofp::Flow_Table_Entry::setInstructionsVector(
        std::vector<ofp_action_output> instructions) {
    this->instructions = instructions;
}
