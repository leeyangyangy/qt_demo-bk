#include "test_leveldb.h"

#include <QDebug>
#include <QDir>

#include "leveldb/write_batch.h"

TestLevelDB::TestLevelDB(QObject *parent) 
    : QObject(parent)
{}

void TestLevelDB::initTestCase()
{
    // 清理旧测试数据
    QDir(m_testDbPath).removeRecursively();
    
    // 打开数据库
    leveldb::Options options;
    options.create_if_missing = true;
    leveldb::Status status = leveldb::DB::Open(
        options, 
        m_testDbPath.toStdString(), 
        &m_db
    );
    
    QVERIFY2(status.ok(), "Failed to open test database");
}

void TestLevelDB::cleanupTestCase()
{
    // 关闭数据库
    delete m_db;
    m_db = nullptr;
    
    // 清理测试目录
    QVERIFY(QDir(m_testDbPath).removeRecursively());
}

void TestLevelDB::testOpenDatabase() const {
    QVERIFY(m_db != nullptr);
    
    // 验证空数据库状态
    leveldb::Iterator* it = m_db->NewIterator(leveldb::ReadOptions());
    it->SeekToFirst();
    QVERIFY(!it->Valid()); // 应无数据
    delete it;
}

void TestLevelDB::testKeyOperations() const {
    // 测试写入
    leveldb::Status s = m_db->Put(
        leveldb::WriteOptions(), 
        "test_key", 
        "test_value"
    );
    QVERIFY(s.ok());

    // 测试读取
    std::string value;
    s = m_db->Get(leveldb::ReadOptions(), "test_key", &value);
    QVERIFY(s.ok());
    QCOMPARE(QString::fromStdString(value), QString("test_value"));

    // 测试不存在键
    s = m_db->Get(leveldb::ReadOptions(), "non_exist_key", &value);
    QVERIFY(s.IsNotFound());
}

void TestLevelDB::testBatchWrite() const {
    leveldb::WriteBatch batch;
    batch.Put("batch_key1", "value1");
    batch.Put("batch_key2", "value2");
    batch.Delete("test_key"); // 删除之前测试的键

    leveldb::Status s = m_db->Write(leveldb::WriteOptions(), &batch);
    QVERIFY(s.ok());

    // 验证批量写入
    std::string value;
    s = m_db->Get(leveldb::ReadOptions(), "batch_key1", &value);
    QVERIFY(s.ok());
    QCOMPARE(QString::fromStdString(value), QString("value1"));

    // 验证删除
    s = m_db->Get(leveldb::ReadOptions(), "test_key", &value);
    QVERIFY(s.IsNotFound());
}

void TestLevelDB::testIteration() const {
    // 写入测试数据
    m_db->Put(leveldb::WriteOptions(), "iter_key1", "A");
    m_db->Put(leveldb::WriteOptions(), "iter_key2", "B");
    m_db->Put(leveldb::WriteOptions(), "iter_key3", "C");

    // 正向遍历
    leveldb::Iterator* it = m_db->NewIterator(leveldb::ReadOptions());
    QStringList keys;
    for (it->SeekToFirst(); it->Valid(); it->Next()) {
        keys << QString::fromStdString(it->key().ToString());
    }
    delete it;

    QCOMPARE(keys, QStringList() << "batch_key1" << "batch_key2" 
             << "iter_key1" << "iter_key2" << "iter_key3");

    // 反向遍历
    it = m_db->NewIterator(leveldb::ReadOptions());
    keys.clear();
    for (it->SeekToLast(); it->Valid(); it->Prev()) {
        keys << QString::fromStdString(it->key().ToString());
    }
    delete it;

    QCOMPARE(keys, QStringList() << "iter_key3" << "iter_key2" << "iter_key1" 
             << "batch_key2" << "batch_key1");
}

void TestLevelDB::testDeleteKey() const {
    // 删除现有键
    leveldb::Status s = m_db->Delete(leveldb::WriteOptions(), "batch_key1");
    QVERIFY(s.ok());

    // 验证删除
    std::string value;
    s = m_db->Get(leveldb::ReadOptions(), "batch_key1", &value);
    QVERIFY(s.IsNotFound());
}
