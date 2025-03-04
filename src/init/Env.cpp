#include "Env.h"

#include <QCoreApplication>
#include <QDir>
#include <qDebug>

Env::Env() {
  appDir = QString("%1/%2").arg(QCoreApplication::applicationDirPath(), "etc");
}

Env& Env::instance() {
  static Env envInstance;  // 静态局部变量，保证全局唯一
  return envInstance;
}

void Env::checkAndInit() const {
  if (const QDir dir(appDir); !dir.exists()) {
    if (dir.mkpath(appDir)) {
      qDebug() << "成功创建配置文件夹:"
               << appDir;
    } else {
      qDebug() << "创建配置文件夹失败";
    }
  }
}


// bool Env::isAppDirExists() const { return QDir(appDir).exists(); }
