cmake_minimum_required(VERSION 3.10) # cmake最低版本
project(cmake_test VERSION 1.0)

set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build Type" FORCE)

# 设置C标准
# set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD 99)

# 设置库和可执行文件的输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

# 包含头文件
include_directories(
    ${PROJECT_SOURCE_DIR}/include/config_manage/include
    ${PROJECT_SOURCE_DIR}/include/observer/include
    ${PROJECT_SOURCE_DIR}/include/uthash/include
    ${PROJECT_SOURCE_DIR}/third_party/cJSON/include
    ${PROJECT_SOURCE_DIR}/third_party/cJSONx/include
    ${PROJECT_SOURCE_DIR}/third_party/uthash/include
)

# 设置编译选项
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize=leak")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address -fsanitize=leak")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

# 添加子目录
add_subdirectory(src)
add_subdirectory(lib)
add_subdirectory(include)
add_subdirectory(third_party)