#include "FileMonitor.h"
#include <QThread>
#include <QDir>
#include <qstandardpaths.h>

// 文件处理任务实现
FileProcessTask::FileProcessTask(const QString& filePath) : filePath(filePath)
{
    qDebug() << "Processing file content:" << filePath;
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&file);
        QString content = in.readAll();
        file.close();

        // 对内容进行操作
        qDebug() << "File content processed:" << content;

        // 保存持久化
        QFile outFile("output.txt");
        if (outFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            QTextStream out(&outFile);
            out << content; // 假设这里直接保存原始内容
            outFile.close();
            qDebug() << "Content saved to output.txt";
        }
    }
}

void FileProcessTask::run()
{
    // 模拟文件处理
    qDebug() << "Processing file in thread:" << QThread::currentThread() << filePath;
    QThread::sleep(2); // 模拟耗时操作
    qDebug() << "Finished processing:" << filePath;
}

// 文件监控类实现
FileMonitor::FileMonitor(QObject* parent)
{
    fileWatcher = new QFileSystemWatcher(this);

    connect(fileWatcher, &QFileSystemWatcher::fileChanged, this, &FileMonitor::onFileChanged);
    connect(fileWatcher, &QFileSystemWatcher::directoryChanged, this, &FileMonitor::onDirectoryChanged);

    // 设置线程池最大线程数
    QThreadPool::globalInstance()->setMaxThreadCount(4);
}

void FileMonitor::addPath(const QString& path)
{
    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
    {
        qDebug() << "Path does not exist:" << path;
        return;
    }
    if (fileInfo.isDir())
    {
        // 如果是目录，递归添加子目录和文件
        addDirectoryRecursively(path);
    }
    else if (fileInfo.isFile())
    {
        // 如果是文件，直接添加到监控
        fileWatcher->addPath(path);
        qDebug() << "Started monitoring file:" << path;
    }
}

void FileMonitor::addDirectoryRecursively(const QString& directoryPath)
{
    QDir dir(directoryPath);
    if (!dir.exists())
    {
        qDebug() << "Directory does not exist:" << directoryPath;
        return;
    }

    // 添加当前目录到监控
    fileWatcher->addPath(directoryPath);
    qDebug() << "A Started monitoring directory:" << directoryPath;

    // 获取目录中的所有文件
    QFileInfoList entries = dir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo& fileInfo : entries)
    {
        fileWatcher->addPath(fileInfo.absoluteFilePath());
        qDebug() << "A Started monitoring file:" << fileInfo.absoluteFilePath();
    }

    // 递归添加子目录
    QFileInfoList subDirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& subDirInfo : subDirs)
    {
        addDirectoryRecursively(subDirInfo.absoluteFilePath());
    }
}

void FileMonitor::syncDirectoryRecursively(const QString& sourcePath, const QString& targetPath)
{
    QDir sourceDir(sourcePath);
    if (!sourceDir.exists())
    {
        qDebug() << "Source directory does not exist:" << sourcePath;
        return;
    }

    // 创建目标目录
    QDir targetDir(targetPath);
    if (!targetDir.exists())
    {
        if (!targetDir.mkpath("."))
        {
            qDebug() << "Failed to create target directory:" << targetPath;
            return;
        }
    }

    // 复制当前目录中的文件
    QFileInfoList entries = sourceDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
    for (const QFileInfo& fileInfo : entries)
    {
        QString sourceFilePath = fileInfo.absoluteFilePath();
        QString targetFilePath = targetDir.filePath(fileInfo.fileName());

        if (QFile::exists(targetFilePath))
        {
            QFile::remove(targetFilePath); // 删除已存在的文件（可选，根据需求）
        }

        if (QFile::copy(sourceFilePath, targetFilePath))
        {
            qDebug() << "Copied file:" << sourceFilePath << "->" << targetFilePath;
        }
        else
        {
            qDebug() << "Failed to copy file:" << sourceFilePath;
        }
    }

    // 递归处理子目录
    QFileInfoList subDirs = sourceDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QFileInfo& subDirInfo : subDirs)
    {
        QString sourceSubDir = subDirInfo.absoluteFilePath();
        QString targetSubDir = targetDir.filePath(subDirInfo.fileName());

        syncDirectoryRecursively(sourceSubDir, targetSubDir);
    }
}


void FileMonitor::setThreadCount(int count)
{
    QThreadPool::globalInstance()->setMaxThreadCount(count);
    qDebug() << "Thread pool size set to:" << count;
}

void FileMonitor::onFileChanged(const QString& path)
{
    qDebug() << "File changed:" << path;
    QThreadPool::globalInstance()->start(new FileProcessTask(path));
}

void FileMonitor::onDirectoryChanged(const QString& path)
{
    qDebug() << "Directory changed:" << path;
    // 可以根据需要处理目录变化
}
