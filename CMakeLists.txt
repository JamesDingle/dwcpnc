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
#
#add_library(ezncdf SHARED ${EZNCDF_SOURCE})
#target_link_libraries(ezncdf PUBLIC netcdf mvec m)
#
set(VBN_SOURCE
        ${CMAKE_SOURCE_DIR}/src/vbn/vbn.c
        ${CMAKE_SOURCE_DIR}/src/vbn/array_product.c
        )
#add_library(vbn STATIC ${VBN_SOURCE})
#
set(DWCPNC_SOURCE
        src/dwcpn/dwcpnc.c
        src/dwcpn/solar.c
        src/dwcpn/profile.c
        src/dwcpn/penguin.c
        src/dwcpn/irradiance.c
    )
#add_library(dwcpnc STATIC ${DWCPNC_SOURCE})
#target_link_libraries(dwcpnc PUBLIC m)

set(THREADPOOL_SOURCE
        src/threadpool/threadpool.c
        )


set(RUN_PP_SOURCE
        src/main.c
        src/args.c
        )

add_executable(run_pp ${EZNCDF_SOURCE} ${VBN_SOURCE} ${DWCPNC_SOURCE} ${RUN_PP_SOURCE})
target_link_libraries(run_pp netcdf)
target_link_libraries(run_pp m)
#target_link_libraries(run_pp ${LIBRARY_OUTPUT_PATH}/libvbn.a)
#target_link_libraries(run_pp ${LIBRARY_OUTPUT_PATH}/libezncdf.a)
#target_link_libraries(run_pp ${LIBRARY_OUTPUT_PATH}/libdwcpnc.a)

set(RUN_PP_PARALLEL_SOURCE
        src/main_parallel.c
        src/args.c
        )
add_executable(run_pp_parallel ${EZNCDF_SOURCE} ${VBN_SOURCE} ${DWCPNC_SOURCE} ${THREADPOOL_SOURCE} ${RUN_PP_PARALLEL_SOURCE})

target_link_libraries(run_pp_parallel netcdf m pthread)

set(RUN_PP_PARALLEL_CHUNK_SOURCE
        src/main_parallel_chunk.c
        src/args.c
        )
add_executable(run_pp_parallel_chunk ${EZNCDF_SOURCE} ${VBN_SOURCE} ${DWCPNC_SOURCE} ${THREADPOOL_SOURCE} ${RUN_PP_PARALLEL_CHUNK_SOURCE})

target_link_libraries(run_pp_parallel_chunk netcdf m pthread)