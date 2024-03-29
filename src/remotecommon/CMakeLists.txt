#set project name and language
project(remotecommon LANGUAGES CXX
    VERSION ${REPO_VERSION}
)

#sum up project Files 
file(GLOB SOURCES 
    ${PROJECT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/*.h
    )

#create library 
add_library(remotecommon SHARED
    ${SOURCES}
    ${HEADER}
    )

target_link_libraries(remotecommon 
    PRIVATE
    Qt5::Core
    Qt5::Network
    )

# announce headers - target perspective
target_include_directories(remotecommon
    PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qtiohelpers/remotecommon>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

#set target Version
set_target_properties(remotecommon PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(remotecommon PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

#generate export header
include(GenerateExportHeader)
generate_export_header(remotecommon)

#install export header
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/remotecommon_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtiohelpers/remotecommon
    )

# install library
install(TARGETS remotecommon
    EXPORT remotecommonExport
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

# configure *ConfigVersion.cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    remotecommonConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMajorVersion
    )

# configure *Config.cmake
configure_file(remotecommonConfig.cmake.in remotecommonConfig.cmake @ONLY)

# install *Config(Version).cmake
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/remotecommonConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/remotecommonConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/remotecommon
    )

# install targets cmake-files
install(EXPORT remotecommonExport
    FILE remotecommonTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/remotecommon
    )

# install public headers
install(
    FILES ${HEADER}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtiohelpers/remotecommon
    )
