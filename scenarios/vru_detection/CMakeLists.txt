#add_artery_feature(vru_detection ClearLaneService.cc PoliceService.cc)
add_artery_feature(vru_detection VruDetectService.cc)
generate_opp_message(DetectVRU.msg TARGET vru_detection DIRECTORY vru_msgs)
add_opp_run(vru_detection NED_FOLDERS ${CMAKE_CURRENT_SOURCE_DIR})

if(WITH_STORYBOARD)
    #target_sources(vru PUBLIC PoliceServiceStoryboard.cc)
endif()

if(WITH_ENVMOD)
    #target_sources(vru PUBLIC PoliceServiceEnvmod.cc)
    target_link_libraries(vru_detection PUBLIC envmod)
endif()

