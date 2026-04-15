cmake_minimum_required(VERSION 3.15)

set(PROJECT_LANGUAGES CXX)
if(APPLE)
    list(APPEND PROJECT_LANGUAGES OBJC)
endif()

project(CloudFileCli LANGUAGES ${PROJECT_LANGUAGES})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OBJC_STANDARD 11)
    set(BACKEND_SOURCES src/backends/mac.cpp src/backends/mac.m)
elseif(WIN32)
    set(BACKEND_SOURCES src/backends/win.cpp)
else()
    message(FATAL_ERROR "Unsupported platform: only macOS and Windows backends are implemented")
endif()

add_executable(cloudfile src/main.cpp src/cloudfile.cpp ${BACKEND_SOURCES})

target_include_directories(cloudfile PRIVATE src)

if(APPLE)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    find_library(FILEPROVIDER_FRAMEWORK FileProvider REQUIRED)
    target_link_libraries(cloudfile ${FOUNDATION_FRAMEWORK} ${FILEPROVIDER_FRAMEWORK})
elseif(WIN32)
    target_link_libraries(cloudfile CldApi)
endif()
