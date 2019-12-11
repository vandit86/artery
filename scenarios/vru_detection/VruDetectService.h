#ifndef VruDetectService_H_
#define VruDetectService_H_

#include "artery/application/ItsG5Service.h"
#include "artery/envmod/LocalEnvironmentModel.h"
#include "Pedestrian.h"


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
    void finish() override;
//    void handleMessage(omnetpp::cMessage*) override;

private:

    unsigned long totalDetectPed = 0;
    double_t totalMeasurments = 0;

    unsigned int instantDetectPedNum = 0; // instant number of detected pedestrian by msg
    double vruSendInterval;
    omnetpp::simtime_t lastSendTime=0;        // time when send last VRU msg
    omnetpp::simtime_t lastReceivedTime=0;    // time when receive last VRU message
    std::vector<Pedestrian> pedIds; // list of detected pedestrians

    bool inSimulation(std::string pedId); // check if pedestrian still presented on simulation

    // my class vars
    const traci::VehicleController* mVehicleController = nullptr;
    artery::LocalEnvironmentModel* mLocalEnvironmentModel;
    std::string mEgoId;



    /* detect pedestrians from all enviroupment objects */
    void detectPedestrians (const artery::TrackedObjectsFilterRange& objs);
    void sendPedestrianInfo(std::vector<Pedestrian> pedIds);    // groupe ped
    void sendPedestrianInfo(Pedestrian ped);    // send one ped



};



#endif /* VruDetectService_H_ */
