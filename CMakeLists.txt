cmake_minimum_required(VERSION 3.23)
project(pm3clock)
include(FetchContent)
option(PM3_LIBRARIES "Path to PM3 libraries")

# include font files
function(include_bin p varname)
    file(READ ${p} binh HEX)
    string(REGEX REPLACE "([a-f0-9][a-f0-9])" "0x\\1," bint ${binh})
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${varname}.h" "unsigned char ${varname}[] = { ${bint} };")
endfunction()

include_bin("./fonts/DMSans-ExtraBold.ttf" dm_sans_extrabold_buf)
include_bin("./fonts/DMSans-SemiBold.ttf" dm_sans_semibold_buf)
# compile raylib

add_subdirectory(raylib)

if (DEFINED PM3_LIBRARIES)
    set(PLATFORM DRM)
    set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
    set(PC_CURL_INCLUDE_DIRS ${PM3_LIBRARIES})
    string(REGEX MATCH "/?([^/]*)/?$" L_DIR_NAME ${PM3_LIBRARIES})
    # todo maybe actually look for ld
    add_link_options("-Wl,--dynamic-linker=/init/efad/mp_sd/${CMAKE_MATCH_1}/ld-linux-armhf.so.3")
    target_compile_definitions(raylib
        PUBLIC
            # AlertCA uses jpgs not png
            SUPPORT_FILEFORMAT_JPG=1
            GRAPHICS_API_OPENGL_SOFTWARE=1
            DEFAULT_GRAPHIC_DEVICE_DRM="/dev/fd0"
    )
else()
    target_compile_definitions(raylib
        PUBLIC
            # AlertCA uses jpgs not png
            SUPPORT_FILEFORMAT_JPG=1
    )
endif()

find_package(CURL)


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