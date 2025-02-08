#include "SyncUtils.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <qDebug>

bool SyncUtils::checkDocCompare(QString const& src, QString const& dest) {
  if (!src.compare(dest, Qt::CaseInsensitive)) {
    qInfo("bool Widget::checkDocCompare 选择失败，路径相同");
    return true;
  }
  return false;
}

bool SyncUtils::checkFileIsDir(QString const& src, QString const& dest) {
  QFileInfo file1_(src);
  QFileInfo file2_(dest);
  if (file1_.isDir() && file2_.isDir()) return true;
  return false;
}

int SyncUtils::checkFilePathPermissions(const QString& filePath) {
  QFileInfo fileInfo(filePath);

  // 检查文件是否存在
  if (!fileInfo.exists()) {
    qDebug() << "File does not exist:" << filePath;
    return -1;  // 文件不存在
  }

  QFile::Permissions perms = fileInfo.permissions();
  int flag = 0;

  // 检查读权限
  if (perms & QFile::ReadUser) {
    qDebug() << "File is readable:" << filePath;
    flag |= 1;  // 读权限标志
  } else {
    qDebug() << "File is NOT readable:" << filePath;
  }

  // 检查写权限
  if (perms & QFile::WriteUser) {
    qDebug() << "File is writable:" << filePath;
    flag |= 2;  // 写权限标志
  } else {
    qDebug() << "File is NOT writable:" << filePath;
  }

  // 返回值：-1（不存在），0（无权限），1（仅读），2（仅写），3（读写）
  return flag;
}

bool SyncUtils::testFilePathWritablePermissions(const QString& path) {
  // 获取当前日期和时间
  QDateTime currentDateTime = QDateTime::currentDateTime();

  // 将日期和时间格式化为字符串
  QString dateTimeString =
      currentDateTime.toString("yyyy-MM-dd_hh-mm-ss");  // 使用下划线替代冒号

  // 拼接字符串
  QString folderName = "tmp_" + dateTimeString;
  QString fullPath = path + "/" + folderName;

  // 检查目录是否存在
  if (const QDir dir; !dir.exists(fullPath)) {
    // 尝试创建目录
    if (dir.mkdir(fullPath)) {
      // qDebug() << "Directory created successfully:" << fullPath;

      // 删除目录
      if (dir.rmdir(fullPath)) {
        // qDebug() << "Directory deleted successfully:" << fullPath;
      } else {
        qDebug() << "Failed to delete directory:" << fullPath;
      }
      return true;
    }
    qDebug() << "Failed to create directory:" << fullPath;
    return false;
  }
  qDebug() << "Directory already exists:" << fullPath;
  return false;
}
