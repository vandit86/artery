# CMake generated Testfile for 
# Source directory: /home/vad/artery/scenarios/testbed
# Build directory: /home/vad/artery/scenarios/testbed
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(testbed-no-api "/home/vad/omnetpp-5.5.1/bin/opp_run_release" "-n" "/home/vad/artery/src/artery:/home/vad/artery/src/traci:/home/vad/artery/extern/veins/examples/veins:/home/vad/artery/extern/veins/src/veins:/home/vad/artery/extern/inet/src:/home/vad/artery/extern/inet/examples:/home/vad/artery/extern/inet/tutorials:/home/vad/artery/extern/inet/showcases" "-l" "/home/vad/artery/src/artery/envmod/libartery_envmod.so" "-l" "/home/vad/artery/scenarios/highway-police/libartery_police.so" "-l" "/home/vad/artery/extern/inet/out/gcc-release/src/libINET.so" "-l" "/home/vad/artery/src/artery/testbed/libartery_testbed.so" "-l" "/home/vad/artery/src/artery/envmod/libartery_envmod.so" "-l" "/home/vad/artery/src/artery/storyboard/libartery_storyboard.so" "-l" "/home/vad/artery/extern/inet/out/gcc-release/src/libINET.so" "-l" "/home/vad/artery/extern/veins/out/gcc-release/src/libveins.so" "-l" "/home/vad/artery/src/artery/libartery_core.so" "omnetpp.ini" "-uCmdenv" "-cnoSeaApi" "--sim-time-limit=5s")
set_tests_properties(testbed-no-api PROPERTIES  WORKING_DIRECTORY "/home/vad/artery/scenarios/testbed")
