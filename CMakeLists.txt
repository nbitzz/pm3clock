cmake_minimum_required(VERSION 3.23)
project(pm3clock)
include(FetchContent)
find_package(CURL)

FetchContent_Declare(raylib
    PREFIX raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
)

FetchContent_MakeAvailable(raylib)

target_compile_definitions(raylib
    PUBLIC
        # AlertCA uses jpgs not png
        SUPPORT_FILEFORMAT_JPG=1
)

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