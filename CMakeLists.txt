cmake_minimum_required(VERSION 3.16)
project(calc_server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRC
    src/main.cpp
    src/server/server.cpp
    src/utils/format.cpp
    third_party/GB-Mahjong/console/console.cpp
    third_party/GB-Mahjong/console/print.cpp
    third_party/GB-Mahjong/mahjong/fan.cpp
    third_party/GB-Mahjong/mahjong/handtiles.cpp
    third_party/GB-Mahjong/mahjong/pack.cpp
    third_party/GB-Mahjong/mahjong/tile.cpp
    third_party/GB-Mahjong/unit_test.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/third_party/cpp-httplib
    ${CMAKE_SOURCE_DIR}/third_party/cxxopts/include
    ${CMAKE_SOURCE_DIR}/third_party/json/include
    ${CMAKE_SOURCE_DIR}/third_party/json/single_include
    ${CMAKE_SOURCE_DIR}/third_party/GB-Mahjong/console
    ${CMAKE_SOURCE_DIR}/third_party/GB-Mahjong/mahjong
)

add_executable(calc_server ${SRC})

set_source_files_properties(
    third_party/GB-Mahjong/unit_test.cpp
    PROPERTIES COMPILE_DEFINITIONS "main=gbmj_unit_main"
)
