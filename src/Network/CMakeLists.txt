set(LIB_NAME network)
add_library(${LIB_NAME})
add_library(${PROJECT_NAME}::${LIB_NAME} ALIAS ${LIB_NAME})

add_custom_command(
        OUTPUT   http11_parser.rl.cpp   httpclient_parser.rl.cpp
        COMMAND ragel -G2 -C ${CMAKE_CURRENT_SOURCE_DIR}/httpclient_parser.rl -o ${CMAKE_CURRENT_BINARY_DIR}/httpclient_parser.rl.cpp
        COMMAND ragel -G2 -C ${CMAKE_CURRENT_SOURCE_DIR}/http11_parser.rl -o ${CMAKE_CURRENT_BINARY_DIR}/http11_parser.rl.cpp
        DEPENDS httpclient_parser.rl http11_parser.rl
)

set(PUBLIC_HEADER
  Address.h
  Socket.h
  ByteArray.h
  Http.h
  http_parser.h
  TcpServer.h
  Stream.h
)

target_sources(
  ${LIB_NAME}
  PRIVATE
  Address.cpp
  Socket.cpp
  ByteArray.cpp
  Http.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/http11_parser.rl.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/httpclient_parser.rl.cpp
  http_parser.cpp
  TcpServer.cpp
  Stream.cpp
)

target_include_directories(
  ${LIB_NAME}
  PUBLIC
  ${SRC_DIR}
)

target_link_libraries(
  ${LIB_NAME}
  PUBLIC
  ${PROJECT_NAME}::log
  ${PROJECT_NAME}::core
)