# 设置当前目录为目标目录
file(GLOB SOURCES "*.c")

# 添加可执行文件
add_executable(MyProject ${SOURCES})

# 链接库
target_link_libraries(MyProject configlib cJSOnlib lsan)

