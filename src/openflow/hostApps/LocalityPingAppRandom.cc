
#ifndef LACALITYPINGAPPRANDOM_H_
#define LACALITYPINGAPPRANDOM_H_

#include <iostream>

#include "openflow/hostApps/LocalityPingAppRandom.h"

#include "inet/networklayer/common/L3AddressResolver.h"
#include "inet/applications/pingapp/PingApp_m.h"
//#include "inet/networklayer/contract/ipv4/Ipv4ControlInfo.h"
//#include "inet/networklayer/contract/ipv6/Ipv6ControlInfo.h"

using namespace std;

namespace ofp{

Define_Module(LocalityPingAppRandom);


void LocalityPingAppRandom::initialize(int stage){
    if(stage == INITSTAGE_LOCAL){
        //set params
        localityRelation = par("localityRelation");

        //read in config
        std::string path = par("pathToGroupConfigFile");
        ifstream ifs( path.c_str() );
        if(ifs){
            string node="";
            string group="";
            string line="";
            while(!ifs.eof()){
                getline(ifs, line);

                node = (cStringTokenizer(line.c_str(),";").asVector())[0];
                group = (cStringTokenizer(line.c_str(),";").asVector())[1];

                //check if the node is me
                if(strstr(getParentModule()->getFullPath().c_str(),node.c_str())!=NULL){
                    localId = group;
                } else {
                    if(groupToNodes.count(group)== 0){
                        groupToNodes[group] = std::vector<std::string>();
                        groupToNodes[group].push_back(node);
                    } else {
                        groupToNodes[group].push_back(node);
                    }
                }
            }
        } else {
            EV << "File not found!" << endl;
        }
        ifs.close();

    }
    PingApp::initialize(stage);
}

void LocalityPingAppRandom::handleMessage(cMessage *msg){

/*    if (!isNodeUp()){
        if (msg->isSelfMessage())
            throw cRuntimeError("Application is not running");
        delete msg;
        return;
    }*/
    if (msg == timer){
        //determine local oder global
        std::string tempTarget = "";
        if(dblrand() <= localityRelation){
            //local so set target
            tempTarget = localId;

        } else {
            //global so set target
            //determine group
            if(groupToNodes.size() > 0) {
                int tempRand = intrand(groupToNodes.size());
                while(tempRand == atoi(localId.c_str()) && (!localId.empty())){
                    tempRand = intrand(groupToNodes.size());
                }
                tempTarget = std::to_string(tempRand);
            }
        }

        //determine target
        bool targetSet = false;
        if(!tempTarget.empty()){
            std::vector<std::string> tempVec = groupToNodes[tempTarget];
            if( 0 < tempVec.size()){
                std::string tempAddress = tempVec[intrand(tempVec.size())];
                if(!tempAddress.empty()){
                    connectAddress = (tempAddress);
                    targetSet = true;
                }
            }
        }

        if(targetSet){
            destAddr = L3AddressResolver().resolve(connectAddress.c_str());
            ASSERT(!destAddr.isUnspecified());
            srcAddr = L3AddressResolver().resolve(par("srcAddr"));
            EV << "Starting up: dest=" << destAddr << "  src=" << srcAddr << "\n";

            sendPingRequest();
        }

        if (isEnabled())
            scheduleNextPingRequest(simTime(), true);

    }
    else {

       // processPingResponse(omnetpp::check_and_cast<inet::Packet *>(msg));
        //TODO:: processPingResponse(int identifier, int seqNumber, Packet *packet);
    }

    if (hasGUI()){
        char buf[40];
        sprintf(buf, "sent: %ld pks\nrcvd: %ld pks", sentCount, numPongs);
        getDisplayString().setTagArg("t", 0, buf);
    }
}


bool LocalityPingAppRandom::isEnabled(){
    return (count == -1 || sentCount < count);
}

} /*end namespace ofp*/

#endif /* LACALITYPINGAPPRANDOM_H_ */
