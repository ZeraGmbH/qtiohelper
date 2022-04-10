cmake_minimum_required(VERSION 3.1)

#set project name and language
project(qtiohelpers LANGUAGES CXX)
set(REPO_VERSION ${PROJECT_VERSION})

#Automoc and RCC ON. Needed in QT projects
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#Pin C++ version
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# yes we mean it for all targets
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

#Set project Version
set(PROJECT_VERSION_MAJOR "0")
set(PROJECT_VERSION_MINOR "4")
set(PROJECT_VERSION_PATCH "0")

set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(REPO_VERSION ${PROJECT_VERSION})

include(FeatureSummary)
include(GNUInstallDirs)

find_package(Qt5 REQUIRED COMPONENTS Core Network SerialPort)

enable_testing()

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
