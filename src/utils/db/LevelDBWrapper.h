/**
******************************************************************************
* File Name      : test_leveldb_wrapper.cpp
* Author         : 17872
* E-mail         : liyangyang0713@foxmail.com
* Create         : 2025/3/5 2:24
******************************************************************************
*/

#ifndef LEVELDB_WRAPPER_H
#define LEVELDB_WRAPPER_H

#include <leveldb/db.h>

#include <iostream>
#include <memory>
#include <string>

class LevelDBWrapper {
 public:
  LevelDBWrapper();
  ~LevelDBWrapper();

  // 数据库操作
  bool Open(const std::string& db_path);
  void Close();

  // 数据操作
  bool Put(const std::string& key, const std::string& value) const;
  bool Get(const std::string& key, std::string& value) const;
  bool Delete(const std::string& key) const;

  // 迭代器
  std::unique_ptr<leveldb::Iterator> NewIterator() const;

 private:
  leveldb::DB* db_;
  leveldb::Options options_;
};

#endif  // LEVELDB_WRAPPER_H