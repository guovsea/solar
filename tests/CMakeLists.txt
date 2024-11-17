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
  test_log.cpp
  test_config.cpp
)
target_link_libraries(${EXE_NAME} 
  PRIVATE 
  ${PROJECT_NAME}::log
  GTest::gtest_main  
)
# copy
set(TEST_ASSERT_FILES ${CMAKE_CURRENT_SOURCE_DIR}/test.yml)
add_custom_command(
    TARGET ${EXE_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${TEST_ASSERT_FILES} $<TARGET_FILE_DIR:${EXE_NAME}>
    COMMENT "Copy test assert files."
)