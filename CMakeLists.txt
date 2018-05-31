cmake_minimum_required(VERSION 3.6)
project(dwcpnc)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-O5 -ffast-math -Wall")
#set(CMAKE_C_FLAGS "-ffast-math -Wall")

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

set(EZNCDF_SOURCE
        ${CMAKE_SOURCE_DIR}/src/ezncdf/ezncdf.c
        ${CMAKE_SOURCE_DIR}/src/ezncdf/geo_math.c
        )

add_library(ezncdf STATIC ${EZNCDF_SOURCE})
#target_link_libraries(ezncdf netcdf)

set(VBN_SOURCE
        ${CMAKE_SOURCE_DIR}/src/vbn/vbn.c
        ${CMAKE_SOURCE_DIR}/src/vbn/array_product.c
        )
add_library(vbn STATIC ${VBN_SOURCE})

set(DWCPNC_SOURCE
        src/dwcpn/dwcpnc.c
        src/dwcpn/solar.c
        src/dwcpn/profile.c
        src/dwcpn/penguin.c
        src/dwcpn/irradiance.c
    )
add_library(dwcpnc STATIC ${DWCPNC_SOURCE})
target_link_libraries(dwcpnc m)


add_executable(run_pp src/main.c src/args.c src/args.h)
target_link_libraries(run_pp ezncdf)
target_link_libraries(run_pp vbn)
target_link_libraries(run_pp dwcpnc)