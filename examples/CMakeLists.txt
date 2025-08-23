add_executable(echo_server)

target_sources(
    echo_server
    PRIVATE
    echo_server.cpp
)

target_link_libraries(echo_server
    PRIVATE
    solar::network
)

add_executable(http_server)

target_sources(
    http_server
    PRIVATE
    http_server.cpp
)

target_link_libraries(http_server
    PRIVATE
    solar::network
)

target_compile_options(http_server PRIVATE -O3)