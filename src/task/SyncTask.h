#include <QObject>
#include <QRunnable>
#include <QDir>
#include <QDebug>

class SyncTask : public QObject, public QRunnable
{
    Q_OBJECT
public:
    SyncTask(const QString& source, const QString& target)
        : sourcePath(source), targetPath(target) {}

    void run() override
    {
        syncDirectoryRecursively(sourcePath, targetPath);
        emit taskCompleted(sourcePath, targetPath); // 任务完成时发送信号
    }

    signals:
        void taskCompleted(const QString& source, const QString& target);

private:
    QString sourcePath;
    QString targetPath;

    void syncDirectoryRecursively(const QString& source, const QString& target)
    {
        QDir sourceDir(source);
        if (!sourceDir.exists())
        {
            qDebug() << "Source directory does not exist:" << source;
            return;
        }

        QDir targetDir(target);
        if (!targetDir.exists())
        {
            if (!targetDir.mkpath("."))
            {
                qDebug() << "Failed to create target directory:" << target;
                return;
            }
        }

        QFileInfoList entries = sourceDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
        for (const QFileInfo& fileInfo : entries)
        {
            QString sourceFilePath = fileInfo.absoluteFilePath();
            QString targetFilePath = targetDir.filePath(fileInfo.fileName());

            if (QFile::exists(targetFilePath))
            {
                QFile::remove(targetFilePath);
            }

            QFile::copy(sourceFilePath, targetFilePath);
        }

        QFileInfoList subDirs = sourceDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo& subDirInfo : subDirs)
        {
            syncDirectoryRecursively(subDirInfo.absoluteFilePath(), targetDir.filePath(subDirInfo.fileName()));
        }
    }
};
