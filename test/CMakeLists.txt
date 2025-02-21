#cmake_minimum_required(VERSION 3.30)
#set(CMAKE_CXX_STANDARD 17)
#
#project(qtest)
#
## 自动查找 Qt 安装路径
#if (NOT CMAKE_PREFIX_PATH)
#    set(CMAKE_PREFIX_PATH "D:/devtools/Qt/5.12.9/mingw73_64/lib/cmake")
#endif ()
#
## 查找 Qt Test 组件
#find_package(Qt5 COMPONENTS Test REQUIRED)
#
## 包含 LevelDB 头文件路径
#include_directories(
#        ${CMAKE_SOURCE_DIR}/libs/leveldb/include
#)
#
## 添加 LevelDB 库（假设父目录已通过 add_subdirectory 引入 LevelDB）
#link_directories(${CMAKE_SOURCE_DIR}/libs/leveldb)
#
## =================================================================
## LevelDB 测试（独立可执行文件）
## =================================================================
#add_executable(test_leveldb
#        test_leveldb.cpp
#)
#
## 链接 LevelDB 和 Qt Test 库
#target_link_libraries(test_leveldb
#        Qt5::Test
#        leveldb  # LevelDB 目标名（来自 add_subdirectory(libs/leveldb)）
#)
#
## =================================================================
## 其他测试（示例）
## =================================================================
#add_executable(test_example
#        test_example.cpp
#)
#
#target_link_libraries(test_example
#        Qt5::Test
#        # 不链接 LevelDB
#)
#
## =================================================================
## 自动处理 Qt 的 MOC（元对象编译器）
## =================================================================
#set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)
#
## 添加覆盖率编译选项
#if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
#    target_compile_options(run_all_tests PRIVATE --coverage)
#    target_link_libraries(run_all_tests PRIVATE --coverage)
#endif()
#
## 添加测试到CTest
#include(CTest)
#
## 自动运行测试
#add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})

cmake_minimum_required(VERSION 3.30)
project(qtest)

# 启用测试支持
enable_testing()

# 查找 Qt Test 组件
find_package(Qt5 COMPONENTS Test Core REQUIRED)

# 包含 LevelDB 头文件路径
include_directories(
        ${CMAKE_SOURCE_DIR}/libs/leveldb/include
)

# 添加 LevelDB 库（假设父目录已通过 add_subdirectory 引入 LevelDB）
link_directories(${CMAKE_SOURCE_DIR}/libs/leveldb)

# =================================================================
# 创建统一测试可执行文件
# =================================================================
add_executable(run_all_tests
        test_main.cpp        # 统一入口文件
        test_leveldb.cpp     # LevelDB 测试
)

# 链接依赖库
target_link_libraries(run_all_tests
        Qt5::Test
        Qt5::Core        # 必须添加
        leveldb          # LevelDB 目标名（来自 add_subdirectory(libs/leveldb)）
)

# =================================================================
# 自动处理 Qt 的 MOC（元对象编译器）
# =================================================================
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 添加覆盖率编译选项
#if (CMAKE_BUILD_TYPE STREQUAL "Coverage")
#    target_compile_options(run_all_tests PRIVATE --coverage)
#    target_link_libraries(run_all_tests PRIVATE --coverage)
#endif()

# 根目录已经添加过了，这里无需添加
#if (CMAKE_BUILD_TYPE STREQUAL "Debug")
#    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
#    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
#endif()

# 注册测试到 CTest
add_test(NAME all_tests COMMAND run_all_tests)