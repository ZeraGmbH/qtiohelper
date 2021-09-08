cmake_minimum_required(VERSION 3.1)

#set project name and language
project(bitinputpoller LANGUAGES CXX
    VERSION 1.00
#    VERSION ${REPO_VERSION}
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
find_package(Qt5 COMPONENTS Core CONFIG REQUIRED)

#sum up project Files 
file(GLOB SOURCES 
    ${PROJECT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/QtBitInputPoller
    )

file(GLOB RESOURCES 
    ${PROJECT_SOURCE_DIR}/*.qrc
    )

#create library 
add_library(bitinputpoller SHARED
    ${SOURCES}
    ${HEADER}
    ${RESOURCES}
    )

target_link_libraries(bitinputpoller 
    PRIVATE
    Qt5::Core
    )


# announce headers - target perspective
target_include_directories(bitinputpoller
    PUBLIC
    $<INSTALL_INTERFACE:include/qtiohelpers/bitinputpoller>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

#set target Version
set_target_properties(bitinputpoller PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(bitinputpoller PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

#generate export header
include(GenerateExportHeader)
generate_export_header(bitinputpoller)

#install export header
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/bitinputpoller_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtiohelpers/bitinputpoller
    )



# install library
install(TARGETS bitinputpoller
    EXPORT bitinputpollerExport
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

# configure *ConfigVersion.cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    bitinputpollerConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMinorVersion
    )

# configure *Config.cmake
configure_file(bitinputpollerConfig.cmake.in bitinputpollerConfig.cmake @ONLY)

# install *Config(Version).cmake
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/bitinputpollerConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/bitinputpollerConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bitinputpoller
    )

# install targets cmake-files
install(EXPORT bitinputpollerExport
    FILE bitinputpollerTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bitinputpoller
    )

# install public headers
install(
    FILES ${HEADER}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtiohelpers/bitinputpoller
    )

# spawn out some info on configuration
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

