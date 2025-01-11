/**
******************************************************************************
* File Name      : test_example.cpp
* Author         : 17872
* E-mail         : liyangyang0713@foxmail.com
* Create         : 2025/1/5 21:10
******************************************************************************
*/

#include <QtTest/QtTest>

// 被测试的类
class MyClass
{
public:
    int add(int a, int b) { return a + b; }
    int multiply(int a, int b) { return a * b; }
};

class TestExample : public QObject
{
    Q_OBJECT

public:
    int add(int a, int b) { return a + b; }
    int multiply(int a, int b) { return a * b; }

private:
    MyClass* myClass; // 测试的目标类指针

private slots:
    // 测试前的初始化
    void initTestCase()
    {
        // 在所有测试开始前运行一次
        qDebug() << "Initializing test case...";
        myClass = new MyClass();
    }

    // 每个测试用例前的准备
    void init()
    {
        qDebug() << "Setting up test...";
    }

    // 测试用例 1：加法
    void testAddition()
    {
        QCOMPARE(myClass->add(2, 3), 5); // 检查是否等于 5
        QCOMPARE(myClass->add(-1, 1), 0); // 检查是否等于 0
    }

    // 测试用例 2：乘法
    void testMultiplication()
    {
        QCOMPARE(myClass->multiply(2, 3), 6); // 检查是否等于 6
        QCOMPARE(myClass->multiply(-1, 3), -3); // 检查是否等于 -3
    }

    // 每个测试用例后的清理
    void cleanup()
    {
        qDebug() << "Cleaning up after test...";
    }

    // 测试结束时的清理
    void cleanupTestCase()
    {
        // 在所有测试结束后运行一次
        qDebug() << "Cleaning up test case...";
        delete myClass;
    }

};

// 这个宏用于创建一个完整的 Qt 应用程序的主函数
// QTEST_MAIN(TestExample)
// 这个宏用于创建一个不需要 Qt 应用程序的主函数
QTEST_APPLESS_MAIN(TestExample)
#include "test_example.moc"
