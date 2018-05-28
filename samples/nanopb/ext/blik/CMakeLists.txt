cmake_minimum_required(VERSION 2.8)
project(NONE)

# NANOPB LIBRARY
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../nanopb/extra)
find_package(Nanopb REQUIRED)
include_directories(${NANOPB_INCLUDE_DIRS})

# Generate proto
nanopb_generate_cpp(PROTO_SRCS PROTO_HDRS RELPATH . blik.proto sub/unlucky.proto)

# Mark them as to be generated and not necessarily available right away
set_source_files_properties(${PROTO_SRCS} ${PROTO_HDRS}
    PROPERTIES GENERATED TRUE)

# Make the source- and header files globally available
SET(PROTO_SRCS  "${PROTO_SRCS}" CACHE INTERNAL "PROTO_SRCS")
SET(PROTO_HDRS  "${PROTO_HDRS}" CACHE INTERNAL "PROTO_HDRS")

# Create target and let it depend on generating the sources first
add_custom_target(blik DEPENDS ${PROTO_SRCS} ${PROTO_HDRS})
