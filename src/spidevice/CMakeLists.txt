cmake_minimum_required(VERSION 3.1)

#set project name and language
project(spidevice LANGUAGES CXX
    VERSION ${REPO_VERSION}
)

#C++ 11 Flag
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#Adding cmake functions like feature_summary()
include(FeatureSummary)
include(GNUInstallDirs)

#Find dependecies
find_package(Qt5 COMPONENTS Core Network CONFIG REQUIRED)

#sum up project Files 
file(GLOB SOURCES 
    ${PROJECT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/QSpiDevice
    )

file(GLOB RESOURCES 
    ${PROJECT_SOURCE_DIR}/*.qrc
    )

#create library 
add_library(spidevice SHARED
    ${SOURCES}
    ${HEADER}
    ${RESOURCES}
    )

target_link_libraries(spidevice 
    PUBLIC
    Qt5::Core
    Qt5::Network
    remotecommon
    )

# announce headers - target perspective
target_include_directories(spidevice
    PUBLIC
    $<INSTALL_INTERFACE:include/qtiohelpers/spidevice>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

#set target Version
set_target_properties(spidevice PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(spidevice PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

#generate export header
include(GenerateExportHeader)
generate_export_header(spidevice)

#install export header
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/spidevice_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtiohelpers/spidevice
    )

# install library
install(TARGETS spidevice
    EXPORT spideviceExport
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

# configure *ConfigVersion.cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    spideviceConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMinorVersion
    )

# configure *Config.cmake
configure_file(spideviceConfig.cmake.in spideviceConfig.cmake @ONLY)

# install *Config(Version).cmake
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/spideviceConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/spideviceConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spidevice
    )

# install targets cmake-files
install(EXPORT spideviceExport
    FILE spideviceTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spidevice
    )

# install public headers
install(
    FILES ${HEADER}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtiohelpers/spidevice
    )

# spawn out some info on configuration
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

