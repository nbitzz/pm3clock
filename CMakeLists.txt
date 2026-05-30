cmake_minimum_required(VERSION 3.23)
project(pm3clock)
include(FetchContent)

# include font files
function(include_bin p varname)
    file(READ ${p} binh HEX)
    string(REGEX REPLACE "([a-f0-9][a-f0-9])" "0x\\1," bint ${binh})
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${varname}.h" "unsigned char ${varname}[] = { ${bint} };")
endfunction()

include_bin("./fonts/DMSans-ExtraBold.ttf" dm_sans_extrabold_buf)
include_bin("./fonts/DMSans-SemiBold.ttf" dm_sans_semibold_buf)

find_package(CURL)

# compile raylib

add_subdirectory(raylib)

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