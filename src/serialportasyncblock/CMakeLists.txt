cmake_minimum_required(VERSION 3.1)

#set project name and language
project(serialportasyncblock LANGUAGES CXX
    VERSION ${REPO_VERSION}
)


#Automoc and RCC ON. Needed in QT projects
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#C++ 11 Flag
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



#Adding cmake functions like feature_summary()
include(FeatureSummary)
include(GNUInstallDirs)

#Find dependecies
find_package(Qt5 COMPONENTS Core REQUIRED)
find_package(Qt5SerialPort REQUIRED)

#sum up project Files 
file(GLOB SOURCES 
    ${PROJECT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/QtSerialPortAsyncBlock
    )

file(GLOB RESOURCES 
    ${PROJECT_SOURCE_DIR}/*.qrc
    )

#create library 
add_library(serialportasyncblock SHARED
    ${SOURCES}
    ${HEADER}
    ${RESOURCES}
    )



target_link_libraries(serialportasyncblock 
    PRIVATE
    Qt5::Core
    Qt5::SerialPort
    )


# announce headers - target perspective
target_include_directories(serialportasyncblock
    PUBLIC
    $<INSTALL_INTERFACE:include/qtiohelpers/serialportasyncblock>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

#set target Version
set_target_properties(serialportasyncblock PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(serialportasyncblock PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

#generate export header
include(GenerateExportHeader)
generate_export_header(serialportasyncblock)

#install export header
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/serialportasyncblock_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtiohelpers/serialportasyncblock
    )



# install library
install(TARGETS serialportasyncblock
    EXPORT serialportasyncblockExport
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

# configure *ConfigVersion.cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    serialportasyncblockConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMinorVersion
    )

# configure *Config.cmake
configure_file(serialportasyncblockConfig.cmake.in serialportasyncblockConfig.cmake @ONLY)

# install *Config(Version).cmake
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/serialportasyncblockConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/serialportasyncblockConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/serialportasyncblock
    )

# install targets cmake-files
install(EXPORT serialportasyncblockExport
    FILE serialportasyncblockTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/serialportasyncblock
    )

# install public headers
install(
    FILES ${HEADER}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtiohelpers/serialportasyncblock
    )

# spawn out some info on configuration
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

