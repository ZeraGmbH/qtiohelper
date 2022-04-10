project(relaymappertest VERSION 1.0 LANGUAGES CXX)

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
