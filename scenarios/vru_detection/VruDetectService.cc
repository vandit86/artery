#include "VruDetectService.h"
#include "vru_msgs/DetectVRU_m.h"
#include "artery/traci/VehicleController.h"
#include <vanetza/btp/data_request.hpp>
#include <vanetza/dcc/profile.hpp>
#include <vanetza/geonet/interface.hpp>

using namespace omnetpp;
using namespace vanetza;

Define_Module(VruDetectService)

void VruDetectService::initialize()
{
    ItsG5BaseService::initialize();
    mVehicleController = &getFacilities().get_const<traci::VehicleController>();
}

void VruDetectService::trigger()
{
    Enter_Method("VruDetectService trigger");
    btp::DataRequestB req;
    req.destination_port = host_cast<VruDetectService::port_type>(getPortNumber());
    req.gn.transport_type = geonet::TransportType::SHB;
    req.gn.traffic_class.tc_id(static_cast<unsigned>(dcc::Profile::DP1));
    req.gn.communication_profile = geonet::CommunicationProfile::ITS_G5;

    const std::string id = mVehicleController->getVehicleId();
    auto& vehicle_api = mVehicleController->getLiteAPI().vehicle();

    auto packet = new DetectVRU();
    packet->setEdgeName(vehicle_api.getRoadID(id).c_str());
    packet->setLaneIndex(vehicle_api.getLaneIndex(id));
    packet->setByteLength(40);
    request(req, packet);
}
