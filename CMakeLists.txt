cmake_minimum_required(VERSION 3.30)
project(FilesSync)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 多平台支持
if (UNIX AND NOT APPLE)
    message(STATUS "Linux platform detected, please ensure Qt libraries are in PATH.")
elseif (APPLE)
    message(STATUS "macOS platform detected, consider using macdeployqt for deployment.")
endif ()

# 自动查找 Qt 安装路径
if (NOT CMAKE_PREFIX_PATH)
    set(CMAKE_PREFIX_PATH "D:/devtools/Qt/5.12.9/mingw73_64/lib/cmake")
endif ()

find_package(Qt5 COMPONENTS Core Gui Widgets Svg Network Test Concurrent REQUIRED)

# 添加子目录，并传递 PROJECT_NAME
add_subdirectory(libs)
add_subdirectory(sdk)
add_subdirectory(src)

# 如果 SDK 是一个外部库，手动添加 include 和库路径
#include_directories(sdk/include)
#link_directories(sdk/lib)

# 调试信息输出
message(STATUS "CMake Prefix Path: ${CMAKE_PREFIX_PATH}")

# 添加测试支持
enable_testing()
add_subdirectory(test)
