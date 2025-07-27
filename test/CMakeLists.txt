set(EXE_NAME solar_test)
add_executable(${EXE_NAME} "")

# gtest
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
FetchContent_MakeAvailable(googletest)

enable_testing()
include(GoogleTest)
target_sources(
  ${EXE_NAME}
  PRIVATE
  test_main.cpp
  test_log.cpp
  test_config.cpp
  test_thread_mutex.cpp
  test_util.cpp
  test_fiber.cpp
  test_scheduler.cpp
  test_iomanager.cpp
  test_hook.cpp
  test_address.cpp
)

target_link_libraries(${EXE_NAME} 
  PRIVATE 
  ${PROJECT_NAME}::log
  ${PROJECT_NAME}::core
  ${PROJECT_NAME}::network
  GTest::gtest
)

set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_definitions(${EXE_NAME} PRIVATE __TEST_DIR="${TEST_DIR}/")
