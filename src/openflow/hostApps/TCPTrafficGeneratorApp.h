

#ifndef OPENFLOW_HOSTAPPS_TCP_TRAFFIC_GENERATOR_APP_H_
#define OPENFLOW_HOSTAPPS_TCP_TRAFFIC_GENERATOR_APP_H_

#include "inet/common/INETDefs.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "inet/transportlayer/contract/tcp/TCPSocket.h"
#include "inet/networklayer/common/L3Address.h"

using namespace std;
using namespace inet;

namespace ofp{

/**
 * Single-connection TCP application.
 */

struct Stats{
    SimTime connectionStarted;
    SimTime connectionEstablished;
    SimTime connectionFinished;
    long transmittedBytes;
};

class TCPTrafficGeneratorApp : public omnetpp::cSimpleModule, public virtual TcpSocket::ICallback
{

  protected:
    omnetpp::cTopology topo;
    int  lineNumbers;
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    unsigned int FileRead( istream & is, vector <char> & buff );
    unsigned int CountLines( const vector <char> & buff, int sz );
    virtual void socketDataArrived(int connId, void *yourPtr, cPacket *msg, bool urgent) ;
    virtual void socketEstablished(int connId, void *yourPtr) ;
    virtual void socketPeerClosed(int connId, void *yourPtr) ;
    virtual void socketClosed(int connId, void *yourPtr) ;
    virtual void socketFailure(int connId, void *yourPtr, int code) ;
    virtual void socketStatusArrived(int connId, void *yourPtr, TcpStatusInfo *status) ;
    std::map<TcpSocket *,Stats> statistics;


    //stats
    simsignal_t connectionFinished;
    simsignal_t connectionStarted;
    simsignal_t connectionEstablished;
    simsignal_t transmittedBytes;


};

} /*end namespace ofp*/

#endif /** OPENFLOW_HOSTAPPS_TCP_TRAFFIC_GENERATOR_APP_H_**/

