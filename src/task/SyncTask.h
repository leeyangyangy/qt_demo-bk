#ifndef SYNCTASK_H
#define SYNCTASK_H

#include <QObject>
#include <QMap>
#include <QDateTime>
#include <QString>
#include <QStringList>
#include <QMutex>
#include "../utils/rule/RulesParser.h"  // 用于读取排除规则

// LevelDB 头文件
#include <QRunnable>

#include "leveldb/db.h"

/**
 * @brief 记录操作日志的结构体
 */
struct OperationLog {
    QDateTime timestamp;
    QString type;
    QString sourcePath;
    QString targetPath;
    QString status;
    QString errorMessage;
};

/**
 * @brief 记录文件冲突的结构体
 */
struct ConflictEntry {
    QString filename;
    QString sourcePath;
    QString targetPath;
    QString oldHash;
    QString newHash;
    QDateTime conflictTime;
};

/**
 * @brief SyncTask 同步任务类
 *
 * 该类负责执行文件目录同步工作，在同步过程中：
 * 1. 根据 RulesParser 配置的规则排除不需要同步的目录或文件；
 * 2. 使用 LevelDB 存储版本记录，数据库保存在可执行程序目录下的 db 文件夹中。
 */
class SyncTask : public QObject,public QRunnable
{
    Q_OBJECT
public:
    static const QString VERSION_FILE;    ///< 版本记录标识（仅用于日志提示）
    static const QString LOG_DIR;         ///< 日志目录
    static const QString CONFLICT_DIR;    ///< 冲突文件目录

    /**
     * @brief 构造函数
     * @param source 同步源目录
     * @param target 同步目标目录
     * @param parent 父对象
     */
    explicit SyncTask(const QString &source, const QString &target, QObject *parent = nullptr);
    virtual ~SyncTask();

    /**
     * @brief 执行同步任务
     */
    void run() override;

signals:
    /**
     * @brief 同步任务完成信号
     * @param source 源目录
     * @param target 目标目录
     */
    void taskCompleted(const QString &source, const QString &target);

private:
    QString sourcePath;      ///< 源目录
    QString targetPath;      ///< 目标目录
    QString taskTimestamp;   ///< 任务时间戳（用于区分日志和冲突目录）

    QList<OperationLog> operationLogs;    ///< 操作日志列表
    QList<ConflictEntry> conflictEntries; ///< 冲突信息列表

    RulesParser *rulesParser;   ///< 排除规则解析器

    leveldb::DB *versionDB;     ///< LevelDB 数据库，用于存储版本记录

    /**
     * @brief 同步过程相关的内部方法
     * @param source
     * @param target
     */
    void syncDirectory(const QString &source, const QString &target);

    /**
     * @brief 同步过程相关的内部方法
     * @param basePath
     * @param relativePath
     * @param source
     * @param target
     * @param oldRecords
     * @param newRecords
     */
    void processDirectory(const QString &basePath,
                          const QString &relativePath,
                          const QString &source,
                          const QString &target,
                          QMap<QString, QString> &oldRecords,
                          QMap<QString, QString> &newRecords);
    QString calculateFileHash(const QString &filePath);
    bool copyFileWithLog(const QString &source, const QString &target);
    bool removeFileWithLog(const QString &path);
    void handleConflict(const QString &targetFile, const QString &relativePath);

    /**
     * @brief 从 LevelDB 中读取版本记录
     * @param recordPath 参数兼容旧接口（现已忽略）
     * @return 版本记录映射表，key 为相对路径，value 为文件 MD5 值
     */
    QMap<QString, QString> readVersionRecords(const QString &recordPath) const;

    /**
     * @brief 将新版本记录写入 LevelDB
     * @param recordPath 参数兼容旧接口（现已忽略）
     * @param records 版本记录映射表
     */
    void writeVersionRecords(const QString &recordPath, const QMap<QString, QString> &records);

    void saveTaskLog();
    void generateConflictReport();
    void ensureDirectoryExists(const QString &path);
    QString getLogDirPath() const;
    QString getConflictDirPath() const;
    void logOperation(const QString &type, const QString &source,
                      const QString &target, const bool success,
                      const QString &error);

    /**
     * @brief 根据配置规则判断该路径是否需要排除
     * @param relativePath 相对于源目录的相对路径
     * @return true 表示排除，false 表示不排除
     */
    bool shouldExclude(const QString &relativePath) const;

    /**
     * @brief 自动清理工作空间
     * @param taskTimestamp 时间戳
     * @param logDir 日志目录
     * @param conflictDir 冲突目录
     */
    void autoCleanWorkingDir(const QString &, const QString &,
                             const QString &) const;
};

#endif // SYNCTASK_H
