#ifndef VruDetectService_H_
#define VruDetectService_H_

#include "artery/application/ItsG5Service.h"
#include "artery/envmod/LocalEnvironmentModel.h"

// forward declaration
namespace traci { class VehicleController; }


class VruDetectService : public artery::ItsG5Service
{
public:
    
    void receiveSignal(omnetpp::cComponent*, omnetpp::simsignal_t, omnetpp::cObject*, omnetpp::cObject*) override;
    
protected:
    void indicate(const vanetza::btp::DataIndication&, omnetpp::cPacket*) override;
    // METHOD CALLED EVERY 0.1s (DEFINED ON OMNET.INI) middelware
    void trigger() override;
    void initialize() override;
//    void finish() override;
//    void handleMessage(omnetpp::cMessage*) override;

private:
    // from example service
    const traci::VehicleController* mVehicleController = nullptr;
    // based on envmod printer
    artery::LocalEnvironmentModel* mLocalEnvironmentModel;
    std::string mEgoId;
    /* detect pedestrians from all enviroupment objects */
    void detectPedestrians (const artery::TrackedObjectsFilterRange& objs);
    void sendPedestrianInfo(std::string egoId, std::vector<std::string> pedIds);

};



#endif /* VruDetectService_H_ */
