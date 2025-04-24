cmake_minimum_required(VERSION 3.20)
get_filename_component(CURRENT_DIR ${CMAKE_CURRENT_SOURCE_DIR} NAME)

project(${CURRENT_DIR})

if(MSVC)
  set( DPS_LIBRARYS 
    libmysql.lib
  )
elseif(APPLE)
  set( DPS_LIBRARYS
    libmysqlclient.a
	libmysqlservices.a
    libssl.dylib
    libcrypto.dylib
    iconv
  )
endif()

cpp_library(${PROJECT_NAME})

if(MSVC)
    #设为启动项

    #set_property(DIRECTORY ${CMAKE_BINARY_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}) 
    set_target_properties(${PROJECT_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${OUT_RUN_PATH})
    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "XCore")

    #set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/GL")
    #set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/LTCG")
endif()
