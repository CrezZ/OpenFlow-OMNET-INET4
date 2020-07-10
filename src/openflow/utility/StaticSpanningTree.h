

#ifndef OPENFLOW_UTILITY_STATICSPANNINGTREE_H_
#define OPENFLOW_UTILITY_STATICSPANNINGTREE_H_

#include <omnetpp.h>
#include "openflow/openflow/switch/engine/OF_Switch.h"

namespace ofp{

class StaticSpanningTree : public omnetpp::cSimpleModule
{
protected:
    struct NodeInfo {
            NodeInfo() {isInTree=false;isProcessed=false;}
            bool isInTree;
            bool isProcessed;
            int moduleID;
            std::vector<int> ports;
            std::vector<int> treeNeighbors;
        };

        typedef std::vector<NodeInfo> NodeInfoVector;
        omnetpp::cTopology topo_spanntree;
        NodeInfoVector nodeInfo;

        virtual int numInitStages() const  {return 5;}
        virtual void initialize(int stage);
        virtual void handleMessage(omnetpp::cMessage *msg);
};

} /*end namespace ofp*/


#endif /* OPENFLOW_UTILITY_STATICSPANNINGTREE_H_ */
