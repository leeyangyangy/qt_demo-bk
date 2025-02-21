cmake_minimum_required(VERSION 3.30)
set(CMAKE_CXX_STANDARD 17)

project(qtest)

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
add_executable(${PROJECT_NAME}
        test_main.cpp        # 统一入口文件
        test_leveldb.cpp     # LevelDB 测试
)

# 链接依赖库
target_link_libraries(${PROJECT_NAME}
        Qt5::Test
        Qt5::Core        # 必须添加
        leveldb          # LevelDB 目标名（来自 add_subdirectory(libs/leveldb)）
)

# 注册测试到 CTest
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME} )