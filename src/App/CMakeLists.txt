set(APP_NAME solar)
add_executable(${APP_NAME} "")

target_sources(
  ${APP_NAME}
  PRIVATE
  main.cpp
)
target_link_libraries(${APP_NAME} 
  PRIVATE 
  ${PROJECT_NAME}::log
)
# target_include_directories(${LIB}
#     PUBLIC
#     ${SRC_DIR}
# )