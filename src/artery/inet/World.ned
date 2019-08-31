package artery.inet;

import artery.storyboard.Storyboard;
import inet.environment.common.PhysicalEnvironment;
import inet.environment.contract.IPhysicalEnvironment;
import inet.visualizer.contract.IPhysicalEnvironmentVisualizer;
import inet.physicallayer.contract.packetlevel.IRadioMedium;
import traci.Manager;

network World
{
    parameters:
        bool withStoryboard = default(false);
        bool withPhysicalEnvironment = default(false);
        int numRoadSideUnits = default(0);
        traci.mapper.vehicleType = default("artery.inet.Car");
        traci.nodes.vehicleSinkModule = default(".mobility");
        storyboard.middlewareModule = default(".middleware");

    submodules:
        traci: Manager {
            parameters:
                @display("p=20,20");
        }

        radioMedium: <default("Ieee80211ScalarRadioMedium")> like IRadioMedium {
            parameters:
                @display("p=60,20");
                mediumLimitCache.carrierFrequency = 5.9GHz;
        }

        canvasVisualizer: <default("PhysicalEnvironmentCanvasVisualizer")> like IPhysicalEnvironmentVisualizer {
            parameters:
                @display("p=100,70");
        }

        physicalEnvironment: <default("PhysicalEnvironment")> like IPhysicalEnvironment if withPhysicalEnvironment {
                  parameters:
                      @display("p=100,20");
          }

        storyboard: Storyboard if withStoryboard {
            parameters:
                @display("p=140,20");
        }

        rsu[numRoadSideUnits]: RSU {
            parameters:
                mobility.initFromDisplayString = false;
        }


}
