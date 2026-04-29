cmake_minimum_required(VERSION 3.16)
project(overlay VERSION 0.1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------------------------------
# Dependencies
# -------------------------------------------------------
find_package(zenohc REQUIRED)
find_package(zenohcxx REQUIRED)
find_package(Threads REQUIRED)

# -------------------------------------------------------
# liboverlay.so  (pre-built, from lib/)
# -------------------------------------------------------
add_library(overlay SHARED IMPORTED)
set_target_properties(overlay PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/liboverlay.so
    INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/include
)

# -------------------------------------------------------
# example_node
# -------------------------------------------------------
add_executable(example_node
    example/example_node.cpp
    transport/zenoh_transport.cpp
)

target_include_directories(example_node
    PRIVATE
        include
        transport
)

target_link_libraries(example_node
    PRIVATE
        overlay
        zenohcxx::zenohc
        Threads::Threads
)