cmake_minimum_required(VERSION 3.16)

project(qtiohelpers LANGUAGES CXX)
set(REPO_VERSION ${PROJECT_VERSION})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")

set(PROJECT_VERSION_MAJOR "0")
set(PROJECT_VERSION_MINOR "4")
set(PROJECT_VERSION_PATCH "0")
set(PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")
set(REPO_VERSION ${PROJECT_VERSION})

include(FeatureSummary)
include(GNUInstallDirs)

set(QT_MAJOR_VER "5" CACHE STRING "Qt major version")
set(QTX "Qt${QT_MAJOR_VER}")

find_package(${QTX} REQUIRED COMPONENTS Core Network SerialPort)

enable_testing()

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
