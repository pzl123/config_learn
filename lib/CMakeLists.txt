# 设置生成静态库和动态库的输出目录
#set(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib)

# 如果你需要分别设置静态库和动态库的输出目录，可以使用以下两个指令：
set(ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib) # 静态库
# set(LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib) # 动态库

# 添加库的源文件
set(LIB_SOURCES
    cJSON.c
    config_fun.c
)

# 创建静态库
add_library(configlib STATIC ${LIB_SOURCES})

# 如果你需要创建动态库，使用以下命令：
# add_library(mylib SHARED ${LIB_SOURCES})


# 设置包含目录
target_include_directories(configlib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# 目标链接库（如果需要）
# target_link_libraries(mylib <other_libraries>)
