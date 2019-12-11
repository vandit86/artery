#ifndef PEDESTRIAN_H
#define PEDESTRIAN_H

struct Pedestrian
{
    std::string pedId;
    double x;
    double y;
    omnetpp::simtime_t updated;

};

#endif // PEDESTRIAN_H
