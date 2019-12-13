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
    unsigned long errorX= 0;            // [0 : 0.5]    position error
    unsigned long errorM= 0;            // (0.5 ; 1.0]
    unsigned long errorL= 0;            // ( > 1.0)

    unsigned long msgCount = 0;
    double errorMid = 0.0;
    unsigned long stepCount= 0;
    unsigned long pedCount = 0;

    double vruSendInterval; //the intervall between 2 messages
    double keepInterval;    //time to keep ped data on lists

    omnetpp::simtime_t lastSendTime=0;        // time when send last VRU msg
    std::vector<Pedestrian> observedPed; // list of instant observed pedestrians by camera/radar
    std::vector<Pedestrian> receivedPed; // list of instant observed pedestrians by VRU detect service

    bool inSimulation(std::string pedId); // check if pedestrian still presented on simulation

    // my class vars
    const traci::VehicleController* mVehicleController = nullptr;
    artery::LocalEnvironmentModel* mLocalEnvironmentModel;
    std::string mEgoId;



    /* detect pedestrians from all enviroupment objects */
    void detectPedestrians (const artery::TrackedObjectsFilterRange& objs);
    void sendPedestrianInfo();                                          // send observed ped list on determined time
    void sendPedestrianInfo(std::vector<Pedestrian> observedPed);       // send list of instant observed ped
    void sendPedestrianInfo(Pedestrian ped);                            // send one ped
    void makeCalc();                                                    // make calculations of tracked values
    void clearObserved();                                               // remove pedestrians out of camera range


};



#endif /* VruDetectService_H_ */
