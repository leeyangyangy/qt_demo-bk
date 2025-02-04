#include "SyncUtils.h"

#include <QDir>
#include <QFileInfo>

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
