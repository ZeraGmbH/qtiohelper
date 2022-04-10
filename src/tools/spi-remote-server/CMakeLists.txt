project(spi-remote-server VERSION 1.0 LANGUAGES CXX)

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
