cplusplus {{
//#include <vector>
#include "../../scenarios/vru_detection/Pedestrian.h"
//#include "VruDetectService.h"
//typedef std::vector<int> IntVector;
//typedef cModule *ModulePtr;
}};

struct Pedestrian;

packet DetectVRU
{
    string vehId;
    Pedestrian pedIds[];
};
