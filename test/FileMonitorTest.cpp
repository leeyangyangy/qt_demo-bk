/**
******************************************************************************  
* File Name      : FileMonitorTest.cpp  
* Author         : 17872  
* E-mail         : liyangyang0713@foxmail.com  
* Create         : 2025/1/7 21:41  
******************************************************************************  
*/
#include <QtTest/QtTest>

#include <QFile>
#include <QDir>
#include <qsignalspy.h>

#include "../src/task/FileMonitor.h" // 包含你的文件监控类

class FileMonitorTest : public QObject {
    Q_OBJECT

private slots:
    void init(); // 初始化测试
    void cleanup(); // 清理测试
    void testFileMonitoring(); // 测试文件监控

private:
    FileMonitor *fileMonitor;
    QString testDir;
};

void FileMonitorTest::init() {
    testDir = QDir::tempPath() + "/testDir"; // 使用临时目录
    QDir().mkdir(testDir); // 创建测试目录
    fileMonitor = new FileMonitor(); // 实例化文件监控类
    fileMonitor->addPath(testDir); // 添加监控路径
}

void FileMonitorTest::cleanup() {
    delete fileMonitor; // 删除监控对象
    QDir(testDir).removeRecursively(); // 清理测试目录
}

void FileMonitorTest::testFileMonitoring() {
    QString testFilePath = testDir + "/testFile.txt";

    QFile file(testFilePath);
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text)); // 确保文件成功打开

    file.write("Initial content"); // 写入初始内容
    file.close();

    QSignalSpy spy(fileMonitor, &FileMonitor::); // 创建信号监视器

    // 修改文件内容以触发信号
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    file.write("Updated content");
    file.close();

    // 等待信号被触发
    QTRY_COMPARE(spy.count(), 1); // 检查信号是否被触发一次
}

QTEST_MAIN(FileMonitorTest)
#include "filemonitortest.moc" // 包含 MOC 文件