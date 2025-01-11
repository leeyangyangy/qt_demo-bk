//
// Created by 17872 on 2025/1/7.
//

#ifndef FILEMONITOR_H
#define FILEMONITOR_H

#include <QFileSystemWatcher>
#include <QThreadPool>
#include <QRunnable>
#include <QDebug>

// 文件处理任务类
class FileProcessTask : public QRunnable
{
public:
    explicit FileProcessTask(const QString& filePath);
    void run() override;

private:
    QString filePath;
};

// 文件监控类
class FileMonitor : public QObject
{
    Q_OBJECT

public:
    explicit FileMonitor(QObject* parent = nullptr);
    // ~FileMonitor() override;

    // 添加需要监听的文件或目录
    void addPath(const QString& path);

    // 动态设置线程池的线程数
    void setThreadCount(int count);

    // 递归搜索文件
    void addDirectoryRecursively(const QString &directoryPath);
    void syncDirectoryRecursively(const QString& sourcePath, const QString& targetPath);

private slots:
    void onFileChanged(const QString& path);
    void onDirectoryChanged(const QString& path);

private:
    QFileSystemWatcher* fileWatcher;
};


#endif //FILEMONITOR_H
