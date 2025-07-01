set(LIB_NAME core)
add_library(${LIB_NAME})
add_library(${PROJECT_NAME}::${LIB_NAME} ALIAS ${LIB_NAME})

set(PUBLIC_HEADER
  Thread.h
  Semaphore.h
  Mutex.h
  MutexImpl.h
  Fiber.h
  Scheduler.h
)

target_sources(
  ${LIB_NAME}
  PRIVATE
  Thread.cpp
  Semaphore.cpp
  Mutex.cpp
  Fiber.cpp
  Scheduler.cpp
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
)