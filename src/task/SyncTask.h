#ifndef SYNCTASK_H
#define SYNCTASK_H

#include <QDateTime>
#include <QList>
#include <QObject>
#include <QRunnable>
#include <QString>

// 用于记录每个操作日志
struct OperationLog {
    QDateTime timestamp;
    QString type;
    QString sourcePath;
    QString targetPath;
    QString status;
    QString errorMessage;
};

// 冲突记录结构体
struct ConflictEntry {
    QString filename;
    QString sourcePath;
    QString targetPath;
    QString oldHash;
    QString newHash;
    QDateTime conflictTime;
};

class SyncTask : public QObject,public QRunnable
{
    Q_OBJECT
public:
    explicit SyncTask(const QString &source, const QString &target, QObject *parent = nullptr);

    // 同步任务入口
    void run();

signals:
    // 同步任务完成后发出此信号
    void taskCompleted(const QString &source, const QString &target);

private:
    // 同步相关目录路径
    QString sourcePath;
    QString targetPath;
    // 用于区分每次任务的时间戳（用于日志、冲突目录）
    QString taskTimestamp;

    // 操作日志记录列表
    QList<OperationLog> operationLogs;
    // 冲突记录列表
    QList<ConflictEntry> conflictEntries;

    // 常量定义（可根据需要调整）
    static const QString VERSION_FILE; // 版本记录文件名
    static const QString LOG_DIR;      // 日志目录
    static const QString CONFLICT_DIR; // 冲突文件目录

    // 目录同步相关函数
    void syncDirectory(const QString &source, const QString &target);
    void processDirectory(const QString &basePath,
                          const QString &relativePath,
                          const QString &source,
                          const QString &target,
                          QMap<QString, QString> &oldRecords,
                          QMap<QString, QString> &newRecords);

    // 文件操作函数
    QString calculateFileHash(const QString &filePath) const;
    bool copyFileWithLog(const QString &source, const QString &target);
    bool removeFileWithLog(const QString &path);
    void handleConflict(const QString &targetFile, const QString &relativePath);

    // 版本记录读写
    QMap<QString, QString> readVersionRecords(const QString &recordPath);
    void writeVersionRecords(const QString &recordPath, const QMap<QString, QString> &records);

    // 生成 Excel 日志和冲突报告
    void saveTaskLog();
    void generateConflictReport();

    // 辅助函数
    void ensureDirectoryExists(const QString &path);
    QString getLogDirPath() const;
    QString getConflictDirPath() const;

    // 日志记录，采用局部静态互斥锁确保线程安全
    void logOperation(const QString &type,
                      const QString &source,
                      const QString &target,
                      const bool success,
                      const QString &error);
};

#endif // SYNCTASK_H

