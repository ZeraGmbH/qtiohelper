project(relaymappertest VERSION 1.0 LANGUAGES CXX)

enable_testing()
add_test(NAME    relaymappertest
         COMMAND relaymappertest)

add_executable(
    relaymappertest
    main.cpp
)

target_link_libraries(
    relaymappertest
    PRIVATE
        Qt5::Core
	relaymapper
)
