//
// Artery V2X Simulation Framework
// Copyright 2014-2017 Hendrik-Joern Guenther, Raphael Riebl, Oliver Trauer
// Licensed under GPLv2, see COPYING file for detailed license and warranty terms.
//

package artery.envmod;

import artery.envmod.LocalEnvironmentModel;
import artery.inet.Car;
import artery.envmod.service.EnvmodPrinter;
import artery.application.ItsG5Service;

module Car extends artery.inet.Car
{
    submodules:
        environmentModel: LocalEnvironmentModel {
            @display("p=214,57");
            middlewareModule = default(".middleware");
        }
}
