#include "VruDetectService.h"
#include <vru_msgs/DetectVRU_m.h>
#include "artery/traci/VehicleController.h"
#include "artery/envmod/EnvironmentModelObject.h"
#include "artery/envmod/LocalEnvironmentModel.h"
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>
#include <boost/units/io.hpp>
#include <math.h>       /* sqrt */

using namespace omnetpp;
using namespace vanetza;
using namespace artery;

static const simsignal_t scSignalCamReceived = cComponent::registerSignal("CamReceived");

Define_Module(VruDetectService)

void VruDetectService::initialize()
{
    ItsG5BaseService::initialize();
    //read parameters from config file
    vruSendInterval = par("vruSendInterval").doubleValue();
    keepInterval = par("keepInterval").doubleValue();
    lastSendTime = simTime();
    subscribe(scSignalCamReceived);
    mLocalEnvironmentModel = getFacilities().get_mutable_ptr<LocalEnvironmentModel>();
    mEgoId = getFacilities().get_const<traci::VehicleController>().getVehicleId();
    mVehicleController = &getFacilities().get_const<traci::VehicleController>();
}

// indicate packege
void VruDetectService::indicate(const btp::DataIndication& ind, omnetpp::cPacket* packet)
{
    Enter_Method("indicate");
    auto detectVruMessage = check_and_cast<const DetectVRU*>(packet);

    //GET REAL POSITION and Compare with received one
    for (int i = 0 ; i < detectVruMessage->getPedIdsArraySize() ; i++){
        const Pedestrian& ped = detectVruMessage->getPedIds(i);
        std::string pedId = ped.pedId;
        if (inSimulation(pedId)){
            // try to find pedestrian on received ped list
            auto pedIt = find_if(receivedPed.begin(), receivedPed.end(), [&pedId](const Pedestrian& obj) {return obj.pedId == pedId;});
            if (pedIt == receivedPed.end())                 // if ped not in list then add to the list
            {
                receivedPed.push_back(ped);                // save to list new ped
            }
            else {                                              // update if ped on the list (recently observed)
                if ((*pedIt).updated < ped.updated){            // and we receive more recent information for this ped
                    (*pedIt).updated = ped.updated;             // update time
                    (*pedIt).x = ped.x;                         // position x
                    (*pedIt).y = ped.y;                         // position y
                }
            }
        }
    }
    pedByMsgCount+=detectVruMessage->getPedIdsArraySize(); // increase by number of received ped info
    delete(detectVruMessage);
}

// called every 0.1s
void VruDetectService::trigger()
{
    Enter_Method("trigger");
    auto& allObjects = mLocalEnvironmentModel->allObjects();        // get all objects from env
    detectPedestrians(filterBySensorCategory(allObjects,"Radar"));  // detect ped in camera field of view
    removeDuplicate();                                              // erase repeated ped from received list
    clearReceived();                                                // remove ped out of msg range
    checkError();
    checkEffect();                                                  // calculate effectivness
    sendPedestrianInfo();                                           // send message to other vehicles
    stepCount++;
}

// select from radar data pedestrians and broadcast this info
void VruDetectService::detectPedestrians (const artery::TrackedObjectsFilterRange& objs){
    auto& ped_api = mVehicleController->getLiteAPI().person();
    omnetpp::simtime_t upTime = simTime();
    bool isNewPed = false ;
    for(const auto &obj : objs ){
        std::weak_ptr<EnvironmentModelObject> obj_ptr = obj.first;
        if (obj_ptr.expired()) continue; /*< objects remain in tracking briefly after leaving simulation */
        std::string pedId = obj_ptr.lock()->getExternalId();

        // if object is a pedestrian
        if ( pedId.rfind("p",0) == 0 ){                         // TODO create isPedestrian functio
            if (inSimulation(pedId)){                           // if ped is on simulation
                // try to find pedestrian on observed ped list
                auto pedIt = find_if(observedPed.begin(), observedPed.end(), [&pedId](const Pedestrian& obj) {return obj.pedId == pedId;});
                if (pedIt == observedPed.end())                 // if ped not in list then add to the list
                {
                    Pedestrian ped;
                    ped.pedId = pedId;
                    ped.x = ped_api.getPosition(pedId).x;       // position x
                    ped.y = ped_api.getPosition(pedId).y;       // position y
                    ped.updated = upTime ;                      // time
                    observedPed.push_back(ped);                 // insert to list
                    isNewPed = true ;
                }
                else {                                              // update if ped on the list (recently observed)
                    (*pedIt).updated = upTime ;                     // update time
                    (*pedIt).x = ped_api.getPosition(pedId).x;      // position x
                    (*pedIt).y = ped_api.getPosition(pedId).y;      // position y
                }

//                std::cout << pedId<< "nexet edge " << ped_api.getNextEdge(pedId)<<  std::endl;
//                ped_api.getPosition(pedId);

            }
        }
    }
    removeUntracked(upTime);    // remove ped if out of camera range (the ped that was not updated on this step)
    if (isNewPed){              // if new ped in camera range
        lastSendTime = 0;       // force to send immidealty
    }
}

// create logic to select moment when to send the Detect VRU packege
void VruDetectService::sendPedestrianInfo()
{
    if (observedPed.size()>0){
        if (simTime()- lastSendTime > vruSendInterval){
            lastSendTime = simTime();
            sendPedestrianInfo(observedPed);
        }
    }
}

// send info about detected pedestrian
void VruDetectService::sendPedestrianInfo(std::vector<Pedestrian> pedIds)
{
    btp::DataRequestB req;
    req.destination_port = host_cast<VruDetectService::port_type>(getPortNumber());
    req.gn.transport_type = geonet::TransportType::SHB;
    req.gn.traffic_class.tc_id(static_cast<unsigned>(dcc::Profile::DP3));
    req.gn.communication_profile = geonet::CommunicationProfile::ITS_G5;

//    create Detect VRU packet
    auto packet = new DetectVRU();
    packet->setVehId(mEgoId.c_str());
    packet->setPedIdsArraySize(pedIds.size());
    int i = 0;
    for (i = 0; i < pedIds.size(); i++)
        packet->setPedIds(i,pedIds[i]);
    packet->setByteLength(20*i);  // size proportional to number of pedestrians
    request(req, packet);
}

// send just one
void VruDetectService::sendPedestrianInfo(Pedestrian ped)
{
    std::vector<Pedestrian> pedV;
    pedV.push_back(ped);
    sendPedestrianInfo(pedV);
}

// if receive Info about observed ped do not concider them
// this way we consider only once simyltaneosly detected ped,
// and camera detected has more weight, removing the ped from received list
void VruDetectService::removeDuplicate()
{
    // not include repited ped from received list
    for (auto it = observedPed.begin(); it != observedPed.end(); it++)
        for (auto itR = receivedPed.begin(); itR != receivedPed.end(); itR++){
            if ( (*it).pedId == (*itR).pedId )  receivedPed.erase(itR--);
        }
    pedCount += (observedPed.size() + receivedPed.size());  // num detected ped step

}

// remove ped that is not receive msg about tham anymore durring (1s)
void VruDetectService::clearReceived()
{
    int ped = 0;
    omnetpp::simtime_t updateInterval = omnetpp::SimTime(keepInterval);
    auto it = std::remove_if(receivedPed.begin(), receivedPed.end(), [&updateInterval](const Pedestrian& item)
    { return (simTime() - item.updated > updateInterval); });
    auto k = it;
    for (k = it; k!=receivedPed.end(); k++) ped++;
    receivedPed.erase(it, receivedPed.end());

    // update number of detected ped
    totalDetectPed+=ped;

//    std::cout<< mEgoId << " observed : ";
//    for (auto it = observedPed.begin(); it != observedPed.end(); it++)
//        std::cout << (*it).pedId << " ";
//    std::cout<<std::endl;
//    std::cout<<mEgoId<<" received : ";
//    for (auto it = receivedPed.begin(); it != receivedPed.end(); it++)
//        std::cout << (*it).pedId << " ";
//    std::cout<<std::endl;
}

void VruDetectService::checkEffect()
{
    //  std::cout << "pos1 "<< mVehicleController->getSpeed().value()<< std::endl;
    //    std::cout << "pos2 "<< mVehicleController->getLiteAPI().vehicle().getPosition(mEgoId).getString()<< std::endl;
    //    std::cout << "angle "<< mVehicleController->getLiteAPI().vehicle().getAngle(mEgoId)<< std::endl;
    std::vector<std::string> inObserved = checkInZone(observedPed);
    std::vector<std::string> inReceived = checkInZone(receivedPed);
    inObserved.insert(inObserved.end(), inReceived.begin(),inReceived.end()); // join

    // calculate effectivness just when pedestrian detected
    auto& ped_api = mVehicleController->getLiteAPI().person();
    std::vector<std::string> persons = checkInZone (ped_api.getIDList());       // get persons in zone
    if (persons.size() > 0 ){
        double size = 0;
        for (auto p = persons.begin(); p != persons.end(); p++){
            if ( std::find(inObserved.begin(), inObserved.end(), *p) == inObserved.end()){
                size++;
            }
        }
        stepCount1++;
        effectivness += (persons.size()-size)/persons.size();
    }
}

// check error about received ped on every step
void VruDetectService::checkError()
{
    auto& ped_api = mVehicleController->getLiteAPI().person();
    for (auto itR = receivedPed.begin(); itR != receivedPed.end(); itR++){
        if (inSimulation(itR->pedId)){
            double rPosx = ped_api.getPosition(itR->pedId).x ; // actual position of ped
            double rPosy = ped_api.getPosition(itR->pedId).y ;
            // get position error TODO:: check if  ped or (*pedIt)
            double error = (double) sqrt ( pow((rPosx-(itR->x)),2) + pow ((rPosy-(itR->y)),2)); // error distance
            if (error > 1.0) errorL++;
            else if (error > 0.5 && error <=1.0) errorM++;
            else errorX ++;
            errorGlobal+=error;
            stepCount2++;

            if (largerError<error) largerError = error; // save the larger error value
        }
    }
}

std::vector<std::string> VruDetectService::checkInZone(std::vector<Pedestrian> ped)
{
    std::vector<std::string> in; // result vectror
    double x = mVehicleController->getLiteAPI().vehicle().getPosition(mEgoId).x; // veh position
    double y = mVehicleController->getLiteAPI().vehicle().getPosition(mEgoId).y;
    // for all ped check if in range
    for (auto it = ped.begin(); it != ped.end(); it++){
        double dist = (double) sqrt ( pow((x-(it->x)),2) + pow ((y-(it->y)),2) ); // distance
        double radius = mVehicleController->getSpeed().value() * 2; // 3*speed(m/s)
        if (dist < radius){
            in.push_back(it->pedId);
        }
    }
    return in;
}

std::vector<std::string> VruDetectService::checkInZone(std::vector<std::string> ped)
{
    std::vector<std::string> in; // result vectror
    auto& ped_api = mVehicleController->getLiteAPI().person();
    double x = mVehicleController->getLiteAPI().vehicle().getPosition(mEgoId).x; // veh position
    double y = mVehicleController->getLiteAPI().vehicle().getPosition(mEgoId).y;
    // for all ped check if in range
    for (auto it = ped.begin(); it != ped.end(); it++){
        if (!inSimulation(*it)) continue;
        double rPosx = ped_api.getPosition(*it).x ; // actual position of ped
        double rPosy = ped_api.getPosition(*it).y ;
        double dist = (double) sqrt ( pow((x-rPosx),2) + pow ((y-rPosy),2)); // distance
        double radius = mVehicleController->getSpeed().value() * 2; // 3*speed(m/s)
        if (dist < radius){
            in.push_back(*it);
        }
    }
    return in;
}

void VruDetectService::finish()
{
    // you could record some scalars at this point
    ItsG5Service::finish();

    // add to tottal detected pedestrians those that in buffer
    totalDetectPed+=observedPed.size();
    totalDetectPed+=receivedPed.size();

    std::cout << "FINISH "<< mEgoId
              << " totPed: " << totalDetectPed
              << " pedByMsgCount: " << pedByMsgCount
              << " X:"<< errorX/stepCount2
              << " M:"<< errorM/stepCount2
              << " L:"<< errorL/stepCount2
              << " midErr: " << errorGlobal/stepCount2
              << " largerErr: " << largerError
              << " midPed: " << ((double)pedCount)/stepCount
              << " effect: " << effectivness/stepCount1
              << std::endl;
}

//if pedestrian still in simulation
bool VruDetectService::inSimulation(std::string pedId)
{
    auto& ped_api = mVehicleController->getLiteAPI().person();
    std::vector<std::string> persons = ped_api.getIDList();  // get vectror of persons in simulation
    if (std::find(persons.begin(), persons.end(), pedId) != persons.end())
        return true ;
    else return false;
}

// remove ped out of camera renge
void VruDetectService::removeUntracked(simtime_t time)
{
    for (auto it = observedPed.begin(); it != observedPed.end(); it++){
        if ( (*it).updated < time ) {
            observedPed.erase(it--);
            totalDetectPed++;   // update number of detected ped
        }
    }
}


// receive signal
void VruDetectService::receiveSignal(cComponent* source, simsignal_t signal, cObject* object, cObject* obj_2)
{
    Enter_Method("receiveSignal");

//    if (signal == scSignalCamReceived) {
//        // auto& vehicle = getFacilities().get_const<traci::VehicleController>();
//        EV_INFO << "Vehicle " << mVehicleController->getVehicleId() << " received a CAM in sibling serivce\n";
//        //std::cout << "Vehicle " << mVehicleController->getVehicleId() << " received a CAM in sibling serivce at: ";
//        if (auto cam = dynamic_cast<CaObject*>(object)) {
//            const auto id = cam->asn1()->header.stationID;
//            const auto latitude = cam->asn1()->cam.camParameters.basicContainer.referencePosition.latitude;
//            const auto longitude = cam->asn1()->cam.camParameters.basicContainer.referencePosition.longitude;
//            std::cout << simTime()<< "::id ="<< id << " lat ="<<latitude<< "lon ="<< longitude << std::endl;
//        }
//    }
}
