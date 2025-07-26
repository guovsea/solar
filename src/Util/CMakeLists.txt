set(LIB_NAME util)
add_library(${LIB_NAME})
add_library(${PROJECT_NAME}::${LIB_NAME} ALIAS ${LIB_NAME})

set(PUBLIC_HEADER
  Util.h
  Singleton.h
  Config.h
  CustomConfigVar.h
  Endian.h
)

target_sources(
  ${LIB_NAME}
  PRIVATE
  Util.cpp
  Config.cpp
)

target_include_directories(
  ${LIB_NAME}
  PUBLIC
  ${SRC_DIR}
)
find_package(yaml-cpp REQUIRED)

target_link_libraries(
  ${LIB_NAME}
  PUBLIC
  yaml-cpp
  ${PROJECT_NAME}::log
)