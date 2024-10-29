set(PUBLIC_HEADER
  Log/Log.h
  Log/LogAppender.h
  Log/LogEvent.h
  Log/LogFormater.h
  Log/Logger.h
  Log/LogLevel.h
)
add_executable(solar "")

target_include_directories(solar PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(
  solar
  PRIVATE
  Log/LogAppender.cpp
  Log/LogEvent.cpp
  Log/LogFormater.cpp
  main.cpp
)
