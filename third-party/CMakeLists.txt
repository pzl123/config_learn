# third_party/CMakeLists.txt

# Add cJSON library
add_library(cJSONlib SHARED
    ${PROJECT_SOURCE_DIR}/third-party/cJSON/src/cJSON.c
)
target_include_directories(cJSONlib PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/cJSON/include>
    $<INSTALL_INTERFACE:include>
)

# Add cJSONx library
add_library(cJSONxlib SHARED
    ${PROJECT_SOURCE_DIR}/third-party/cJSONx/src/cJSONx.c
)
target_include_directories(cJSONxlib PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third-party/cJSONx/include>
    $<INSTALL_INTERFACE:include>
)

# Add uthash library
# add_library(uthashlib STATIC
#     ${PROJECT_SOURCE_DIR}/third_party/uthash/src/uthash.c
# )
# target_include_directories(uthashlib PUBLIC
#     $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/third_party/uthash/include>
#     $<INSTALL_INTERFACE:include>
# )