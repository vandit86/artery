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
    instantDetectPedNum = detectVruMessage->getPedIdsArraySize();
    if (instantDetectPedNum > 0){
        std::cout<< "Vehicle "<< mEgoId
                 << " receive msg from: "<< detectVruMessage->getVehId()
                 << " at : " << simTime()
                 << " ped list size :  " << detectVruMessage->getPedIdsArraySize() ;

        //GET REAL POSITION and Compare with received one
        for (int i = 0 ; i < detectVruMessage->getPedIdsArraySize() ; i++){
            const Pedestrian& ped = detectVruMessage->getPedIds(i);
            if (inSimulation(ped.pedId)){
                if (ped_api.getPosition(ped.pedId).x != ped.x || ped_api.getPosition(ped.pedId).y != ped.y){

                    double rPosx = ped_api.getPosition(ped.pedId).x ; // actual position of ped
                    double rPosy = ped_api.getPosition(ped.pedId).y ;
                    double error = sqrt ( pow((rPosx-ped.x),2) + pow ((rPosy-ped.y),2)); // error distance
                    std::cout<< " Error : " << error;
                }
            }
        }
        std::cout << "]" <<  std::endl;
    }
    delete(detectVruMessage);
    lastReceivedTime = simTime(); // save last received time
}

// called every 0.1s
void VruDetectService::trigger()
{
    Enter_Method("trigger");
    // get all objects from env
    auto& allObjects = mLocalEnvironmentModel->allObjects();
    detectPedestrians(filterBySensorCategory(allObjects,"Radar"));
}

// select from radar data pedestrians and broadcast this info
void VruDetectService::detectPedestrians (const artery::TrackedObjectsFilterRange& objs){
    auto& ped_api = mVehicleController->getLiteAPI().person();
    for(const auto &obj : objs ){
        std::weak_ptr<EnvironmentModelObject> obj_ptr = obj.first;
        if (obj_ptr.expired()) continue; /*< objects remain in tracking briefly after leaving simulation */
        std::string pedId = obj_ptr.lock()->getExternalId();
        // if object is a pedestrian

        if ( pedId.rfind("p",0) == 0 ){ // TODO create isPedestrian functio
            // try to find pedestrian on ped list
            auto pedIt = find_if(pedIds.begin(), pedIds.end(), [&pedId](const Pedestrian& obj) {return obj.pedId == pedId;});

            // if ped not in list of detected ped then add to the list
            if (pedIt == pedIds.end())
            {
                if (inSimulation(pedId)){
                    Pedestrian ped;
                    ped.pedId = pedId;
                    ped.x = ped_api.getPosition(pedId).x;       // position x
                    ped.y = ped_api.getPosition(pedId).y;       // position y
                    ped.updated = simTime();                    // time
                    pedIds.push_back(ped);                      // insert to list
                    //sendPedestrianInfo(ped);                  // send one ped info
                }
            }
        }
    }

    // put to 0 if not receive any message durring 3 intervals of sending
    if (simTime()- lastReceivedTime > vruSendInterval*3){
        instantDetectPedNum = 0;
    }

    // print instant detected vehicles (radar + message)
    std::cout << mEgoId << " : " << pedIds.size() + instantDetectPedNum << std::endl;

    totalDetectPed+= (pedIds.size() + instantDetectPedNum) ;
    totalMeasurments++;

    if (pedIds.size()>0){
        // create logic to select moment when to send the Detect VRU packege
        if (simTime()- lastSendTime > vruSendInterval){
            lastSendTime = simTime();
            //sendPedestrianInfo(pedIds);
        }
        pedIds.clear(); // remove all elements from vector to mantain data updated
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
    packet->setByteLength(40*i);  // size proportional to number of pedestrians
    request(req, packet);
}

// send just one
void VruDetectService::sendPedestrianInfo(Pedestrian ped)
{
    std::vector<Pedestrian> pedV;
    pedV.push_back(ped);
    sendPedestrianInfo(pedV);
}

void VruDetectService::finish()
{
    // you could record some scalars at this point
    ItsG5Service::finish();
    std::cout << "FINISH "<< mEgoId << " : " << totalDetectPed/totalMeasurments << " tcount: "<< totalMeasurments << " totPed: " << totalDetectPed<<std::endl;
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
