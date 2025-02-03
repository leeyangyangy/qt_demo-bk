#include "SyncTask.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>

// 包含 QXlsx 相关头文件（确保 CMake 配置正确）
#include "xlsxdocument.h"

// 定义常量（可根据需要修改）
const QString SyncTask::VERSION_FILE = "version_records.txt";
const QString SyncTask::LOG_DIR = "logs";
const QString SyncTask::CONFLICT_DIR = "conflicts";

SyncTask::SyncTask(const QString &source, const QString &target,
                   QObject *parent)
    : QObject(parent), sourcePath(source), targetPath(target) {
  // 使用当前时间生成任务时间戳
  taskTimestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
}

void SyncTask::run() {
  qDebug() << "🚀 Starting synchronization task...";

  // 创建目标目录、冲突目录、日志目录（如果不存在）
  ensureDirectoryExists(targetPath);
  ensureDirectoryExists(getConflictDirPath());
  ensureDirectoryExists(getLogDirPath());

  // 执行目录同步
  syncDirectory(sourcePath, targetPath);

  // 生成 Excel 日志和冲突报告（切换到主线程执行以确保 QPainter 有效）
  saveTaskLog();
  generateConflictReport();

  qDebug() << "✅ Task completed successfully";
  emit taskCompleted(sourcePath, targetPath);
}

void SyncTask::syncDirectory(const QString &source, const QString &target) {
  // 版本记录文件位于目标目录下
  const QString recordPath = QDir(target).filePath(VERSION_FILE);
  QMap<QString, QString> oldRecords = readVersionRecords(recordPath);
  QMap<QString, QString> newRecords;

  processDirectory(source, "", source, target, oldRecords, newRecords);
  writeVersionRecords(recordPath, newRecords);
}

void SyncTask::processDirectory(const QString &basePath,
                                const QString &relativePath,
                                const QString &source, const QString &target,
                                QMap<QString, QString> &oldRecords,
                                QMap<QString, QString> &newRecords) {
  QDir currentDir(QDir(basePath).filePath(relativePath));
  foreach (const QFileInfo &fileInfo,
           currentDir.entryInfoList(QDir::Files | QDir::Dirs |
                                    QDir::NoDotAndDotDot)) {
    QString newRelativePath = relativePath.isEmpty()
                                  ? fileInfo.fileName()
                                  : relativePath + "/" + fileInfo.fileName();

    if (fileInfo.isDir()) {
      processDirectory(basePath, newRelativePath, source, target, oldRecords,
                       newRecords);
      continue;
    }

    // 计算文件 MD5
    QString sourceFile = QDir(source).filePath(newRelativePath);
    QString md5 = calculateFileHash(sourceFile);
    newRecords[newRelativePath] = md5;

    QString targetFile = QDir(target).filePath(newRelativePath);
    if (!oldRecords.contains(newRelativePath) || !QFile::exists(targetFile)) {
      ensureDirectoryExists(QFileInfo(targetFile).path());
      copyFileWithLog(sourceFile, targetFile);
      continue;
    }
    if (md5 != oldRecords[newRelativePath]) {
      handleConflict(targetFile, newRelativePath);
      if (removeFileWithLog(targetFile)) {
        copyFileWithLog(sourceFile, targetFile);
      }
    }
  }
}

QString SyncTask::calculateFileHash(const QString &filePath) const {
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    // logOperation("HASH", filePath, "", false, file.errorString());
    qDebug() << "Failed to open file for hashing:" << filePath
             << file.errorString();
    return "";
  }
  QCryptographicHash hash(QCryptographicHash::Md5);
  if (hash.addData(&file)) {
    file.close();
    return hash.result().toHex();
  }
  file.close();
  return "";
}

bool SyncTask::copyFileWithLog(const QString &source, const QString &target) {
  if (QFile::exists(target)) {
    QFile::remove(target);
  }
  bool success = QFile::copy(source, target);
  QString error = success ? "" : QString("Copy error: %1").arg(source);
  logOperation("COPY", source, target, success, error);
  return success;
}

bool SyncTask::removeFileWithLog(const QString &path) {
  bool success = QFile::remove(path);
  QString error = success ? "" : QString("Delete error: %1").arg(path);
  logOperation("DELETE", path, "", success, error);
  return success;
}

void SyncTask::handleConflict(const QString &targetFile,
                              const QString &relativePath) {
  // 在冲突目录下以文件名创建子目录
  QString conflictDir =
      QDir(getConflictDirPath()).filePath(QFileInfo(relativePath).fileName());
  ensureDirectoryExists(conflictDir);

  // 备份目标文件到冲突目录
  QString backupFile =
      QDir(conflictDir).filePath(QFileInfo(targetFile).fileName());
  bool backupSuccess = QFile::copy(targetFile, backupFile);
  logOperation(
      "BACKUP", targetFile, backupFile, backupSuccess,
      backupSuccess ? "" : QString("Backup failed: %1").arg(targetFile));

  // 创建 MD5 标记文件
  if (backupSuccess) {
    QString oldHash = calculateFileHash(targetFile);
    QFile md5File(QDir(conflictDir).filePath(oldHash + ".md5flag"));
    bool md5Success = md5File.open(QIODevice::WriteOnly);
    if (md5Success) md5File.close();
    logOperation("MD5_FLAG", "", md5File.fileName(), md5Success,
                 md5Success ? "" : md5File.errorString());
  }

  // 记录冲突信息
  ConflictEntry entry;
  entry.filename = QFileInfo(targetFile).fileName();
  entry.sourcePath = QDir(sourcePath).filePath(relativePath);
  entry.targetPath = targetFile;
  entry.oldHash = calculateFileHash(targetFile);
  entry.newHash = calculateFileHash(entry.sourcePath);
  entry.conflictTime = QDateTime::currentDateTime();
  conflictEntries.append(entry);
}

QMap<QString, QString> SyncTask::readVersionRecords(const QString &recordPath) {
  QMap<QString, QString> records;
  QFile file(recordPath);
  if (file.open(QIODevice::ReadOnly)) {
    QTextStream in(&file);
    while (!in.atEnd()) {
      QString line = in.readLine().trimmed();
      QStringList parts = line.split("|");
      if (parts.size() == 2) {
        records[parts[0]] = parts[1];
      }
    }
    file.close();
  } else {
    logOperation("READ_RECORD", recordPath, "", false, file.errorString());
  }
  return records;
}

void SyncTask::writeVersionRecords(const QString &recordPath,
                                   const QMap<QString, QString> &records) {
  QFile file(recordPath);
  if (file.open(QIODevice::WriteOnly)) {
    QTextStream out(&file);
    for (auto it = records.begin(); it != records.end(); ++it) {
      out << it.key() << "|" << it.value() << "\n";
    }
    file.close();
    logOperation("WRITE_RECORD", "", recordPath, true, "");
  } else {
    logOperation("WRITE_RECORD", "", recordPath, false, file.errorString());
  }
}

void SyncTask::saveTaskLog() {
  // 切换到主线程执行 Excel 生成操作
  QMetaObject::invokeMethod(
      qApp,
      [this]() {
        QXlsx::Document xlsx;
        QXlsx::Format headerFormat;
        headerFormat.setFontBold(true);
        // 设置字体颜色为红色
        headerFormat.setFontColor(QColor(Qt::red));

        // 设置单元格背景颜色为淡绿色
        headerFormat.setPatternBackgroundColor(
            QColor(152, 251, 152));  // 浅绿色

        QStringList headers = {"Timestamp",   "Operation", "Source Path",
                               "Target Path", "Status",    "Error Message"};
        for (int col = 0; col < headers.size(); ++col) {
          xlsx.write(1, col + 1, headers[col], headerFormat);
        }
        int row = 2;
        for (const OperationLog &log : qAsConst(operationLogs)) {
          xlsx.write(row, 1, log.timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"));
          xlsx.write(row, 2, log.type);
          xlsx.write(row, 3, log.sourcePath);
          xlsx.write(row, 4, log.targetPath);
          xlsx.write(row, 5, log.status);
          xlsx.write(row, 6, log.errorMessage);
          ++row;
        }
        for (int col = 1; col <= headers.size(); ++col) {
          xlsx.setColumnWidth(col, 25);
        }
        QString logPath = QDir(getLogDirPath()).filePath("operation_log.xlsx");
        if (xlsx.saveAs(logPath)) {
          qDebug() << "📊 Operation log saved to:" << logPath;
        } else {
          qDebug() << "❌ Failed to save operation log";
        }
      },
      Qt::BlockingQueuedConnection);
}

void SyncTask::generateConflictReport() {
  if (conflictEntries.isEmpty()) return;
  QMetaObject::invokeMethod(
      qApp,
      [this]() {
        QXlsx::Document xlsx;
        QXlsx::Format headerFormat;
        headerFormat.setFontBold(true);

        // 设置字体颜色为红色
        headerFormat.setFontColor(QColor(Qt::red));

        // 设置单元格背景颜色为淡绿色
        headerFormat.setPatternBackgroundColor(
            QColor(152, 251, 152));  // 浅绿色
        QStringList headers = {"Filename", "Source Path", "Target Path",
                               "Old Hash", "New Hash",    "Conflict Time"};
        for (int col = 0; col < headers.size(); ++col) {
          xlsx.write(1, col + 1, headers[col], headerFormat);
        }
        int row = 2;
        for (const ConflictEntry &entry : qAsConst(conflictEntries)) {
          xlsx.write(row, 1, entry.filename);
          xlsx.write(row, 2, entry.sourcePath);
          xlsx.write(row, 3, entry.targetPath);
          xlsx.write(row, 4, entry.oldHash);
          xlsx.write(row, 5, entry.newHash);
          xlsx.write(row, 6,
                     entry.conflictTime.toString("yyyy-MM-dd HH:mm:ss"));
          ++row;
        }
        QString reportPath =
            QDir(getLogDirPath()).filePath("conflict_report.xlsx");
        if (xlsx.saveAs(reportPath)) {
          qDebug() << "📄 Conflict report saved to:" << reportPath;
        } else {
          qDebug() << "❌ Failed to save conflict report";
        }
      },
      Qt::BlockingQueuedConnection);
}

void SyncTask::ensureDirectoryExists(const QString &path) {
  if (!QDir(path).exists()) {
    bool success = QDir().mkpath(path);
    logOperation("CREATE_DIR", "", path, success,
                 success ? "" : "Failed to create directory");
  }
}

QString SyncTask::getLogDirPath() const {
  return QDir::current().filePath(QString("%1/%2").arg(LOG_DIR, taskTimestamp));
}

QString SyncTask::getConflictDirPath() const {
  return QDir::current().filePath(
      QString("%1/%2").arg(CONFLICT_DIR, taskTimestamp));
}

void SyncTask::logOperation(const QString &type, const QString &source,
                            const QString &target, const bool success,
                            const QString &error) {
  // 使用局部静态 QMutex 保护 operationLogs 写入，确保线程安全
  static QMutex mutex;
  QMutexLocker locker(&mutex);
  OperationLog log;
  log.timestamp = QDateTime::currentDateTime();
  log.type = type;
  log.sourcePath = source;
  log.targetPath = target;
  log.status = success ? "SUCCESS" : "FAILED";
  log.errorMessage = error;
  operationLogs.append(log);
}
