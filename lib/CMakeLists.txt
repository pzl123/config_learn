# lib/CMakeLists.txt

# Add your custom library
add_library(config_managelib SHARED
    ${PROJECT_SOURCE_DIR}/include/config_manage/src/config_manage.c
)
target_include_directories(config_managelib PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/config/include>
    $<INSTALL_INTERFACE:include>
)

add_library(observerlib SHARED
    ${PROJECT_SOURCE_DIR}/include/observer/src/observer.c
)
target_include_directories(observerlib PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/observer/include>
    $<INSTALL_INTERFACE:include>
)
