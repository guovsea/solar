
set(LIB_NAME log)
add_library(${LIB_NAME})
add_library(${PROJECT_NAME}::${LIB_NAME} ALIAS ${LIB_NAME})

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
  ${LIB_NAME}
  PRIVATE
  Log.cpp
  LogAppender.cpp
  LogEvent.cpp
  LogFormater.cpp
  Logger.cpp
  LogLevel.cpp
  LoggerManager.cpp
)

target_include_directories(
  ${LIB_NAME}
  PUBLIC
  ${SRC_DIR}
)

target_link_libraries(
  ${LIB_NAME}
  PUBLIC
  ${PROJECT_NAME}::util
)