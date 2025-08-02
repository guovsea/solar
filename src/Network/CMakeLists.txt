set(LIB_NAME network)
add_library(${LIB_NAME})
add_library(${PROJECT_NAME}::${LIB_NAME} ALIAS ${LIB_NAME})

set(PUBLIC_HEADER
  Address.h
  Socket.h
  ByteArray.h
  Http.h
)

target_sources(
  ${LIB_NAME}
  PRIVATE
  Address.cpp
  Socket.cpp
  ByteArray.cpp
  Http.cpp
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