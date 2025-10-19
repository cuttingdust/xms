cmake_minimum_required(VERSION 3.20)
get_filename_component(CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} NAME)

project(${CURRENT_DIR})

set(DPS_LIBRARYS 
    ${LOG_MOUDLES}
    ${SSL_MOUDLES}
    ${Protobuf_MOUDLES}
)

set(${PROJECT_NAME}_SHARED ON CACHE BOOL "" FORCE)

#################### 导出PB-h ####################
set(PROTOC_EXPORT_INCLUDE "XPlatfrom_Global.h")
set(PROTOC_EXPORT_CLASS 
    XMsgHead 
    XDirReq
    XDirRes
    XServiceMap
    XServiceMap_XService
    XServiceMap_XServiceList
	XRegisterReq
	XMessageRes
    XGetServiceReq
    XConfig
    XLoadConfigReq
    XDirConfig
    XLoadAllConfigReq
    XConfigList
    XLoginReq
    XGatewayConfig
    XMsgHeart
    XAddUserReq
    XLoginRes
    XAddLogReq
    ###########
    XFileInfo
    XFileInfoList
    XGetDirReq
    XDiskInfo
)
set(PROTOC_EXPORT_DEFINE "XPLATFROM_EXPORT")

cpp_library(${PROJECT_NAME})


if(MSVC)
    #设为启动项

    # set_property(DIRECTORY ${CMAKE_BINARY_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}) 
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${OUT_RUN_PATH})
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "XCore")

    # set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/GL")
    # set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/LTCG")
endif()