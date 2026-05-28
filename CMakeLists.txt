cmake_minimum_required(VERSION 3.23)
find_package(CURL)
find_library(raylib, raylib)
project(pm3clock)
add_executable(pm3clock)
target_sources(pm3clock
    PRIVATE
        main.cpp
        weather.cpp
        alertca.cpp
)
target_link_libraries(pm3clock
    PRIVATE
        CURL::libcurl
        raylib
)