/**
******************************************************************************
* File Name      : FileProcessTask.cpp
* Author         : 17872
* E-mail         : liyangyang0713@foxmail.com
* Create         : 2025/1/7 20:36
******************************************************************************
*/

#ifndef FILEMONITOR_H
#define FILEMONITOR_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QThreadPool>
#include <QRunnable>
#include <QDebug>

// 文件处理任务类
class FileProcessTask : public QRunnable {
public:
    explicit FileProcessTask(const QString &filePath);
    void run() override;

private:
    QString filePath;
};

#endif // FILEMONITOR_H