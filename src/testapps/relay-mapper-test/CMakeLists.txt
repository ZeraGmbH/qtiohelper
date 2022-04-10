project(relaymappertest VERSION 1.0 LANGUAGES CXX)

# Tell CMake to call moc automaticly
set(CMAKE_AUTOMOC ON)

include(FeatureSummary)
include(GNUInstallDirs)

find_package(Qt5 REQUIRED COMPONENTS Core)

# We're using ${PROJECT_NAME} for target name but it's not necessary
add_executable(
    relaymappertest
    main.cpp        # Our example project consists only of one .cpp file
)

target_link_libraries(
    relaymappertest
    PRIVATE
        Qt5::Core
	relaymapper
)

install(TARGETS relaymappertest
    EXPORT relaymappertest
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
