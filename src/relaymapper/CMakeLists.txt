#set project name and language
project(relaymapper LANGUAGES CXX
    VERSION ${REPO_VERSION}
)

#sum up project Files 
file(GLOB SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/QRelayMapper
    ${CMAKE_CURRENT_SOURCE_DIR}/QRelaySequencer
    ${CMAKE_CURRENT_SOURCE_DIR}/QRelaySerializer
    )

#create library 
add_library(relaymapper SHARED
    ${SOURCES}
    ${HEADER}
    )

target_link_libraries(relaymapper 
    PRIVATE
    Qt5::Core
    )

# announce headers - target perspective
target_include_directories(relaymapper
    PUBLIC
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/qtiohelpers/relaymapper>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

#set target Version
set_target_properties(relaymapper PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(relaymapper PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

#generate export header
include(GenerateExportHeader)
generate_export_header(relaymapper)

#install export header
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/relaymapper_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtiohelpers/relaymapper
    )

# install library
install(TARGETS relaymapper
    EXPORT relaymapperExport
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

# configure *ConfigVersion.cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    relaymapperConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMajorVersion
    )

# configure *Config.cmake
configure_file(relaymapperConfig.cmake.in relaymapperConfig.cmake @ONLY)

# install *Config(Version).cmake
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/relaymapperConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/relaymapperConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/relaymapper
    )

# install targets cmake-files
install(EXPORT relaymapperExport
    FILE relaymapperTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/relaymapper
    )

# install public headers
install(
    FILES ${HEADER}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtiohelpers/relaymapper
    )
