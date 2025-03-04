add_library(leveldb_wrapper LevelDBWrapper.cpp)

# 包含 LevelDB 头文件路径
target_include_directories(leveldb_wrapper PUBLIC ${CMAKE_SOURCE_DIR}/libs/leveldb/include)

# 链接 LevelDB 库
target_link_libraries(leveldb_wrapper leveldb)