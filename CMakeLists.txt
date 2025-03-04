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

#set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/install/${TARGET_ARCH})
# 自动检测 32 位或 64 位，设置对应的 Qt 安装路径 | CMAKE_SIZEOF_VOID_P：表示指针的大小，8 代表 64 位，4 代表 32 位
if (NOT CMAKE_PREFIX_PATH)
    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        # 64 位编译
        set(CMAKE_PREFIX_PATH "D:/devtools/Qt/5.12.9/mingw73_64/lib/cmake")
        message(STATUS "Detected 64-bit build, using Qt 64-bit.")
        set(MINGW_BIN_PATH "D:/devtools/Qt/5.12.9/mingw73_64/bin")
        set(LIBGCC_DLL "libgcc_s_seh-1.dll")
        message(STATUS "Using MinGW 64-bit DLLs.")
    elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
        # 32 位编译
        set(CMAKE_PREFIX_PATH "D:/devtools/Qt/5.12.9/mingw73_32/lib/cmake")
        message(STATUS "Detected 32-bit build, using Qt 32-bit.")
        set(MINGW_BIN_PATH "D:/devtools/Qt/5.12.9/mingw73_32/bin")
        set(LIBGCC_DLL "libgcc_s_dw2-1.dll")
        message(STATUS "Using MinGW 32-bit DLLs.")
    else ()
        message(WARNING "Unknown architecture. Please check your toolchain settings.")
    endif ()
endif ()

find_package(Qt5 COMPONENTS Core Sql Gui Widgets Svg Network Test Concurrent REQUIRED)

# 添加子目录，并传递 PROJECT_NAME
add_subdirectory(libs)
add_subdirectory(sdk)
add_subdirectory(src)

# 如果 SDK 是一个外部库，手动添加 include 和库路径
#include_directories(sdk/include)
#link_directories(sdk/lib)

# 调试信息输出
message(STATUS "CMake Prefix Path: ${CMAKE_PREFIX_PATH}")
message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

# 添加测试支持
enable_testing()
add_subdirectory(test)
add_subdirectory(src/utils/db)

# 覆盖率编译选项（仅限 Debug 模式）
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # GCC/Clang 标志
        add_compile_options(-fprofile-arcs -ftest-coverage)
        add_link_options(--coverage)
    elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # MSVC 标志（需配合 OpenCppCoverage）
        add_compile_options(/DEBUG)
    endif()
endif()

