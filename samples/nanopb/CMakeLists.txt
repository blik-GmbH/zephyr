include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)
project(NONE)

# Extend zephyrs minimal stdint for missing types
include_directories("ext/nanopb_syshdr")
add_definitions(-DPB_SYSTEM_HEADER=\"pb_syshdr.h\")

# Protobuf definitions
add_subdirectory (ext/blik)

# Generated protobufs
include_directories("${CMAKE_CURRENT_BINARY_DIR}/ext/blik")

# Make nanopb headers available
include_directories("ext/nanopb")

# Mark them as to be generated and not necessarily available right away
set_source_files_properties(${PROTO_SRCS} ${PROTO_HDRS}
    PROPERTIES GENERATED TRUE)

# Build sample with protobuf sources
target_sources(app PRIVATE src/main.c ${PROTO_SRCS} ${PROTO_HDRS})

# Ensure to generate the protobuf files before building the sample
add_dependencies(app blik)
