cmake_minimum_required(VERSION 3.16)
project(calc_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB SRC
    src/*.cpp
    src/server/*.cpp
    src/utils/*.cpp
    third_party/GB-Mahjong/console/*.cpp
    third_party/GB-Mahjong/mahjong/*.cpp
    third_party/GB-Mahjong/unit_test.cpp
)

add_executable(calc_server ${SRC})

target_include_directories(calc_server PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/cpp-httplib
    ${CMAKE_SOURCE_DIR}/third_party/cxxopts/include
    ${CMAKE_SOURCE_DIR}/third_party/json/include
    ${CMAKE_SOURCE_DIR}/third_party/json/single_include
    ${CMAKE_SOURCE_DIR}/third_party/GB-Mahjong/
    ${CMAKE_SOURCE_DIR}/third_party/GB-Mahjong/console
    ${CMAKE_SOURCE_DIR}/third_party/GB-Mahjong/mahjong
)

# Link Windows winsock library when building on Windows (MSVC/MinGW)
if (WIN32)
    target_link_libraries(calc_server PRIVATE ws2_32)
endif ()

set_source_files_properties(
    third_party/GB-Mahjong/unit_test.cpp
    PROPERTIES COMPILE_DEFINITIONS "main=gbmj_unit_main"
)
