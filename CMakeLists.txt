##-DCMAKE_TOOLCHAIN_FILE=Toolchain.cmake
#-DCMAKE_BUILD_TYPE=Debug
#-DCMAKE_BUILD_TYPE=Release

project(APB)
cmake_minimum_required(VERSION 2.8.8)

if(WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fexec-charset=GBK -fpermissive")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fexec-charset=UTF-8 -fpermissive")
endif()
option(BUILD_SHARED_LIBS "build shared libs" ON)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug" )
    set(DIR_LINK "${PROJECT_BINARY_DIR}/binD")
else()
    set(DIR_LINK "${PROJECT_BINARY_DIR}/bin")
endif()
set(LIBRARY_OUTPUT_PATH ${DIR_LINK})
set(EXECUTABLE_OUTPUT_PATH ${DIR_LINK})

message(STATUS "CMAKE_BUILD_TYPE:${CMAKE_BUILD_TYPE}")
message(STATUS "CMAKE_TOOLCHAIN_FILE:${CMAKE_TOOLCHAIN_FILE}")
message(STATUS "BUILD_SHARED_LIBS:${BUILD_SHARED_LIBS}")

set(DIR_ORACLE /usr/local/oracle)
link_directories(${DIR_ORACLE}/lib)
link_directories(${DIR_LINK})


find_package(Qt5Core)
find_package(Qt5Network)
find_package(Qt5Widgets)
find_package(Qt5SerialPort)
find_package(Qt5Sql)
set(CMAKE_AUTOMOC ON)

include_directories(
    ${Qt5Widgets_INCLUDE_DIRS}
    ${Qt5Network_INCLUDE_DIRS}
    ${Qt5Core_INCLUDE_DIRS}
    ${Qt5SerialPort_INCLUDE_DIRS}
    ${Qt5Sql_INCLUDE_DIRS}
    ${DIR_ORACLE}/include
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/RTCM2RNX
)

set(SRC_LIST
    main.cpp
    OracleWrapper/oci_wrapper.cpp
    OracleWrapper/corsdbmanager.cpp
    RTCM2RNX/func_Cmn_Rtcm.cpp
    RTCM2RNX/func_Convrnx.cpp
    RTCM2RNX/func_RinexOut.cpp
    RTCM2RNX/func_Rtcm.cpp
    RTCM2RNX/func_Rtcm2De.cpp
    RTCM2RNX/func_Rtcm3De.cpp
    RTCM2RNX/func_Rtcm3En.cpp
    RTCM2RNX/func_Time.cpp
    RTCM2RNX/func_DataQC.cpp
    RTCM2RNX/func_MP.cpp
    RTCM2RNX/rtcm3dec.cpp
    RTCM2RNX/RTCM2RNX.cpp
    util.cpp
    NMEA0183/nmea0183dec.cpp
    DataXml/dbStruct.cpp
    DataXml/tableStruct.cpp
    DataXml/toCorCalculateRecXml.cpp
    DataXml/toCorSatDelXml.cpp
    DataXml/toCorSatTotalXml.cpp
    DataXml/toCorStatusRec.cpp
    DataXml/toMntAlarmDetXml.cpp
    DataXml/toMntAlarmTalXml.cpp
    DataXml/toMntCorExtXml.cpp
    DataXml/toMntTimestampXml.cpp
    DataXml/toString.cpp
)

set(FILE_DLL
        Qt5::Widgets
        Qt5::Core
        Qt5::Network
        Qt5::SerialPort
        Qt5::Sql
        libclntsh.so
        libocilib.so
        libnnz12.so
)
if(WIN32)
    set(QT_USE_QTMAIN TRUE)
    add_executable(${PROJECT_NAME} WIN32 ${SRC_LIST})
else()
    add_executable(${PROJECT_NAME} ${SRC_LIST})
endif()
target_link_libraries(${PROJECT_NAME} ${FILE_DLL})
