cmake_minimum_required(VERSION 2.8.8)
set(TARGET db_test)
project(${TARGET})


set(TT_DIR /opt/TimesTen/timesten)

set(RX_CXX_FLAGS "-fPIC -DPIC -pipe -W -Wall -Wextra -Wpointer-arith -Wno-unused-parameter -Wunused-function -Wunused-variable -Wunused-value -Werror -Wcast-qual -Wa,--execstack -ggdb -Wno-strict-aliasing -fno-omit-frame-pointer -fdata-sections -ffunction-sections -fno-strict-aliasing -fvisibility=hidden -Wno-attributes -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free -Wl,--export-dynamic -Werror=format-security")
message("-- Build optimized version")
set(RX_CXX_FLAGS "${RX_CXX_FLAGS} -O1 -finline-functions -finline-functions-called-once")
set(RX_C_FLAGS "${RX_CXX_FLAGS} -Wstrict-prototypes -Wmissing-prototypes -Wold-style-definition")
if(CMAKE_COMPILER_IS_GNUCXX)
    set(RX_C_FLAGS "${RX_C_FLAGS} -std=gnu99")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${RX_C_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${RX_C_FLAGS}")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${RX_CXX_FLAGS} -fno-weak")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${RX_CXX_FLAGS} -fno-weak")

SET(CMAKE_EXE_LINK_STATIC_C_FLAGS "${CMAKE_EXE_LINK_STATIC_C_FLAGS} -Wl,--gc-sections")
SET(CMAKE_EXE_LINK_STATIC_CXX_FLAGS "${CMAKE_EXE_LINK_STATIC_CXX_FLAGS} -Wl,--gc-sections")

## build as debug version
set(CMAKE_BUILD_TYPE Debug)

set(${TARGET}_SRCS_COMMON
    src/test_dao.cpp
    src/test_dao.h
    src/test_utils.h
)

set(${TARGET}_SRCS
    ${${TARGET}_SRCS_COMMON}
    src/db_test.cpp
)

set(${TARGET}_SRCS_STMT
    ${${TARGET}_SRCS_COMMON}
    src/err_stmt.cpp
)

include_directories(
    ${TT_DIR}/include
)

add_executable(${TARGET}_tt ${${TARGET}_SRCS})
set_target_properties (${TARGET}_tt PROPERTIES COMPILE_DEFINITIONS "TT")
target_link_libraries(${TARGET}_tt
    ${TT_DIR}/lib/libtten.so
)

add_executable(${TARGET}_pq ${${TARGET}_SRCS})
set_target_properties (${TARGET}_pq PROPERTIES COMPILE_DEFINITIONS "PQ")

target_link_libraries(${TARGET}_pq
    /usr/lib64/libodbc.so.2
)

add_executable(${TARGET}_pq_stmt ${${TARGET}_SRCS_STMT})
set_target_properties (${TARGET}_pq_stmt PROPERTIES COMPILE_DEFINITIONS "PQ")

target_link_libraries(${TARGET}_pq_stmt
    /usr/lib64/libodbc.so.2
)
