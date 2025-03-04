/**
******************************************************************************
* File Name      : test_leveldb_wrapper.cpp
* Author         : 17872
* E-mail         : liyangyang0713@foxmail.com
* Create         : 2025/3/5 2:24
******************************************************************************
*/
#include "LevelDBWrapper.h"

#include <iostream>

LevelDBWrapper::LevelDBWrapper() : db_(nullptr) {
  options_.create_if_missing = true;  // 自动创建数据库
}

LevelDBWrapper::~LevelDBWrapper() {
  Close();  // 确保资源释放
}

// ================= 核心方法 =================
void LevelDBWrapper::Close() {
  if (db_ != nullptr) {
    delete db_;
    db_ = nullptr;
  }
}

bool LevelDBWrapper::Open(const std::string& db_path) {
  Close();  // 关键改进：先关闭已有连接

  leveldb::Status status = leveldb::DB::Open(options_, db_path, &db_);
  if (!status.ok()) {
    std::cerr << "[Error] Open failed: " << status.ToString() << std::endl;
    return false;
  }
  return true;
}

// ================= 数据操作 =================
bool LevelDBWrapper::Put(const std::string& key,
                         const std::string& value) const {
  if (!db_) {
    std::cerr << "[Error] Put failed: Database not open" << std::endl;
    return false;
  }

  leveldb::Status status = db_->Put(leveldb::WriteOptions(), key, value);
  if (!status.ok()) {
    std::cerr << "[Error] Put operation failed: " << status.ToString()
              << std::endl;
    return false;
  }
  return true;
}

bool LevelDBWrapper::Get(const std::string& key, std::string& value) const {
  if (!db_) {
    std::cerr << "[Error] Get failed: Database not open" << std::endl;
    return false;
  }

  leveldb::Status status = db_->Get(leveldb::ReadOptions(), key, &value);
  if (!status.ok()) {
    if (!status.IsNotFound()) {  // 忽略"未找到"的常规错误
      std::cerr << "[Error] Get operation failed: " << status.ToString()
                << std::endl;
    }
    return false;
  }
  return true;
}

bool LevelDBWrapper::Delete(const std::string& key) const {
  if (!db_) {
    std::cerr << "[Error] Delete failed: Database not open" << std::endl;
    return false;
  }

  leveldb::Status status = db_->Delete(leveldb::WriteOptions(), key);
  if (!status.ok()) {
    std::cerr << "[Error] Delete operation failed: " << status.ToString()
              << std::endl;
    return false;
  }
  return true;
}

// ================= 迭代器 =================
std::unique_ptr<leveldb::Iterator> LevelDBWrapper::NewIterator() const {
  if (!db_) {
    std::cerr << "[Error] Cannot create iterator: Database not open"
              << std::endl;
    return nullptr;
  }
  return std::unique_ptr<leveldb::Iterator>(
      db_->NewIterator(leveldb::ReadOptions()));
}