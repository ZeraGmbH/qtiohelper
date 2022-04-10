#set project name and language
project(actuasense LANGUAGES CXX
    VERSION ${REPO_VERSION}
)

#sum up project Files 
file(GLOB SOURCES 
    ${PROJECT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/QActuaSense
    )

file(GLOB RESOURCES 
    ${PROJECT_SOURCE_DIR}/*.qrc
    )

#create library 
add_library(actuasense SHARED
    ${SOURCES}
    ${HEADER}
    ${RESOURCES}
    )

target_link_libraries(actuasense 
    PRIVATE
    Qt5::Core
    )

# announce headers - target perspective
target_include_directories(actuasense
    PUBLIC
    $<INSTALL_INTERFACE:include/qtiohelpers/actuasense>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

#set target Version
set_target_properties(actuasense PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(actuasense PROPERTIES SOVERSION ${PROJECT_VERSION_MAJOR})

#generate export header
include(GenerateExportHeader)
generate_export_header(actuasense)

#install export header
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/actuasense_export.h
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtiohelpers/actuasense
    )

# install library
install(TARGETS actuasense
    EXPORT actuasenseExport
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )

# configure *ConfigVersion.cmake
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    actuasenseConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY SameMinorVersion
    )

# configure *Config.cmake
configure_file(actuasenseConfig.cmake.in actuasenseConfig.cmake @ONLY)

# install *Config(Version).cmake
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/actuasenseConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/actuasenseConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/actuasense
    )

# install targets cmake-files
install(EXPORT actuasenseExport
    FILE actuasenseTargets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/actuasense
    )

# install public headers
install(
    FILES ${HEADER}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/qtiohelpers/actuasense
    )
