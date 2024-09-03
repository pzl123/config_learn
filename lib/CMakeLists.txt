# 设置生成静态库和动态库的输出目录
#set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

# 如果你需要分别设置静态库和动态库的输出目录，可以使用以下两个指令：
set(ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib) # 静态库
# set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib) # 动态库

add_library(mylib STATIC
    module1
    module2
)

target_include_directories(mylib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
