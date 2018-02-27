cmake_minimum_required(VERSION 3.6)
project(dwcpnc)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c dwcpnc.c dwcpnc.h solar.c solar.h profile.c profile.h penguin.c penguin.h)
add_executable(dwcpnc ${SOURCE_FILES})
target_link_libraries(dwcpnc m)