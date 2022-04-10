project(spi-remote-server VERSION 1.0 LANGUAGES CXX)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DQT_QML_DEBUG -DQT_DECLARATIVE_DEBUG")
endif()

include(FeatureSummary)
include(GNUInstallDirs)

find_package(Qt5 REQUIRED COMPONENTS Core Network)

# We're using ${PROJECT_NAME} for target name but it's not necessary
add_executable(
    spi-remote-server
    main.cpp
)

target_link_libraries(
    spi-remote-server
    PUBLIC
    Qt5::Core
    Qt5::Network
    remotecommon
    spidevice
)

install(TARGETS spi-remote-server
    EXPORT spi-remote-server
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
