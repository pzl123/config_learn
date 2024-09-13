# include/CMakeLists.txt

# 如果需要安装头文件，可以在这里做安装操作
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include
        DESTINATION include
        FILES_MATCHING PATTERN "*.h"
)