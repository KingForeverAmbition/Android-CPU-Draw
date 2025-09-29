cmake_minimum_required(VERSION 3.21.0)

set(NDK_PATH 填写你的NDK路径) # 自己填
set(CMAKE_SYSTEM_NAME ANDROID)
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_SYSTEM_VERSION 30)
set(ANDROID_PLATFORM 30)
set(ANDROID_SDK_TOOLS_VERSION 30)
set(ANDROID_ABI arm64-v8a)
set(ANDROID_NDK ${NDK_PATH})
set(CMAKE_TOOLCHAIN_FILE ${NDK_PATH}/build/cmake/android.toolchain.cmake)
set(ANDROID_SDK_ROOT ${NDK_PATH})


project(CPUDrawDemo)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -ffast-math -fvisibility=hidden -Wno-deprecated-copy-with-user-provided-copy")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -w")


include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/include/core
    ${PROJECT_SOURCE_DIR}/include/graphics
    ${PROJECT_SOURCE_DIR}/include/text
    ${PROJECT_SOURCE_DIR}/include/input
    ${PROJECT_SOURCE_DIR}/include/ui
    ${PROJECT_SOURCE_DIR}/include/platform
    ${PROJECT_SOURCE_DIR}/third_party
    ${PROJECT_SOURCE_DIR}/third_party/stb
)



set(GRAPHICS_SOURCES
    src/graphics/Primitives.cpp
    src/graphics/DrawList.cpp
)

set(TEXT_SOURCES
    src/text/TextRenderer.cpp
)

set(INPUT_SOURCES
    src/input/TouchHelper.cpp
)

set(UI_SOURCES
    src/ui/UIWidget.cpp
    src/ui/FloatingMenu.cpp
)

set(MAIN_SOURCES
    src/main.cpp
)


set(ALL_SOURCES
    ${GRAPHICS_SOURCES}
    ${TEXT_SOURCES}
    ${INPUT_SOURCES}
    ${UI_SOURCES}
    ${MAIN_SOURCES}
)


add_executable(${PROJECT_NAME} ${ALL_SOURCES})


target_link_libraries(${PROJECT_NAME}
    android
    log
)


set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}/CMakeFiles
    COMMENT "Cleaning all build files..."
)
