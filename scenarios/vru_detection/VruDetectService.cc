#include "VruDetectService.h"
//#include "vru_msgs/DetectVRU_m.h"
#include <vru_msgs/DetectVRU_m.h>
#include "artery/traci/VehicleController.h"
#include "artery/envmod/EnvironmentModelObject.h"
#include "artery/envmod/LocalEnvironmentModel.h"
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>
#include <boost/units/io.hpp>

using namespace omnetpp;
using namespace vanetza;
using namespace artery;

static const simsignal_t scSignalCamReceived = cComponent::registerSignal("CamReceived");

Define_Module(VruDetectService)

void VruDetectService::initialize()
{
    ItsG5BaseService::initialize();
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

    if (packet->getByteLength() == 40) {
        //        EV_INFO << "packet indication on channel " << net.channel << "\n";
    }

    const std::string my_id = mVehicleController->getVehicleId();
    auto& vehicle_api = mVehicleController->getLiteAPI().vehicle();

    //  pedestrians array mast contains values
    if (detectVruMessage->getPedIdArraySize() > 0){
        std::cout<< "Vehicle "<< my_id
                 << " receive vru message from: "<< detectVruMessage->getVehId()
                 << " at : " << simTime();

        for (int i = 0 ; i < detectVruMessage->getPedIdArraySize() ; i++){
            std::cout << " locate ped: " << detectVruMessage->getPedId(i);

        }
        std::cout << std::endl;
    }

    delete(detectVruMessage);
}

// called every 0.1s
void VruDetectService::trigger()
{
    Enter_Method("trigger");
//    std::cout << "Trigger: " << mEgoId<<  std::endl;

    // get all objects from env
    auto& allObjects = mLocalEnvironmentModel->allObjects();
    detectPedestrians(filterBySensorCategory(allObjects,"Radar"));
}

// select from radar data pedestrians and broadcast this info
void VruDetectService::detectPedestrians (const artery::TrackedObjectsFilterRange& objs){
    std::vector<std::string> pedIds;
    for(const auto &obj : objs ){
        std::weak_ptr<EnvironmentModelObject> obj_ptr = obj.first;
        if (obj_ptr.expired()) continue; /*< objects remain in tracking briefly after leaving simulation */
        std::string pedId = obj_ptr.lock()->getExternalId();
        // if object is a pedestrian
        if ( pedId.rfind("p",0) == 0 ){
            const auto& vd = obj_ptr.lock()->getVehicleData();
//            std::cout << "Detect ped: " << pedId
//                      << "pedestrian ID: " << vd.station_id()
//                      << " lon: " << vd.longitude()
//                      << " lat: " << vd.latitude()
//                      << " speed: " << vd.speed()
//                      << " when: " << vd.updated()
//                      << std::endl;
            pedIds.push_back(pedId);
        }
    }
    if (pedIds.size()>0){
        sendPedestrianInfo(mEgoId, pedIds);
    }
}

// send info about detected pedestrian
void VruDetectService::sendPedestrianInfo(std::string egoId, std::vector<std::string> pedIds)
{
    btp::DataRequestB req;
    req.destination_port = host_cast<VruDetectService::port_type>(getPortNumber());
    req.gn.transport_type = geonet::TransportType::SHB;
    req.gn.traffic_class.tc_id(static_cast<unsigned>(dcc::Profile::DP1));
    req.gn.communication_profile = geonet::CommunicationProfile::ITS_G5;

    const std::string id = mVehicleController->getVehicleId();
//    auto& vehicle_api = mVehicleController->getLiteAPI().vehicle();

    auto packet = new DetectVRU();
//    packet->setEdgeName(vehicle_api.getRoadID(id).c_str());
    packet->setVehId(egoId.c_str());
    packet->setPedIdArraySize(pedIds.size());
    int i = 0;
    for (i = 0; i < pedIds.size(); i++)
        packet->setPedId(i,pedIds[i].c_str());
    packet->setByteLength(40*i);  // size proportional to number of pedestrians
    request(req, packet);
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
