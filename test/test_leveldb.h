#ifndef TEST_LEVELDB_H
#define TEST_LEVELDB_H

#include <QtTest/QtTest>
#include <leveldb/db.h>
#include <leveldb/options.h>

class TestLevelDB final : public QObject
{
  Q_OBJECT

public:
  explicit TestLevelDB(QObject *parent = nullptr);

  private slots:
      void initTestCase();     // 测试前初始化
  void cleanupTestCase();  // 测试后清理

  void testOpenDatabase() const; // 测试数据库创建
  void testKeyOperations() const;  // 测试键值操作
  void testBatchWrite() const;   // 测试批量写入
  void testIteration() const;    // 测试数据遍历
  void testDeleteKey() const;    // 测试删除操作

private:
  leveldb::DB* m_db = nullptr;
  const QString m_testDbPath = "./test_db_tmp"; // 测试数据库路径
};

#endif // TEST_LEVELDB_H