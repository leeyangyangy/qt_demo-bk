add_subdirectory(QXlsx/QXlsx)
add_subdirectory(xxHash/cmake_unofficial)
# 添加 LevelDB 子目录并配置选项
option(LEVELDB_BUILD_TESTS "Build LevelDB tests" OFF)
option(LEVELDB_BUILD_BENCHMARKS "Build LevelDB benchmarks" OFF)
set(LEVELDB_WITH_SNAPPY OFF) # 不使用 Snappy 压缩
set(BUILD_SHARED_LIBS OFF) # 生成静态库
add_subdirectory(leveldb)
