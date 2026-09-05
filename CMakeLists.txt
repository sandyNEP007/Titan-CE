cmake_minimum_required(VERSION 3.22.1)

project(TitanCE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Titan-CE

    src/main.cpp
    src/board.cpp
    src/engine_search.cpp
    src/evaluation.cpp
    src/move.cpp
    src/movegen.cpp
    src/pst.cpp
    src/uci.cpp
    src/zobrist.cpp
)

target_include_directories(Titan-CE PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)