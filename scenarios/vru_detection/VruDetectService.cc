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
    //subscribe(scSignalCamReceived);
    mLocalEnvironmentModel = getFacilities().get_mutable_ptr<LocalEnvironmentModel>();
    mEgoId = getFacilities().get_const<traci::VehicleController>().getVehicleId();
    mVehicleController = &getFacilities().get_const<traci::VehicleController>();
}

// indicate packege
void VruDetectService::indicate(const btp::DataIndication& ind, omnetpp::cPacket* packet)
{
    Enter_Method("indicate");
    auto detectVruMessage = check_and_cast<const DetectVRU*>(packet);
    auto& ped_api = mVehicleController->getLiteAPI().person();

    //  pedestrians array mast contains values
    std::cout<< "Vehicle "<< mEgoId
             << " receive msg from: "<< detectVruMessage->getVehId()
             << " at : " << simTime()
             << " ped list size : " << detectVruMessage->getPedIdsArraySize();
    double errSum =0.0;
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
            double rPosx = ped_api.getPosition(pedId).x ; // actual position of ped
            double rPosy = ped_api.getPosition(pedId).y ;
            // get position error TODO:: check if  ped or (*pedIt)
            double error = sqrt ( pow((rPosx-ped.x),2) + pow ((rPosy-ped.y),2)); // error distance
            std::cout<< " || " << ped.pedId << " Error : " << error;
            if (error > 1.0) errorL++;
            else if (error > 0.5 && error <=1.0) errorM++;
            else errorX ++;
            errSum+=error;
        }
    }
    std::cout << "]" <<  std::endl;
    msgCount++;
    errorMid+= (errSum/detectVruMessage->getPedIdsArraySize());
    delete(detectVruMessage);
}

// called every 0.1s
void VruDetectService::trigger()
{
    Enter_Method("trigger");
    // get all objects from env
    auto& allObjects = mLocalEnvironmentModel->allObjects();

    detectPedestrians(filterBySensorCategory(allObjects,"Radar"));  // detect ped in camera field of view
    makeCalc();                                                     // erase repeated ped from received list
    clearObserved();                                                // remove ped out of camera range
    sendPedestrianInfo();                                           // send message to other vehicles
}

// select from radar data pedestrians and broadcast this info
void VruDetectService::detectPedestrians (const artery::TrackedObjectsFilterRange& objs){
    auto& ped_api = mVehicleController->getLiteAPI().person();
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
                    ped.updated = simTime();                    // time
                    observedPed.push_back(ped);                 // insert to list
                }
                else {                                              // update if ped on the list (recently observed)
                    (*pedIt).updated = simTime();                   // update time
                    (*pedIt).x = ped_api.getPosition(pedId).x;      // position x
                    (*pedIt).y = ped_api.getPosition(pedId).y;      // position y
                }
            }
        }
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
    req.gn.traffic_class.tc_id(static_cast<unsigned>(dcc::Profile::DP1));
    req.gn.communication_profile = geonet::CommunicationProfile::ITS_G5;

//    create Detect VRU packet
    auto packet = new DetectVRU();
    packet->setVehId(mEgoId.c_str());
    packet->setPedIdsArraySize(pedIds.size());
    int i = 0;
    for (i = 0; i < pedIds.size(); i++)
        packet->setPedIds(i,pedIds[i]);
    packet->setByteLength(10*i);  // size proportional to number of pedestrians
    request(req, packet);
}

// send just one
void VruDetectService::sendPedestrianInfo(Pedestrian ped)
{
    std::vector<Pedestrian> pedV;
    pedV.push_back(ped);
    sendPedestrianInfo(pedV);
}

// TODO::check interval of time ??
// make calculation of observed values
void VruDetectService::makeCalc()
{
    // not include repited ped from received list
    //int rSize = receivedPed.size();
    for (auto it = observedPed.begin(); it != observedPed.end(); it++)
        for (auto itR = receivedPed.begin(); itR != receivedPed.end(); itR++){
            if ( (*it).pedId == (*itR).pedId )  receivedPed.erase(itR--);
        }
    // print instant detected vehicles (radar + message)
    std::cout << mEgoId << " : " << observedPed.size() + receivedPed.size()<< std::endl;
    pedCount += (observedPed.size() + receivedPed.size());
    stepCount++;
}

// remove ped that is not observed anymore
void VruDetectService::clearObserved()
{
    int ped = 0;
    omnetpp::simtime_t updateInterval = omnetpp::SimTime(keepInterval);
    auto it = std::remove_if(observedPed.begin(), observedPed.end(), [&updateInterval](const Pedestrian& item)
    { return (simTime() - item.updated > updateInterval); });
    auto k = it;
    for (; k!=observedPed.end(); k++) ped++;
    observedPed.erase(it, observedPed.end());

    it = std::remove_if(receivedPed.begin(), receivedPed.end(), [&updateInterval](const Pedestrian& item)
    { return (simTime() - item.updated > updateInterval); });

    for (k = it; k!=receivedPed.end(); k++) ped++;
    receivedPed.erase(it, receivedPed.end());

    // update number of detected ped
    totalDetectPed+=ped;

    std::cout<< mEgoId << " observed : ";
    for (auto it = observedPed.begin(); it != observedPed.end(); it++)
        std::cout << (*it).pedId << " ";
    std::cout<<std::endl;
    std::cout<<mEgoId<<" received : ";
    for (auto it = receivedPed.begin(); it != receivedPed.end(); it++)
        std::cout << (*it).pedId << " ";
    std::cout<<std::endl;
//    std::cout<< mEgoId << " ped : " << ped << "Total ped " << totalDetectPed << std::endl;
}

void VruDetectService::finish()
{
    // you could record some scalars at this point
    ItsG5Service::finish();
    double midPed = (double) pedCount/stepCount;
    double midError = errorMid/msgCount;
    std::cout << "FINISH "<< mEgoId << " totPed: " << totalDetectPed
              << " X:"<< errorX << " M:"<< errorM << " L:"<< errorL
              << " minPed: " << midPed
              << " midError: " << midError
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


void VruDetectService::receiveSignal(cComponent* source, simsignal_t signal, cObject* object, cObject* obj_2)
{
    Enter_Method("receiveSignal");

    if (signal == scSignalCamReceived) {
       // auto& vehicle = getFacilities().get_const<traci::VehicleController>();
        EV_INFO << "Vehicle " << mVehicleController->getVehicleId() << " received a CAM in sibling serivce\n";
        std::cout << "Vehicle " << mVehicleController->getVehicleId() << " received a CAM in sibling serivce at: ";
        if (auto cam = dynamic_cast<CaObject*>(object)) {
            const auto id = cam->asn1()->header.stationID;
            const auto latitude = cam->asn1()->cam.camParameters.basicContainer.referencePosition.latitude;
            const auto longitude = cam->asn1()->cam.camParameters.basicContainer.referencePosition.longitude;
            std::cout << simTime()<< "::id ="<< id << " lat ="<<latitude<< "lon ="<< longitude << std::endl;
        }
    }
}
