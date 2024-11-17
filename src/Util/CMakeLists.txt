set(LIB util)
add_library(${LIB})
add_library(${PROJECT_NAME}::${LIB} ALIAS ${LIB})

set(PUBLIC_HEADER
  Util.h
  Singleton.h
  Config.h
)

target_sources(
  ${LIB}
  PRIVATE
  Util.cpp
  Config.cpp
)

target_include_directories(
  ${LIB}
  PUBLIC
  ${SRC_DIR}
)
find_package(yaml-cpp REQUIRED)

target_link_libraries(
  ${LIB}
  PUBLIC
  yaml-cpp::yaml-cpp
  ${PROJECT_NAME}::log
)