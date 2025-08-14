cmake_minimum_required(VERSION 3.12)

# 使用 vcpkg 工具链文件 Warning  工具连设置必须是在project 前面
set(CMAKE_TOOLCHAIN_FILE $ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake CACHE STRING "Vcpkg toolchain file")
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(xms)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
if(MSVC)
set(CMAKE_GENERATOR_PLATFORM x64)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE Debug) 

# QT
# UIC 的自控一下会生成大量的配置文件 和一个ui 和 四个 ui_*.h的情况 ，所以我们采用老办法
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# set(AUTOUIC_EXECUTABLE "$ENV{QTDIR}/bin/uic.exe")
# set(AUTOMOC_EXECUTABLE "$ENV{QTDIR}/bin/moc.exe")
# set(AUTORCC_EXECUTABLE "$ENV{QTDIR}/bin/rcc.exe")

# spdlog
find_package(spdlog REQUIRED)
message("spdlog_FOUND = ${spdlog_FOUND}")

# 查找 vcpkg 安装的库
find_package(Qt6 6.7.2 REQUIRED Core Gui Widgets)
message("Found Qt6 FOUND: ${Qt6_FOUND}")

# OpenSSL
find_package(OpenSSL REQUIRED)
message("OpenSSL_FOUND = ${OpenSSL_FOUND}")
message("Found OpenSSL VERSION: ${OpenSSL_VERSION}")

#libevent
find_package(Libevent CONFIG REQUIRED)
message("Found Libevent FOUND: ${Libevent_FOUND}")
message("Found Libevent VERSION: ${Libevent_VERSION}")

find_package(protobuf CONFIG REQUIRED)
message("Found protobuf FOUND: ${protobuf_FOUND}")
message("Found protobuf VERSION: ${protobuf_VERSION}")
find_program(PROTOC_EXECUTABLE protoc)
if(NOT PROTOC_EXECUTABLE)
    message(FATAL_ERROR "protoc executable not found")
else()
    message(STATUS "Found protoc: ${PROTOC_EXECUTABLE}")	
endif()

# OpenMP
find_package(OpenMP)
message("OpenMP_FOUND = ${OpenMP_FOUND}")

if(OpenMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# 外部依赖项
set(DPS_INCLUDES)
set(DPS_TARGETS)
set(DPS_LIBRARYS)

# 资源文件

# #图标
set(RC_FILE)

# #qrc 文件
set(QRC_SOURCE_FILES)

# 忽略特定库
if(MSVC)
SET(CMAKE_EXE_LINKER_FLAGS /NODEFAULTLIB:"libcmt.lib;libc.lib")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CommonTools)


add_subdirectory(xmysql)
add_subdirectory(xplatfrom)
add_subdirectory(xtest_service_client)
add_subdirectory(xtest_pb_service)
add_subdirectory(xms_gateway)
add_subdirectory(register_client)
add_subdirectory(register_server)
add_subdirectory(config_server)
add_subdirectory(config_client)
add_subdirectory(config_gui)
add_subdirectory(xtest_xplatform_ssl)
add_subdirectory(xtest_service_client_ssl)
add_subdirectory(watchdog)

watchdog(register_server 3)
watchdog(config_server 3)

# 拷贝assert目录到输出目录
set(ASSERT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/assert)
file(COPY ${ASSERT_PATH} DESTINATION ${OUT_RUN_PATH})
