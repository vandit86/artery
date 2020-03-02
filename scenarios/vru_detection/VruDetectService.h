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
    double errorX= 0.0;            // [0 : 0.5]    position error
    double errorM= 0.0;            // (0.5 ; 1.0]
    double errorL= 0.0;            // ( > 1.0)
    double errorGlobal = 0.0;       // total error value

    double pedByMsgCount = 0.0;
    double effectivness = 0.0;      // is all ped in interest zone is detected
    unsigned long stepCount= 0;
    unsigned long stepCount1= 0;
    unsigned long stepCount2= 1;
    unsigned long pedCount = 0;         // num ped detected ped step

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
    void removeUntracked(omnetpp::simtime_t time);
    void detectPedestrians (const artery::TrackedObjectsFilterRange& objs);
    void sendPedestrianInfo();                                          // send observed ped list on determined time
    void sendPedestrianInfo(std::vector<Pedestrian> observedPed);       // send list of instant observed ped
    void sendPedestrianInfo(Pedestrian ped);                            // send one ped
    void removeDuplicate();                                          // make calculations of tracked values
    void clearReceived();                                               // remove pedestrians out of camera range
    void checkEffect();
    void checkError();
    std::vector<std::string> checkInZone(std::vector<Pedestrian> ped);  // get ped that on important zone
    std::vector<std::string> checkInZone(std::vector<std::string> ped);

};



#endif /* VruDetectService_H_ */
