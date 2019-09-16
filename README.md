
Artery enables V2X simulations based on ETSI ITS-G5 protocols like GeoNetworking and BTP.
Single vehicles can be equipped with multiple ITS-G5 services through Artery's middleware, which also provides common Facilities for these services.

Artery started as an extension of the [Veins framework](http://veins.car2x.org) but can be used independently nowadays.
Please refer to its [documentation](http://veins.car2x.org/documentation) for details about Veins.

## Requirements

Compatible versions of INET, Veins, Vanetza, and other components are managed as [git submodules](https://git-scm.com/docs/git-submodule) in the *extern* subdirectory.
Please make sure to fetch these submodules when cloning our repository!

    git clone --recurse-submodule https://github.com/vandit86/artery

Alternatively, you can also load these submodules after cloning:

    git submodule update --init --recursive
    
    
# Vadym's Artery instalation guide

successfully tested and installed on Linux Mint 19.2

## Requirements (pre-instalation steps)

You need a C++ Compiler with C++11 support, Boost and Vanetza libraries for building Artery along with Veins.
Artery and Veins build upon the discrete event simulator [OMNeT++](https://omnetpp.org), which you need to obtain as well.
We have tested Artery with OMNeT++ 5.4, GNU GCC 7.3 and Boost 1.65.1 successfully.
SUMO 1.0 or later is required since its TraCI protocol is not compatible with earlier versions anymore.
Only [CMake](http://www.cmake.org) is the officially supported way for building Artery.


    sudo nano /etc/apt/sources.list 
    add hear ubuntu xenial repositories 


    sudo apt-get update && sudo apt-get install libesd0-dev
    sudo apt-get install build-essential gcc g++ bison flex perl tcl-dev tk-dev blt libxml2-dev zlib1g-dev default-jre doxygen graphviz libwebkitgtk-1.0-0 openmpi-bin libopenmpi-dev libpcap-dev autoconf automake libtool libproj-dev libgdal1-dev libfox-1.6-dev libgdal-dev libxerces-c-dev qt4-dev-tools 

install boost lib 
    sudo apt-get install libboost-all-dev

*install geolib  from source* 
    https://geographiclib.sourceforge.io/html/install.html 

install crypto lib
    sudo apt-get install libcrypto++-dev libcrypto++-doc libcrypto++-utils


### omnet++ 

*https://www.nsnam.com/2015/02/omnet-installation-in-linux-mint-171.html*

libproj library download separately and install (https://debian.pkgs.org/8/debian-main-i386/libproj0_4.8.0-5_i386.deb.html) 

    sudo apt-get install build-essential gcc g++ bison flex perl tcl-dev tk-dev blt libxml2-dev zlib1g-dev default-jre doxygen graphviz libwebkitgtk-1.0-0 openmpi-bin libopenmpi-dev libpcap-dev autoconf automake libtool libgdal1-dev libfox-1.6-dev libgdal-dev libxerces-c-dev

__instalation guide omnet++:__ 
    https://doc.omnetpp.org/omnetpp/InstallGuide.pdf

change ./confirguration file if nedded (exluding thouse futures is requered to install sumo):
    ./configure WITH_OSGEARTH=no WITH_OSG=no
    make MODE=release

If afterwords you need to make changes on config file, you have to remove all previous libraries and recompile OMNeT++:
    $ make cleanall
    $ make
 general OMNeT++ requires that its bin directory should be in the PATH. You shouldadd a line something like this to your .bashrc:$ 
    export PATH=$HOME/omnetpp-5.5/bin:$PATH
 
## Artery Build instructions

Veins and Vanetza need to be built before Artery itself.
For this purpose, a Makefile is located in Artery's root directory.
The following steps assume you have cloned the Artery repository on your (preferably Linux) machine and you have moved your terminal to this clone's directory.
You can build all of these external project dependencies at once via the default make all target.

### Veins

Please make sure you have all dependencies of Veins installed and set up OMNeT++ beforehand.
If you have any doubts, refer to the Veins tutorial. The bundled version of Veins is then built with following steps:

    cd extern/veins
    ./configure
    make MODE=release

if error :: 
*_TraCI API Compatibility error .. -> change source code on veins (or install older sumo version)_*

Alternatively, you can also just invoke make veins in the root directory of Artery, which executes exactly these steps but saves you some typing.

### Vanetza

Similarly, you can simply call 
    make vanetza 
in the root directory and it will try to build Vanetza for you in extern/vanetza/build.

### INET

As the next step, you need to build INET. Make sure you are in the root directory of Artery and simply call 
    make inet 
there. INET's build dependencies are listed in its install manual.

if compilation error : 
in "inet/common/queue/PacketQueue.cc" change to : 

    namespace inet {

    PacketQueue::PacketQueue(const char *name) :
        cPacketQueue(name)
    {
    }
    
remove nullptr and recompile 

### SimuLTE

    make simulte

*_required working INET-Framework installation (v3.*) NOT 4_* 

or 

Building SimuLTE from the command line

    Extract the downloaded SimuLTE tarball next to the INET directory
    Change to the simulte directory.
    Type "make makefiles". This should generate the makefiles.
    Type "make" to build the simulte executable (debug version). Use "make MODE=release" to build release version.
    You can run examples by changing into a directory under 'simulations', and executing "./run"

h3. Artery

must call commands below to make integration with simulte
or switch to ON all build functions on CMakeLists.txt and than compile normaly 


    mkdir build
    cd build
    cmake -DWITH_SIMULTE=ON ..
    cmake --build .

check CMakeLists.txt for build options

These steps create a build directory for Artery, configure the build directory and finally build Artery there.

## Running Artery

When you have finished building Artery, you can give the shipped example a try to see if everything runs smoothly.
With current Artery versions there is no need to start Veins' sumo-launchd script any more.
Instead, Artery will start a SUMO instance on its own with appropriate parameters.
You can start the example scenario of Artery (located in scenarios/artery) simply by calling the run_example target from Artery's root directory:

    cmake --build build --target run_example

Please make sure that sumo can be executed within in your environment because this is the default SUMO executable used by Artery.
You can, however, specify which SUMO executable shall be used explicilty.
If you want Artery to start SUMO with a graphical user interface, you can put the following line in your omnetpp.ini:

    *.traci.launcher.sumo = "sumo-gui" 


### Running 802.11p and LTE together

SIMULTE scenario combines two interface card for heterogeneous communication 802.11p (V2V) and mobile LTE
run this command on root directory to see the example work

    cmake --build build/ --target run_simulte

#### running without OMNET++ GUI

changes on file AddOppRun.cmake 
    
    # set(RUN_FLAGS "" CACHE STRING "Flags appended to run command (and debug)")
    set(RUN FLAGS "-u Cmdenv") # this is to run on cmdenv
    set(RUN FLAGS "-c Config") # this is to run specific configuration

### import artery to Qt creator IDE (successfully tested)

To open an existing CMake project:

    Select File > Open File or Project.
    Select the CMakeLists.txt file from your CMake project.
    Select a kit that is configured to use CMake for building the project.
    In Projects, right-click the project name to open the context menu, and then select Run CMake to have the project contents listed in the view.


### import Artery as project into the OMNeT++ IDE (Not tested on new Omnet IDE)

    cmake /artery/root/dir -G 'Eclipse CDT4 - Unix Makefiles'
    Import -> Select root directory (select directory where you executed step 1) -> Finish
    Right-click on project -> Add OMNeT++ support

### Obstacle visualisation with Omnet++

 Veins config :
    add and configure AnnotationManager to src/veins/World.ned

 INET config :
        1)add module from INET packege
        canvasVisualizer: <default("PhysicalEnvironmentCanvasVisualizer")> like IPhysicalEnvironmentVisualizer {
        2) convert from osm poly format to INET format using external tool
        3) add to omnet.ini
        *.physicalEnvironment.config = xmldoc("obstacles.xml") # physical objects are defined in a separate XML file.
        
### use two interfaces (LTE and WLAN)

extend any scenario with Lte.world as main .ned file with

    *.node[*].middleware.services = xmldoc("services.xml")

include on services.xml all required services i.e. CAM, DEM , Radar (see below)  


### Sensor measurments representation (frond RADAR and CamSensor)

1) **use envmod configuration on artery scenario to see example** ..
add to the services.xml this service :

    <service type="artery.envmod.service.EnvmodPrinter">
        <listener port="7001" />
    </service>

**this will print on EV DETAIL log the data from sensors listed on sensors.xml**


# SUMO 

use netedit to create scenarious -- *.net.xml 

## Generate random trips  

see 
    /usr/share/sumo/tools/randomTrips.py --help

## Add VRU on suimulation 

to generate rou.xml file with pedestrians : random pedestrian demand using the option --pedestrians. The option --max-dist <FLOAT> may be used to limit the maximum air distance of pedestrian walks.

    /usr/share/sumo/tools/randomTrips.py -n net.net.xml  -r net.rou.xml --begin=0 --end=20 --pedestrians

Caution

To ensure proper generation of crossings, road lanes need to prohibit pedestrians either by setting **disallow="pedestrian"** or by explicitly specifying all other allowed classes using attribute allow When adding sidewalks via attribute sidewalkWidth or any of the heuristics above, pedestrians will be forbidden automatically on the remaining lanes.









