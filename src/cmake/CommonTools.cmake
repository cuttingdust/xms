# 输入路径
set(SDK_DIR
    # ${UPMODELEDITOR_ROOT}/SDK/me-sdk
    # ${UPMODELEDITOR_ROOT}/SDK/sdk
    # ${UPMODELEDITOR_ROOT}/SDK/wvm-sdk
)
message("SDK_DIR = ${SDK_DIR}")

set(SDK_INCLUDE
	# $ENV{UPS_Library}
    # ${UPMODELEDITOR_ROOT}/SDK/me-sdk/include
    # ${UPMODELEDITOR_ROOT}/SDK/sdk/include
    # ${UPMODELEDITOR_ROOT}/SDK/wvm-sdk/include
	$ENV{MYSQL_SERVER80}/include
)

set(SDK_LIB_DIRECTORY
    # ${UPMODELEDITOR_ROOT}/SDK/me-sdk/lib
    # ${UPMODELEDITOR_ROOT}/SDK/sdk/lib
    # ${UPMODELEDITOR_ROOT}/SDK/wvm-sdk/lib
	$ENV{MYSQL_SERVER80}/lib
)

# 输出路径
set(OUT ${CMAKE_CURRENT_SOURCE_DIR}/../out)
message("out = ${OUT}")
set(OUT_LIB_PATH ${OUT}/lib)
set(OUT_DLL_PATH ${OUT}/bin.x64)
set(OUT_INCLUDE_PATH ${OUT}/include)
set(OUT_RUN_PATH ${OUT}/bin.x64)

# 安装与查找
string(REPLACE "\\" "/" INSTALL_PREFIX ${OUT})
set(CMAKE_INSTALL_PREFIX ${INSTALL_PREFIX})
message("CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_PREFIX_PATH ${INSTALL_PREFIX}/lib/config)
message("CMAKE_PREFIX_PATH = ${CMAKE_PREFIX_PATH}")

# Qt Moudle
set(QT6_MOUDLES
    Qt6::Core
	Qt6::Widgets
    Qt6::Gui
)

# libevent
set(Libevent_MOUDLES
	libevent::core
	libevent::extra
	libevent::openssl
)

# openssl 
set(SSL_MOUDLES 
	OpenSSL::SSL 
	OpenSSL::Crypto
)

# protobuf
set(Protobuf_MOUDLES
	protobuf::libprotoc
	protobuf::libprotobuf
	protobuf::libprotobuf-lite
)

#spdlog
set(LOG_MOUDLES
	spdlog::spdlog_header_only
)

# 获取当前目录下源码和头文件
macro(get_src_include)
    aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/src SRC)
    aux_source_directory(${CMAKE_CURRENT_LIST_DIR}/Source SOURCE)

    list(APPEND SRC ${SOURCE})
	# message("SRC = ${SRC}")
	
	FILE(GLOB H_FILE_I ${CMAKE_CURRENT_LIST_DIR}/include/*.h)
    
	# 图标资源
	FILE(GLOB RC_FILE ${CMAKE_CURRENT_LIST_DIR}/src/*.rc)
	
	# 安装的时候 不暴露出去
    FILE(GLOB UI_FILES ${CMAKE_CURRENT_LIST_DIR}/src/*.ui)
    FILE(GLOB QRC_SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/src/*.qrc)
	FILE(GLOB PROTO_FILES ${CMAKE_CURRENT_LIST_DIR}/src/*.proto)

	
    if(RC_FILE)
        source_group("Resource Files" FILES ${RC_FILE})
    endif()

    if(UI_FILES)
        qt_wrap_ui(UIC_HEADER ${UI_FILES})
        source_group("Resource Files" FILES ${UI_FILES})
        source_group("Generate Files" FILES ${UIC_HEADER})
    endif()

    if(QRC_SOURCE_FILES)
        qt6_add_resources(QRC_FILES ${QRC_SOURCE_FILES})
        # qt_wrap_cpp() moc 相关
        source_group("Resource Files" FILES ${QRC_SOURCE_FILES})
		source_group("Generate Files" FILES ${QRC_FILES})
    endif()
	
	if(PROTO_FILES)
		message(STATUS "PROTO_FILES = ${PROTO_FILES}")
		source_group("Resource Files" FILES ${PROTO_FILES})
		if(PROTOC_EXECUTABLE)
        execute_process(
            COMMAND ${PROTOC_EXECUTABLE} -I=${CMAKE_CURRENT_LIST_DIR}/src --cpp_out=${CMAKE_CURRENT_LIST_DIR}/src ${PROTO_FILES}
            WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        )

        FILE(GLOB PROTO_CC_FILE ${CMAKE_CURRENT_LIST_DIR}/src/*.pb.cc)
        FILE(GLOB PROTO_HREADER_FILE ${CMAKE_CURRENT_LIST_DIR}/src/*.pb.h)
        
        FILE(COPY ${PROTO_HREADER_FILE} DESTINATION ${CMAKE_CURRENT_LIST_DIR}/include)
        FILE(REMOVE ${PROTO_HREADER_FILE})

        FILE(GLOB PROTO_HREADER_FILE ${CMAKE_CURRENT_LIST_DIR}/include/*.pb.h)
		
		if(PROTOC_EXPORT_INCLUDE)
			add_include_to_headers(PROTO_HREADER_FILE ${PROTOC_EXPORT_INCLUDE})
            if(PROTOC_EXPORT_CLASS)
                if(PROTOC_EXPORT_DEFINE)
                    replace_class_name_in_headers(PROTO_HREADER_FILE PROTOC_EXPORT_CLASS ${PROTOC_EXPORT_DEFINE})
                endif()
            endif()
		endif()
		
        source_group("Generate Files" FILES ${PROTO_CC_FILE})
        source_group("Generate Files" FILES ${PROTO_HREADER_FILE})		
        endif()
	endif()
endmacro()

# GCC 设置忽略编译告警
macro(remove_warnings)
    add_definitions(-Wno-unused-value -Wno-unknown-pragmas -Wno-sequence-point
        -Wno-delete-non-virtual-dtor -Wno-unused-but-set-variable
        -Wno-sign-compare -Wno-unused-variable -Wno-return-local-addr
        -Wno-unused-function -Wno-deprecated-declarations)
endmacro()

# 配置编译参数
macro(set_cpp name)
    target_link_directories(${name} PRIVATE ${SDK_LIB_DIRECTORY})

    #message("Qt6_FOUND = ${Qt6_FOUND}")
    #target_link_libraries(${name} ${QT6_MOUDLES})
	
	message("Libevent_FOUND = ${Libevent_FOUND}")
    target_link_libraries(${name} ${Libevent_MOUDLES})
	

    message("DPS_INCLUDES = ${DPS_INCLUDES}")

    # 路径被两次引用 1 编译slib库时 2 install export写入config时
    target_include_directories(${name} PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> # install时为空,只有编译时有值
        $<INSTALL_INTERFACE:include> # 只有install时有值 /home/hdb/xcpp/include
    )

    target_include_directories(${name} PRIVATE
        ${DPS_INCLUDES}
        ${SDK_INCLUDE}
    )
    set(DPS_INCLUDES "")

    message("DPS_TARGETS = ${DPS_TARGETS}")

    if(DPS_TARGETS)
        add_dependencies(${name} ${DPS_TARGETS})

        if(TEST_FIND)
            foreach(target IN LISTS DPS_TARGETS)
                message("   ++++++++++++++++++++++${target}+++++++++++++++++++++++")
                find_package(${target} ${version})
                message("   ${target}_FOUND = ${${target}_FOUND}")

                if(NOT ${target}_FOUND)
                    continue()
                endif()

                get_target_property(inc ${target} INTERFACE_INCLUDE_DIRECTORIES)
                message("   INTERFACE_INCLUDE_DIRECTORIES = ${inc}")
                message("   +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++")
            endforeach()
        endif()

        target_link_libraries(${name} ${DPS_TARGETS})

        set(DPS_TARGETS "")
    endif()

    message("DPS_LIBRARYS = ${DPS_LIBRARYS}")
    target_link_libraries(${name} ${DPS_LIBRARYS})
    set(DPS_LIBRARYS "")

    target_compile_features(${name} PRIVATE
        cxx_std_23
    )

    target_link_options(${name} PRIVATE
        -D2:-AllowCompatibleILVersions
    )

    target_compile_definitions(${name} PUBLIC
        -DUNICODE
        -D_UNICODE
        -DNOMINMAX
        -D_USE_MATH_DEFINES
    )

    if(MSVC)
		 target_compile_definitions(${name} PUBLIC
			-D_CRT_SECURE_NO_WARNINGS
		)
		
		
		target_compile_options(${name} PUBLIC
			/wd4251 # 添加抑制 C4251 警告的选项
		)
		
        set_target_properties(${name} PROPERTIES
            COMPILE_FLAGS "/Zc:wchar_t"	# 是
			#COMPILE_FLAGS "/Zc:wchar_t-" #否
        )

        # set_target_properties(${name} PROPERTIES
        # COMPILE_FLAGS "-bigobj"
        # )
        set_target_properties(${PROJECT_NAME} PROPERTIES
            MSVC_RUNTIME_LIBRARY MultiThreadedDLL
        )
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "")
        set(CMAKE_BUILD_TYPE RelWithDebInfo)
    endif()

    set(CONF_TYPES Debug Release RelWithDebInfo MinSizeRel)
    list(APPEND CONF_TYPES "")

    foreach(type IN LISTS CONF_TYPES)
        set(conf "")

        if(type)
            string(TOUPPER _${type} conf)
        endif()

        set_target_properties(${name} PROPERTIES
            RUNTIME_OUTPUT_DIRECTORY${conf} ${OUT_RUN_PATH} # dll  exe 执行程序
            LIBRARY_OUTPUT_DIRECTORY${conf} ${OUT_LIB_PATH} # .so .dylib
            ARCHIVE_OUTPUT_DIRECTORY${conf} ${OUT_LIB_PATH} # .lib .a
            PDB_OUTPUT_DIRECTORY${conf} ${OUT_RUN_PATH} # pdb
        )
    endforeach()

    # set_target_properties(${name} PROPERTIES
    # DEBUG_POSTFIX "_d"
    # )
    set(debug_postfix "")

    if(WIN32)
        get_target_property(debug_postfix ${name} DEBUG_POSTFIX)
    endif()
endmacro()

# 配置库环境配置（兼容windows linux mac）
function(cpp_library name)
    message(STATUS "================ ${name} cpp_library =================")
    message("CMAKE_CURRENT_LIST_DIR = ${CMAKE_CURRENT_LIST_DIR}")
    option(${name}_SHARED "OFF is static cpp_library" ON)
    message("${name}_SHARED = ${${name}_SHARED}")
    option(TEST_FIND "ON is test find_pakage" OFF)

    set(TYPE STATIC)

    if(${name}_SHARED)
        set(TYPE SHARED)

        if(WIN32)
            set(WINDOWS_EXPORT_ALL_SYMBOLS ON)
        endif()
    endif()

    get_src_include()

    add_library(${name} ${TYPE}
        ${UI_FILES}
        ${UIC_HEADER}
        ${QRC_FILES}
		${QRC_SOURCE_FILES}
		
		${PROTO_FILES}
        ${PROTO_CC_FILE}
        ${PROTO_HREADER_FILE}
		
		${SRC}
        ${H_FILE_I}
    )


    if(NOT version)
        set(version 1.0)
    endif()

    set_cpp(${name})

    if(${name}_SHARED)
        target_compile_definitions(${name} PRIVATE ${name}_EXPORTS)
    else()
        target_compile_definitions(${name} PRIVATE ${name}_STATIC)
    endif()

    # 设置安装的头文件
    set_target_properties(${name} PROPERTIES
        PUBLIC_HEADER "${H_FILE_I}"
    )

    install(TARGETS ${name}
        EXPORT ${name}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PUBLIC_HEADER DESTINATION include
    )

    set(CONF_VER_DIR ${OUT_LIB_PATH}/config/${name}-${version})
    string(REPLACE "\\" "/" CONF_VER_DIR ${CONF_VER_DIR})

    # 支持find_package
    # 生成并安装配置文件
    instaLl(EXPORT ${name} FILE ${name}Config.cmake
        DESTINATION ${CONF_VER_DIR}
    )

    #
    # 版本文件
    set(CONF_VER_FILE
        ${OUT_LIB_PATH}/config/${name}-${version}/${name}ConfigVersion.cmake)

    string(REPLACE "\\" "/" CONF_VER_FILE ${CONF_VER_FILE})

    message("CONF_VER_FILE = ${CONF_VER_FILE}")
    include(CMakePackageConfigHelpers)
    write_basic_package_version_file(
        ${CONF_VER_FILE}
        VERSION ${version}
        COMPATIBILITY SameMajorVersion # 版本兼容问题
    )

    install(FILES ${CONF_VER_FILE}
        DESTINATION lib/config/${name}-${version}
    )
    message(STATUS "==================================================================")
endfunction()

function(cpp_execute name)
    message(STATUS "================ ${name} cpp_execute =================")
	get_src_include()

    # 添加执行程序
    add_executable(${name}	
		${UI_FILES}
        ${UIC_HEADER}
		${QRC_FILES}
		${QRC_SOURCE_FILES}
        ${RC_FILE}
		
		${PROTO_FILES}
        ${PROTO_CC_FILE}
        ${PROTO_HREADER_FILE}
		
		${SRC}
        ${H_FILE_I}
    )

    # 设置配置信息
    set_cpp(${name})

    # 第二种 链接的方式 推荐第一种
    math(EXPR size "${ARGC}-1")

    if(size GREATER 0)
        foreach(i RANGE 1 ${size})
            message("target_link_libraries ${ARGV${i}}")
            set(lib_name ${ARGV${i}})
            target_link_libraries(${name} ${lib_name}${debug_postfix})
        endforeach()
    endif()

    message(STATUS "==================================================================")
endfunction()

#########################################额外的工具函数###################################
function(print_list list)
    foreach(val IN LISTS ${list})
        message("val = ${val}")
    endforeach()
endfunction()

function(add_include_to_header header_file include_file)
    # 检查文件是否存在
    if(NOT EXISTS "${header_file}")
        message(FATAL_ERROR "Header file ${header_file} does not exist!")
    endif()

    # 读取原始文件内容
    file(READ "${header_file}" original_content)

    # 构建包含的内容
    set(include_statement "#pragma once\n#include \"${include_file}\"\n")

     # 检查是否已经包含该文件
    string(FIND "${original_content}" "${include_statement}" include_found)

    # 如果没有找到包含语句，则添加
    if(include_found EQUAL -1)
        # 将新内容写入文件
        file(WRITE "${header_file}" "${include_statement}${original_content}")
        message(STATUS "Added include for ${include_file} to ${header_file}")
    else()
        message(STATUS "Include for ${include_file} already exists in ${header_file}")
    endif()
endfunction()

function(add_include_to_headers header_list include_file)
    foreach(header_file IN LISTS ${header_list}) 
        add_include_to_header(${header_file} ${include_file})
    endforeach()
endfunction()

function(replace_class_name_in_header header_file class_name macro_name)
    # 确保头文件存在
    if(NOT EXISTS "${header_file}")
        message(FATAL_ERROR "Header file ${header_file} does not exist!")
    endif()

    # 读取头文件内容
    file(READ "${header_file}" original_content)

    # 替换类名
    string(REPLACE "class ${class_name} final" "class ${macro_name} ${class_name} final" modified_content "${original_content}")

    # 写回到头文件
    file(WRITE "${header_file}" "${modified_content}")

    message(STATUS "Replaced class name '${class_name}' with '${macro_name}' in ${header_file}")
endfunction()

function(replace_class_name_in_headers header_list class_name macro_name)
    foreach(header_file IN LISTS ${header_list})
        replace_class_names(${header_file} ${class_name} ${macro_name})
    endforeach()
endfunction()

function(replace_class_names header_file class_names macro_name)
    foreach(class_name IN LISTS ${class_names})
        #message("class_name = ${class_name}")    
        replace_class_name_in_header(${header_file} ${class_name} ${macro_name})
    endforeach()
endfunction()


function(watchdog name timeout)
    message(STATUS "================ ${name} watchdog =================")
    # 设置被守护的程序
    set(WATCHDOG_EXECUTABLE "${name}")

    # 设置守护程序的路径
    set(WATCHDOG_PROGRESS "watchdog")

    if(WIN32)
        # 设置脚本文件路径
        set(script_file ${OUT_RUN_PATH}/${WATCHDOG_PROGRESS}_${WATCHDOG_EXECUTABLE}.bat)
        message(STATUS "script_file = ${script_file}")
		message(STATUS "timeout = ${timeout}")

        # 设置脚本内容
        file(WRITE ${script_file} "@echo off\n")
        file(APPEND ${script_file} "setlocal\n")
        file(APPEND ${script_file} "${WATCHDOG_PROGRESS}.exe ${timeout} ${WATCHDOG_EXECUTABLE}.exe\n")
	elseif(APPLE OR UNIX)
		# ///////////////////////////start//////////////////////////////
		message("+++++++++++++++++++++++start++++++++++++++++++++")
		# 设置脚本文件路径
        set(script_file ${OUT_RUN_PATH}/${WATCHDOG_PROGRESS}_${WATCHDOG_EXECUTABLE}.sh)
		message(STATUS "script_file = ${script_file}")
		message(STATUS "timeout = ${timeout}")
		 
		# 设置脚本内容
        file(WRITE ${script_file} "#!/bin/bash\n")
        # file(APPEND ${script_file} "export LD_LIBRARY_PATH=${OUT_LIB_PATH}\n")
        file(APPEND ${script_file} "./${WATCHDOG_PROGRESS} ${timeout} ./${WATCHDOG_EXECUTABLE}\n")
		
		 # 设置文件权限
		# execute_process(COMMAND chmod +x ${script_file} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
		
        file(CHMOD ${script_file} FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
												   GROUP_READ GROUP_WRITE GROUP_EXECUTE
												   WORLD_READ WORLD_WRITE WORLD_EXECUTE)
		message("+++++++++++++++++++++++++++++++++++++++++++++++++")
		# ////////////////////////////stop//////////////////////////////
		
		# message("+++++++++++++++++++++++stop++++++++++++++++++++")
		# # 设置脚本文件路径
        # set(script_file ${OUT_RUN_PATH}/${WATCHDOG_PROGRESS}_${WATCHDOG_EXECUTABLE}_kill.sh)
		# message(STATUS "script_file = ${script_file}")
		 
		# # 设置脚本内容
        # file(WRITE ${script_file} "#!/bin/bash\n")
        # # file(APPEND ${script_file} "export LD_LIBRARY_PATH=${OUT_LIB_PATH}\n")
        # file(APPLEND ${script_file} "pkill ${WATCHDOG_PROGRESS}\n")
		
		 # # 设置文件权限
		# # execute_process(COMMAND chmod +x ${script_file} WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
		
        # file(CHMOD ${script_file} FILE_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE 
												   # GROUP_READ GROUP_WRITE GROUP_EXECUTE
												   # WORLD_READ WORLD_WRITE WORLD_EXECUTE)
		# message("+++++++++++++++++++++++++++++++++++++++++++++++++")
	endif()
    message(STATUS "===================================================")

endfunction()

########################################################################################