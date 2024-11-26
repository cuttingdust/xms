cmake_minimum_required(VERSION 3.20)
project(xplatfrom)

set(DPS_LIBRARYS 
protobuf::libprotoc
protobuf::libprotobuf
protobuf::libprotobuf-lite
)

set(${PROJECT_NAME}_SHARED OFF CACHE BOOL "" FORCE)

cpp_library(${PROJECT_NAME})

if(MSVC)
    #设为启动项

    # set_property(DIRECTORY ${CMAKE_BINARY_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}) 
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${OUT_RUN_PATH})
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "XCore")

    # set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/GL")
    # set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/LTCG")
endif()