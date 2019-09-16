#ifndef VruDetectService_H_
#define VruDetectService_H_

#include "artery/application/ItsG5Service.h"

// forward declaration
namespace traci { class VehicleController; }

class VruDetectService : public artery::ItsG5Service
{
    public:
        void trigger() override;

    protected:
        void initialize() override;
        const traci::VehicleController* mVehicleController = nullptr;
};

#endif /* VruDetectService_H_ */
