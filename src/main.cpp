#include <QSharedMemory>
#include <QSystemSemaphore>

#include "widget.h"

int main(int argc, char* argv[]) {
  QApplication a(argc, argv);

  // 创建“SingleApp”的共享内存块
  static QSharedMemory* FilesSyncSingleApp = new QSharedMemory("FilesSyncSingleApp");
  if (!FilesSyncSingleApp->create(1)) {
    QMessageBox::warning(
        nullptr, "警告",
        "该程序已经在运行中了，请勿重复运行！");  // 弹出提示框 注意：该提示应该在
                                          // qApp->quit();之前，否则提示框将会一闪而过
    a.quit();
    delete FilesSyncSingleApp;
    return -1;
  }

  Widget w;
  w.resize(1024, 768);
  w.show();
  return a.exec();
}