#set project name and language
project(bitinputpoller LANGUAGES CXX
    VERSION ${REPO_VERSION}
)

#sum up project Files 
file(GLOB SOURCES 
    ${PROJECT_SOURCE_DIR}/*.cpp
    )

file(GLOB HEADER
    ${PROJECT_SOURCE_DIR}/*.h
    ${PROJECT_SOURCE_DIR}/QBitInputPoller
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
