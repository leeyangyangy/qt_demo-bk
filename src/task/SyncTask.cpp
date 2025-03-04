#include "SyncTask.h"

#include <QApplication>
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonObject>
#include <QMutexLocker>
#include <QTextStream>

// 包含 QXlsx 相关头文件（确保 CMake 配置正确）
#include "../utils/SyncUtils.h"
#include "xlsxdocument.h"

// LevelDB 头文件
#include <thread>

#include "leveldb/db.h"
#include "leveldb/write_batch.h"
#include "xxhash.h"

// 定义常量
const QString SyncTask::VERSION_FILE = "version_records";  // 用作数据库标识
const QString SyncTask::LOG_DIR = "logs";
const QString SyncTask::CONFLICT_DIR = "conflicts";
/**
 * @brief
 * @param source 源目录
 * @param target 目标目录
 * @param parent 父对象
 * @TODO
 *        优化目标，
 *        1.
 * 减少配置项的重复io读写，应该从env类中，在widget中初始化时候就读入内存，
 *        2. 同步时候为什么没有得到预期结果？找出问题所在
 *        3. 数据库操作，应该及时写入，而不是等待线程结束后才写入
 *        4.
 * 设置缓存门槛，防止堆积大量数据，导致内存溢出问题，同时防止在低配置电脑上出现不可预期问题
 */
SyncTask::SyncTask(const QString &source, const QString &target,
                   QObject *parent)
    : QObject(parent),
      sourcePath(source),
      targetPath(target),
      versionDB(nullptr) {
  // 使用当前时间生成任务时间戳
  taskTimestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");

  // 初始化 RulesParser，从可执行程序目录下 etc/FileSyncRules.conf 加载规则
  const QString configPath =
      QCoreApplication::applicationDirPath() + "/etc/FileSyncRules.conf";
  rulesParser = new RulesParser(configPath);

  // 初始化 LevelDB 数据库，存储在可执行程序目录下的 db 文件夹中
  const QString dbDir =
      QString("%1/%2").arg(QCoreApplication::applicationDirPath(), "db");
  ensureDirectoryExists(dbDir);
  // const QString dbPath = QDir(dbDir).filePath(VERSION_FILE);

  const QString dbPath = QDir(dbDir).filePath(SyncUtils::computeXXHash(target.split("/").last()));
  leveldb::Options options;
  options.create_if_missing = true;
  const leveldb::Status status =
      leveldb::DB::Open(options, dbPath.toStdString(), &versionDB);
  if (!status.ok()) {
    qWarning() << "cant load or create LevelDB :"
               << QString::fromStdString(status.ToString());
    versionDB = nullptr;
  }
}

SyncTask::~SyncTask() {
  delete rulesParser;
  delete versionDB;
}

void SyncTask::run() {
  qDebug() << "🚀 Starting" << sourcePath.split("/").last()
           << " synchronization task...";
  if (!SyncUtils::testFilePathWritablePermissions(targetPath)) {
    qDebug() << "🚫 Target {" << targetPath
             << "} is not writable! synchronization task aborted.";
    // emit taskCompleted(sourcePath, targetPath);
    // return;
  }
  // 确保目标目录存在（版本记录存储在 LevelDB 中，与目标目录无关）
  ensureDirectoryExists(targetPath);
  // 备份目录和日志目录均采用程序运行目录
  ensureDirectoryExists(getConflictDirPath());
  ensureDirectoryExists(getLogDirPath());

  // 执行目录同步
  syncDirectory(sourcePath, targetPath);

  // 生成 Excel 日志和冲突报告（切换到主线程执行以确保 QPainter 有效）
  saveTaskLog();
  generateConflictReport();

  qDebug() << "✅ Task" << sourcePath.split("/").last()
           << "completed successfully";
  emit taskCompleted(sourcePath, targetPath);
  // 清理目录
  // autoCleanWorkingDir(taskTimestamp, LOG_DIR, CONFLICT_DIR);
}

void SyncTask::syncDirectory(const QString &source, const QString &target) {
  // 从 LevelDB 中读取旧版本记录
  QMap<QString, QString> oldRecords = readVersionRecords(QString());
  QMap<QString, QString> newRecords;

  processDirectory(source, "", source, target, oldRecords, newRecords);

  // 将新版本记录写入 LevelDB
  writeVersionRecords(QString(), newRecords);
}

void SyncTask::processDirectory(const QString &basePath,
                                const QString &relativePath,
                                const QString &source, const QString &target,
                                QMap<QString, QString> &oldRecords,
                                QMap<QString, QString> &newRecords) {
  // 如果正在处理的目录正好为程序运行目录，则不进行同步
  if (QDir(basePath).absolutePath() == QDir::current().absolutePath()) {
    qDebug() << "Skipping program directory:" << basePath;
    return;
  }

  const QDir currentDir(QDir(basePath).filePath(relativePath));
  QFileInfoList entries =
      currentDir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
  foreach (const QFileInfo &fileInfo, entries) {
    QString newRelativePath = relativePath.isEmpty()
                                  ? fileInfo.fileName()
                                  : relativePath + "/" + fileInfo.fileName();

    // 如果目标版本记录、日志、冲突目录或RulesParser属于同步内容，则跳过
    if (newRelativePath == VERSION_FILE ||
        newRelativePath.startsWith(LOG_DIR) ||
        newRelativePath.startsWith(CONFLICT_DIR) ||
        shouldExclude(newRelativePath)) {
      // qDebug() << "Skipping reserved path:" << newRelativePath;
      continue;
    }

    if (fileInfo.isDir()) {
      processDirectory(basePath, newRelativePath, source, target, oldRecords,
                       newRecords);
      continue;
    }

    // 计算源文件 MD5 值
    QString sourceFile = QDir(source).filePath(newRelativePath);
    QString md5 = calculateFileHash(sourceFile);
    newRecords[newRelativePath] = md5;

    QString targetFile = QDir(target).filePath(newRelativePath);
    // 若目标中不存在此文件，则直接拷贝
    if (!oldRecords.contains(newRelativePath) || !QFile::exists(targetFile)) {
      ensureDirectoryExists(QFileInfo(targetFile).path());
      copyFileWithLog(sourceFile, targetFile);
      continue;
    }
    // 若 MD5 不同，则先备份目标文件，再覆盖
    if (md5 != oldRecords[newRelativePath]) {
      handleConflict(targetFile, newRelativePath);
      if (removeFileWithLog(targetFile)) {
        copyFileWithLog(sourceFile, targetFile);
      }
    }
  }
}

QString SyncTask::calculateFileHash(const QString &filePath) {
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly)) {
    qDebug() << "Failed to open file for hashing:" << filePath
             << file.errorString();
    return "";
  }

  // 创建 xxHash 状态，使用种子 0
  XXH64_state_t *state = XXH64_createState();
  if (state == nullptr) {
    qDebug() << "Failed to create xxHash state";
    file.close();
    return "";
  }
  if (XXH64_reset(state, 0) != XXH_OK) {
    qDebug() << "Failed to reset xxHash state";
    XXH64_freeState(state);
    file.close();
    return "";
  }

  // 采用 8KB 缓冲区分块读取文件数据
  constexpr qint64 bufferSize = 8192;
  char buffer[bufferSize];
  qint64 bytesRead = 0;
  while ((bytesRead = file.read(buffer, bufferSize)) > 0) {
    if (XXH64_update(state, buffer, static_cast<size_t>(bytesRead)) != XXH_OK) {
      qDebug() << "xxHash update failed";
      XXH64_freeState(state);
      file.close();
      return "";
    }
  }
  const unsigned long long hashValue = XXH64_digest(state);
  XXH64_freeState(state);
  file.close();

  // 转换为十六进制字符串（不保证固定宽度，如需要可自行填充0）
  QString hashHex = QString::number(hashValue, 16);
  // qDebug () << "Hash of" << filePath << "is" << hashHex;
  return hashHex;
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
  // 备份目标文件的目录：程序运行目录/conflicts/<任务时间戳>/<文件名>/
  const QString fileName = QFileInfo(targetFile).fileName();
  const QString conflictDir = QString("%1/%2/%3/%4")
                            .arg(QCoreApplication::applicationDirPath(),
                                 CONFLICT_DIR, taskTimestamp, fileName);
  qDebug() << "conflictDir :" << conflictDir;
  ensureDirectoryExists(conflictDir);

  // 备份文件路径：冲突目录下原文件名
  QString backupFile = QDir(conflictDir).filePath(fileName);
  bool backupSuccess = QFile::copy(targetFile, backupFile);
  logOperation("BACKUP", targetFile, backupFile, backupSuccess,
               backupSuccess ? "" : "Backup failed");

  if (backupSuccess) {
    // 创建 MD5 标记文件：文件名.md5
    const QString hashValue = calculateFileHash(targetFile);
    const QString md5FilePath = QDir(conflictDir).filePath(hashValue);
    QFile md5File(md5FilePath);
    const bool md5Success = md5File.open(QIODevice::WriteOnly);
    if (md5Success) {
      QTextStream out(&md5File);
      out << hashValue;
      md5File.close();
    }
    logOperation("MD5_FLAG", "", md5FilePath, md5Success,
                 md5Success ? "" : "Failed to create MD5 file");
  }

  // 记录冲突信息
  ConflictEntry entry;
  entry.filename = fileName;
  entry.sourcePath = QDir(sourcePath).filePath(relativePath);
  entry.targetPath = targetFile;
  entry.oldHash = calculateFileHash(targetFile);
  entry.newHash = calculateFileHash(entry.sourcePath);
  entry.conflictTime = QDateTime::currentDateTime();
  conflictEntries.append(entry);
}

// TODO 读写优化
QMap<QString, QString> SyncTask::readVersionRecords(
    const QString & /*recordPath*/) const {
  QMap<QString, QString> records;
  if (!versionDB) return records;

  leveldb::Iterator *it = versionDB->NewIterator(leveldb::ReadOptions());
  for (it->SeekToFirst(); it->Valid(); it->Next()) {
    QString key = QString::fromStdString(it->key().ToString());
    QString value = QString::fromStdString(it->value().ToString());
    records[key] = value;
  }
  delete it;
  return records;
}

// TODO 读写优化
void SyncTask::writeVersionRecords(const QString & /*recordPath*/,
                                   const QMap<QString, QString> &records) {
  if (!versionDB) return;

  leveldb::WriteBatch batch;
  QMapIterator<QString, QString> it(records);
  while (it.hasNext()) {
    it.next();
    batch.Put(it.key().toStdString(), it.value().toStdString());
  }
  leveldb::Status s = versionDB->Write(leveldb::WriteOptions(), &batch);
  if (!s.ok()) {
    qWarning() << "write LevelDB fail:" << QString::fromStdString(s.ToString());
  } else {
    logOperation("WRITE_RECORD", "", "LevelDB", true, "");
  }
}

void SyncTask::saveTaskLog() {
  // 切换到主线程执行 Excel 生成操作
  QMetaObject::invokeMethod(
      qApp,
      [this] {
        QXlsx::Document xlsx;
        QXlsx::Format headerFormat;
        headerFormat.setFontBold(true);
        headerFormat.setFontColor(QColor(Qt::red));
        headerFormat.setPatternBackgroundColor(QColor(152, 251, 152));

        QStringList headers = {"Timestamp",   "Operation", "Source Path",
                               "Target Path", "Status",    "Error Message"};
        for (int col = 0; col < headers.size(); ++col) {
          xlsx.write(1, col + 1, headers[col], headerFormat);
        }
        int row = 2;
        for (const auto &[timestamp, type, sourcePath, targetPath, status,
                          errorMessage] : qAsConst(operationLogs)) {
          xlsx.write(row, 1, timestamp.toString("yyyy-MM-dd HH:mm:ss.zzz"));
          xlsx.write(row, 2, type);
          xlsx.write(row, 3, sourcePath);
          xlsx.write(row, 4, targetPath);
          xlsx.write(row, 5, status);
          xlsx.write(row, 6, errorMessage);
          ++row;
        }
        for (int col = 1; col <= headers.size(); ++col) {
          xlsx.setColumnWidth(col, 50);
        }
        QString logPath =
          QString("%1/%2").arg(getLogDirPath(), "operation_log.xlsx");
        if (xlsx.saveAs(logPath)) {
          // qDebug() << "📊 Operation log saved to:" << logPath;
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
      [this] {
        QXlsx::Document xlsx;
        QXlsx::Format headerFormat;
        headerFormat.setFontBold(true);
        headerFormat.setFontColor(QColor(Qt::red));
        headerFormat.setPatternBackgroundColor(QColor(152, 251, 152));

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

        // 调整列宽
        xlsx.setColumnWidth(1, 30);  // Filename
        xlsx.setColumnWidth(2, 50);  // Source Path
        xlsx.setColumnWidth(3, 50);  // Target Path
        xlsx.setColumnWidth(4, 40);  // Old Hash
        xlsx.setColumnWidth(5, 40);  // New Hash
        xlsx.setColumnWidth(6, 25);  // Conflict Time

        const QString reportPath =
            QString("%1/%2").arg(getLogDirPath(), "conflict_report.xlsx");
        if (xlsx.saveAs(reportPath)) {
          // qDebug() << "📄 Conflict report saved to:" << reportPath;
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
  // 日志目录放在程序运行目录下
  return QString("%1/%2/%3")
      .arg(QCoreApplication::applicationDirPath(), LOG_DIR, taskTimestamp);
}

QString SyncTask::getConflictDirPath() const {
  // 冲突目录放在程序运行目录下
  return QString("%1/%2/%3")
      .arg(QCoreApplication::applicationDirPath(), CONFLICT_DIR, taskTimestamp);
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

bool SyncTask::shouldExclude(const QString &relativePath) const {
  if (!rulesParser) return false;

  // 检查是否匹配排除的目录规则
  for (const QString &dir : rulesParser->getExcludeDirs()) {
    if (relativePath.contains(dir, Qt::CaseInsensitive)) return true;
  }
  // 检查是否匹配排除的文件扩展名规则
  for (const QString &ext : rulesParser->getExcludeExts()) {
    if (relativePath.endsWith(ext, Qt::CaseInsensitive)) return true;
  }
  return false;
}

void SyncTask::autoCleanWorkingDir(const QString &, const QString &,
                                   const QString &) const {
  // 获取冲突目录路径（例如
  // D:/Coding/CLionProjects/FilesSync/build/src/conflicts/20250303_001328/）
  QDir conflictDir(getConflictDirPath());

  // 获取日志目录路径（例如
  // D:/Coding/CLionProjects/FilesSync/build/src/logs/20250303_001328/）
  QDir logDir(getLogDirPath());

  // 检查冲突目录是否存在
  if (!conflictDir.exists()) {
    return;
  }

  // 检查冲突目录是否为空
  const QStringList conflictEntries =
      conflictDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
  if (conflictEntries.isEmpty()) {
    // 删除冲突目录
    if (!conflictDir.removeRecursively()) {
      return;
    }
    // 删除日志目录
    if (logDir.exists())
      if (!logDir.removeRecursively()) {
      }
  }
}
