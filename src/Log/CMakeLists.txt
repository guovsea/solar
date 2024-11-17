
set(LIB log)
add_library(${LIB})
add_library(${PROJECT_NAME}::${LIB} ALIAS ${LIB})

set(PUBLIC_HEADER
  Log.h
  LogAppender.h
  LogEvent.h
  LogFormater.h
  Logger.h
  LogLevel.h
  LoggerManager.h
)
target_sources(
  ${LIB}
  PRIVATE
  LogAppender.cpp
  LogEvent.cpp
  LogFormater.cpp
  Logger.cpp
  LogLevel.cpp
  LoggerManager.cpp
)

target_include_directories(
  ${LIB}
  PUBLIC
  ${SRC_DIR}
)

target_link_libraries(
  ${LIB}
  PUBLIC
  ${PROJECT_NAME}::util
)