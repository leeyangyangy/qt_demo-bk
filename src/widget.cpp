#include "widget.h"

#include <qpointer.h>

#include "task/FileMonitor.h"
#include "task/SyncTask.h"
#include "utils/SyncUtils.h"

// 定义静态成员变量
QTextEdit* Widget::globalLogArea = nullptr;
QMutex Widget::logMutex;

Widget::Widget(QWidget* parent)
    : QMainWindow(parent),
      workspaceLayout(new QVBoxLayout()),
      logArea(nullptr),
      configFilePath("files.json"),
      configSystemPath("etc.json"),
      ui(new Ui::Widget) {
  setWindowTitle("-- 文件同步管理器 -- 测试版 V1.0 24-1231 制作：LEEYANGY");
  setWindowIcon(QIcon(":/svg/files.png"));

  // 在获取布局时候初始化监控进程
  monitor = new TriggerMonitor(this,this);
  monitor->startMonitoring();

  // 创建系统托盘图标
  // trayIcon = new QSystemTrayIcon(this);
  // 初始化 trayIcon
  trayIcon.reset(new QSystemTrayIcon(this));
  // 设置托盘图标
  trayIcon->setIcon(QIcon(":/png/filesync.png"));

  trayIcon->setToolTip("FilesSync");

  QMenu* trayMenu = new QMenu(this);

  QAction* restoreAction = trayMenu->addAction(
      QIcon(":/svg/files.svg"), "显示主窗口", this, &Widget::showNormal);

  QAction* exitAction = trayMenu->addAction(QIcon(":/svg/exit.svg"), "退出",
                                            this, &Widget::onExit);

  trayIcon->setContextMenu(trayMenu);
  trayIcon->setVisible(true);

  // 设置托盘图标
  trayIcon->setIcon(QIcon(":/png/filesync.png"));

  // // 处理托盘图标的点击事件
  // QObject::connect(trayIcon, &QSystemTrayIcon::activated,
  // [&](QSystemTrayIcon::ActivationReason reason) {
  //     if (reason == QSystemTrayIcon::Trigger) {
  //         QMetaObject::invokeMethod(this, [this]() {
  //             QMessageBox::information(nullptr, "Tray Icon Clicked", "You
  //             clicked the tray icon.");
  //         });
  //     }
  // });

  // 处理托盘图标的点击事件
  connect(trayIcon.data(), &QSystemTrayIcon::activated,
          [&](QSystemTrayIcon::ActivationReason reason) {
            if (reason == QSystemTrayIcon::Trigger) {
              QMetaObject::invokeMethod(this, [this]() {
                QMessageBox::information(nullptr, "Tray Icon Clicked",
                                         "You clicked the tray icon.");
              });
            }
          });

  QWidget* centralWidget = new QWidget(this);
  QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

  setupMenuBar();
  setupWorkspace(mainLayout);
  setupLogArea(mainLayout);
  setupProgressArea(mainLayout);

  setCentralWidget(centralWidget);

  globalLogArea = logArea;
  qInstallMessageHandler(logMessageHandler);

  loadWorkspaceConfig();
}

Widget::~Widget() {
  saveWorkspaceConfig();
  monitor->stopMonitoring();
  // 释放内存？
  delete ui;
  // delete trayIcon;
  // delete progressBar;
  // delete taskListWidget;
}

void Widget::closeEvent(QCloseEvent* event) {
  saveWorkspaceConfig();
  // QMainWindow::closeEvent(event); // 关闭
  this->showMinimized();  // 仅最小化
  event->ignore();        // 阻止关闭
}

void Widget::setupMenuBar() {
  const auto menuBar = new QMenuBar(this);

  QMenu* confMenu = new QMenu("配置(C)", this);
  QShortcut* confShortcut = new QShortcut(QKeySequence("C"), this);
  connect(confShortcut, &QShortcut::activated, this, &Widget::onNewProject);

  QAction* newAction = confMenu->addAction(
      QIcon(":/svg/new_conf.svg"), "新建配置", this, &Widget::onNewProject);
  newAction->setShortcut(QKeySequence("Ctrl+N"));

  QAction* openAction =
      confMenu->addAction(QIcon(":/svg/open_conf.svg"), "打开配置", this,
                          &Widget::onOpenConfiguration);
  openAction->setShortcut(QKeySequence("Ctrl+O"));

  QAction* saveAction = confMenu->addAction(QIcon(":/svg/save.svg"), "保存配置",
                                            this, &Widget::onSaveConfiguration);
  saveAction->setShortcut(QKeySequence("Ctrl+S"));

  QAction* ruleAction = confMenu->addAction(QIcon(":/svg/rules.svg"),
                                            "规则引擎", this, &Widget::onRules);
  ruleAction->setShortcut(QKeySequence("Ctrl+Shift+R"));

  QAction* exitAction = confMenu->addAction(QIcon(":/svg/exit.svg"), "退出",
                                            this, &Widget::onExit);
  exitAction->setShortcut(QKeySequence("Esc"));

  menuBar->addMenu(confMenu);

  QMenu* syncMenu = new QMenu("同步(S)", this);
  QShortcut* syncShortcut = new QShortcut(QKeySequence("S"), this);
  connect(syncShortcut, &QShortcut::activated, this, &Widget::onSync);

  // QAction* syncAction = syncMenu->addAction(QIcon(":/svg/sync.svg"),
  // "立即同步", this, &Widget::onSync);

  syncAction = syncMenu->addAction(QIcon(":/svg/sync.svg"), "立即同步", this,
                                   &Widget::onSync);

  syncAction->setShortcut(QKeySequence("Ctrl+Shift+S"));

  // 点击按钮时启动同步任务
  connect(syncAction, &QAction::triggered, this, &Widget::onSyncActionClicked);
  // 当 TriggerMonitor 触发同步任务时通知
  // qDebug() << "monitor pointer: " << monitor;
  // connect(monitor, &TriggerMonitor::syncTriggered, this,
  // &Widget::onSyncTriggered);
  connect(monitor, &TriggerMonitor::syncTriggered, this,
          &Widget::onSyncTriggered);
  // qDebug() << "connect result: " << success;

  // 同步任务完成时恢复按钮状态
  connect(monitor, &TriggerMonitor::syncTriggered, this,
          &Widget::onTaskCompleted);

  triggerTimeAction = syncMenu->addAction(
      QIcon(":/svg/hourglass.svg"), "触发时间", this, &Widget::onSystemInfo);
  triggerTimeAction->setShortcut(QKeySequence("Ctrl+Shift+T"));

  menuBar->addMenu(syncMenu);

  QMenu* helpMenu = new QMenu("帮助(H)", this);
  QShortcut* helpShortcut = new QShortcut(QKeySequence("H"), this);
  connect(helpShortcut, &QShortcut::activated, this, &Widget::onAbout);

  QAction* aboutAction =
      new QAction(QIcon(":/svg/author.svg"), tr("&关于"), this);
  aboutAction->setShortcut(QKeySequence("Ctrl+A"));
  connect(aboutAction, &QAction::triggered, this, &Widget::onAbout);
  helpMenu->addAction(aboutAction);

  QAction* updateLogAction =
      new QAction(QIcon(":/svg/update-log.svg"), tr("&更新日志"), this);
  updateLogAction->setShortcut(QKeySequence("Ctrl+U"));
  connect(updateLogAction, &QAction::triggered, this, &Widget::onUpdateLog);
  helpMenu->addAction(updateLogAction);

  QAction* getatestVersionAction =
      new QAction(QIcon(":/svg/update.svg"), tr("&获取新版本"), this);
  getatestVersionAction->setShortcut(QKeySequence("Ctrl+G"));
  connect(getatestVersionAction, &QAction::triggered, this,
          &Widget::onGetLatestVersion);
  helpMenu->addAction(getatestVersionAction);

  menuBar->addMenu(helpMenu);
  menuBar->addSeparator();
  setMenuBar(menuBar);
}

void Widget::setupWorkspace(QVBoxLayout* mainLayout) {
  QGroupBox* workspaceGroup = new QGroupBox("工作空间", this);
  workspaceLayout = new QVBoxLayout(workspaceGroup);

  QScrollArea* scrollArea = new QScrollArea(this);
  QWidget* scrollContent = new QWidget(this);
  scrollContent->setLayout(workspaceLayout);
  scrollArea->setWidget(scrollContent);
  scrollArea->setWidgetResizable(true);

  int fixedHeight = 200;
  scrollArea->setFixedHeight(fixedHeight);

  QPushButton* addButton = new QPushButton("+ 添加新行", this);
  connect(addButton, &QPushButton::clicked, this, [this]() { addFileRow(); });

  QVBoxLayout* groupLayout = new QVBoxLayout(workspaceGroup);
  groupLayout->addWidget(scrollArea);
  groupLayout->addWidget(addButton);

  mainLayout->addWidget(workspaceGroup);
}

void Widget::setupLogArea(QVBoxLayout* mainLayout) {
  QLabel* logLabel = new QLabel("日志", this);
  logArea = new QTextEdit(this);
  logArea->setReadOnly(true);

  QPushButton* clearLogButton = new QPushButton("清空日志", this);
  connect(clearLogButton, &QPushButton::clicked, logArea, &QTextEdit::clear);

  QHBoxLayout* logControlLayout = new QHBoxLayout();
  logControlLayout->addWidget(logLabel);
  logControlLayout->addStretch();
  logControlLayout->addWidget(clearLogButton);

  mainLayout->addLayout(logControlLayout);
  mainLayout->addWidget(logArea);
}

/**
 * @brief 进度条显示区域设置
 *
 * @param mainLayout 布局容器
 * @return void
 * @note
 */
void Widget::setupProgressArea(QVBoxLayout* mainLayout) {
  QLabel* progLabel = new QLabel("任务进度", this);
  progressBar.reset(new QProgressBar());  // 重新初始化
  progressBar->setRange(0, 100);          // 进度条范围 0-100
  progressBar->setValue(0);               // 初始值为 0
  mainLayout->addWidget(progLabel);
  mainLayout->addWidget(progressBar.get());  // 添加到布局

  // 存储进度条指针，方便后续更新
  // this->progressBar = progressBar;
}

/**
 * @brief 更新进度条
 *
 * @param value 进度条值
 * @return void
 */
void Widget::updateProgressBar(int progress) const {
  if (progressBar) {
    progressBar->setValue(progress);  // 更新进度条的值
  }
}

// bool Widget::checkDocCompare(const QString& src, const QString& dest) {
//   if (!src.compare(dest, Qt::CaseInsensitive)) {
//     qInfo("bool Widget::checkDocCompare 选择失败，路径相同");
//     return true;
//   } else {
//     return false;
//   }
// }

void Widget::addFileRow() {
  QHBoxLayout* rowLayout = new QHBoxLayout();

  // 使用 QPointer 管理控件指针
  QPointer<QPushButton> fileDialogButton1 =
      new QPushButton("选择监听文件", this);
  QPointer<QLineEdit> fileInfo1 = new QLineEdit(this);
  fileInfo1->setReadOnly(true);

  QPointer<QPushButton> fileDialogButton2 =
      new QPushButton("选择同步文件夹", this);
  QPointer<QLineEdit> fileInfo2 = new QLineEdit(this);
  fileInfo2->setReadOnly(true);

  QPointer<QPushButton> deleteButton = new QPushButton("删除", this);

  // 连接信号槽
  connect(fileDialogButton1, &QPushButton::clicked, this,
          [fileInfo1, fileInfo2, this]() {
            QString filePath = QFileDialog::getExistingDirectory(
                nullptr, "选择监听原始文件夹");
            if (!filePath.isEmpty()) {
              if (SyncUtils::checkDocCompare(fileInfo2->text(), filePath)) {
                QMessageBox::warning(this, "警告",
                                     "选择的监听文件夹不能与目标文件夹相同");
                fileInfo1->setText("");
                return;
              }
              fileInfo1->setText(filePath);
            }
          });

  connect(fileDialogButton2, &QPushButton::clicked, this,
          [fileInfo1, fileInfo2, this]() {
            QString filePath = QFileDialog::getExistingDirectory(
                nullptr, "选择目标路径文件夹");
            if (!filePath.isEmpty()) {
              if (SyncUtils::checkDocCompare(fileInfo1->text(), filePath)) {
                QMessageBox::warning(this, "警告",
                                     "选择的目标文件夹不能与监听文件夹相同");
                fileInfo2->setText("");
                return;
              }
              fileInfo2->setText(filePath);
            }
          });

  connect(deleteButton, &QPushButton::clicked, this,
          [this, rowLayout, fileInfo1, fileInfo2, fileDialogButton1,
           fileDialogButton2, deleteButton]() {
            // 从布局中移除控件
            rowLayout->removeWidget(fileDialogButton1);
            rowLayout->removeWidget(fileInfo1);
            rowLayout->removeWidget(fileDialogButton2);
            rowLayout->removeWidget(fileInfo2);
            rowLayout->removeWidget(deleteButton);

            // 删除控件
            fileDialogButton1->deleteLater();
            fileInfo1->deleteLater();
            fileDialogButton2->deleteLater();
            fileInfo2->deleteLater();
            deleteButton->deleteLater();

            // 删除布局
            workspaceLayout->removeItem(rowLayout);
            rowLayout->deleteLater();
          });

  // 添加控件到布局
  rowLayout->addWidget(fileDialogButton1);
  rowLayout->addWidget(fileInfo1);
  rowLayout->addWidget(fileDialogButton2);
  rowLayout->addWidget(fileInfo2);
  rowLayout->addWidget(deleteButton);

  // 添加布局到工作空间
  workspaceLayout->addLayout(rowLayout);
}

void Widget::loadWorkspaceConfig() {
  QFile configFile(configFilePath);
  if (!configFile.open(QIODevice::ReadOnly)) {
    qWarning() << "无法打开配置文件，创建默认配置";
    saveWorkspaceConfig();
    return;
  }

  QByteArray data = configFile.readAll();
  configFile.close();

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (!doc.isObject()) {
    qWarning() << "配置文件格式错误，创建默认配置";
    saveWorkspaceConfig();
    return;
  }

  QJsonArray rows = doc.object().value("workspace").toArray();
  for (const QJsonValue& value : rows) {
    QJsonObject row = value.toObject();

    QString file1 = row.value("file1").toString();
    QString file2 = row.value("file2").toString();

    addFileRow();
    QLayoutItem* item = workspaceLayout->itemAt(workspaceLayout->count() - 1);
    QHBoxLayout* rowLayout =
        item ? qobject_cast<QHBoxLayout*>(item->layout()) : nullptr;
    if (rowLayout) {
      QLineEdit* fileInfo1 =
          qobject_cast<QLineEdit*>(rowLayout->itemAt(1)->widget());
      QLineEdit* fileInfo2 =
          qobject_cast<QLineEdit*>(rowLayout->itemAt(3)->widget());
      if (fileInfo1 && fileInfo2) {
        fileInfo1->setText(file1);
        fileInfo2->setText(file2);
      }
    }
  }
}

void Widget::saveWorkspaceConfig() {
  QJsonArray rows;
  for (int i = 0; i < workspaceLayout->count(); ++i) {
    QLayoutItem* item = workspaceLayout->itemAt(i);
    QHBoxLayout* rowLayout =
        item ? qobject_cast<QHBoxLayout*>(item->layout()) : nullptr;
    if (!rowLayout) continue;

    QLineEdit* fileInfo1 =
        qobject_cast<QLineEdit*>(rowLayout->itemAt(1)->widget());
    QLineEdit* fileInfo2 =
        qobject_cast<QLineEdit*>(rowLayout->itemAt(3)->widget());

    if (fileInfo1 && fileInfo2) {
      QJsonObject row;
      row["file1"] = fileInfo1->text();
      row["file2"] = fileInfo2->text();
      rows.append(row);
    }
  }

  QJsonObject config;
  config["workspace"] = rows;

  QJsonDocument doc(config);
  QFile configFile(configFilePath);
  if (!configFile.open(QIODevice::WriteOnly)) {
    qWarning() << "无法写入配置文件";
    return;
  }

  configFile.write(doc.toJson());
  configFile.close();
}

void Widget::clearWorkspaceConfig() {
  if (!workspaceLayout) {
    qDebug() << "workspaceLayout 为空，无需清理";
    return;
  }

  qDebug() << "开始清理所有文件行";

  // 遍历 workspaceLayout
  while (workspaceLayout->count() > 0) {
    QLayoutItem* item = workspaceLayout->takeAt(0);
    if (!item) continue;

    QHBoxLayout* rowLayout = qobject_cast<QHBoxLayout*>(item->layout());
    if (!rowLayout) {
      delete item;
      continue;
    }

    // 遍历 QHBoxLayout 并删除控件
    while (rowLayout->count() > 0) {
      QLayoutItem* rowItem = rowLayout->takeAt(0);
      if (!rowItem) continue;

      QWidget* widget = rowItem->widget();
      if (widget) {
        widget->hide();
        widget->deleteLater();
      }

      delete rowItem;
    }

    // 删除行布局
    delete rowLayout;
  }

  qDebug() << "文件行清理完成";
}

void Widget::onNewProject() {
  qDebug() << "新建项目";
  clearWorkspaceConfig();
}

void Widget::onOpenConfiguration() {
  qInfo() << "加载配置...";

  QString selectedFilePath = QFileDialog::getOpenFileName(
      this, tr("选择配置文件"), "", tr("配置文件 (*.json);;所有文件 (*)"));

  if (!selectedFilePath.isEmpty()) {
    // 先清除当前配置
    clearWorkspaceConfig();

    // 设置新配置路径并加载
    configFilePath = selectedFilePath;
    loadWorkspaceConfig();
    qInfo() << "加载配置完成。";
  } else {
    qInfo() << "未选择文件，取消加载配置。";
  }
}

void Widget::onExit() { QApplication::quit(); }

void Widget::onAbout() {
  QDialog dialog(this);
  dialog.setWindowTitle("关于文件同步管理器");
  dialog.setFixedSize(800, 480);

  QVBoxLayout* layout = new QVBoxLayout(&dialog);

  auto* lintr =
      new QLabel("软件简介：基于C++&Qt开发的文件同步管理器。", &dialog);
  auto* lauthor = new QLabel("作者：LEEYANGY", &dialog);
  auto* laddr = new QLabel("开源地址：", &dialog);

  QLabel* linkLabel = new QLabel(
      "<a "
      "href=\"https://github.com/leeyangyangy/qt_demo-bk/tree/"
      "filesync-cmake-clion\">"
      "https://github.com/leeyangyangy/qt_demo-bk/tree/filesync-cmake-clion</"
      "a>",
      &dialog);
  linkLabel->setTextInteractionFlags(Qt::TextSelectableByMouse |
                                     Qt::LinksAccessibleByMouse);
  linkLabel->setOpenExternalLinks(true);  // 允许点击打开链接

  layout->addWidget(lintr);
  layout->addWidget(lauthor);
  layout->addWidget(laddr);
  layout->addWidget(linkLabel);

  QPushButton* okButton = new QPushButton("确定", &dialog);
  layout->addWidget(okButton);
  connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);

  dialog.setLayout(layout);
  dialog.exec();
}

void Widget::onSync() {
  // QFile configFile(configFilePath);
  // if (!configFile.open(QIODevice::ReadOnly)) {
  //   qWarning() << "无法打开配置文件，创建默认配置";
  //   return;
  // }
  //
  // QByteArray data = configFile.readAll();
  // configFile.close();
  //
  // QJsonDocument doc = QJsonDocument::fromJson(data);
  // if (!doc.isObject()) {
  //   qWarning() << "配置文件格式错误，创建默认配置";
  //   return;
  // }
  // int taskCount = 0;
  // completedTasks.store(0);        // 任务开始前重置计数器
  // updateProgressBar(taskCount);
  // QJsonArray rows = doc.object().value("workspace").toArray();
  // int totalTasks = rows.size();
  // for (const QJsonValue& value : rows) {
  //   QJsonObject row = value.toObject();
  //
  //   QString file1 = row.value("file1").toString();
  //   QString file2 = row.value("file2").toString();
  //
  //   if (SyncUtils::checkDocCompare(file1, file2)) {
  //     QMessageBox::about(this, "提示", "文件夹路径相同，请检查并修改");
  //     return;
  //   }
  //
  //   if (!SyncUtils::checkFileIsDir(file1, file2)) {
  //     QMessageBox::warning(this, "提示",
  //                          QString("监听路径\n%1\n"
  //                                  "或\n"
  //                                  "目标路径\n%2\n不是文件夹，请检查并修改")
  //                              .arg(file1)
  //                              .arg(file2));
  //     return;
  //   }
  //
  //   if (file1.isEmpty() || file2.isEmpty()) {
  //     QMessageBox::about(this, "提示", "文件夹选择有误，请检查并修改");
  //   } else {
  //     ++taskCount;
  //     this->syncAction->setEnabled(false);
  //     const auto task = new SyncTask(file1, file2);
  //     connect(task, &SyncTask::taskCompleted, this,
  //             [this, taskCount, totalTasks](const QString& source,
  //                                           const QString& target) {
  //               this->completedTasks.fetch_add(
  //                   1, std::memory_order_relaxed);  // **原子增加**
  //               int progress =
  //                   static_cast<double>(completedTasks) / totalTasks * 100;
  //               qInfo() << "同步完成:" << source << " -> " << target
  //                       << "taskCount:" << taskCount
  //                       << "completedTasks:" << completedTasks
  //                       << "totalTasks:" << totalTasks
  //                       << "progress:" << progress;
  //
  //               updateProgressBar(progress);
  //               if (completedTasks == taskCount) {
  //                 // QMessageBox::about(this, "任务完成",
  //                 // "所有同步任务已完成！");
  //                 syncAction->setEnabled(true);
  //               }
  //             });
  //
  //     QThreadPool::globalInstance()->start(task);
  //   }
  // }
  monitor->triggerSync();
}

// TODO
void Widget::onUpdateLog() {
  QMessageBox::about(this, "更新日志",
                     "开发计划--> 1.2 支持局域网文件互传\n"
                     "当前开发进度-->即将支持ftp、hdfs等传输\n"
                     "1.0-->挂载磁盘文件之间相互传输");
}

// TODO
void Widget::onGetLatestVersion() {
  QMessageBox::about(this, "获取新版本", "已经是最新版本了\n");
}

void Widget::onSaveConfiguration() {
  qDebug() << "保存配置文件";
  saveWorkspaceConfig();
}

// TODO
void Widget::onSystemInfo() { monitor->showSettingsDialog(); }

// TODO
void Widget::onRules() {
  qDebug() << "规则管理 -- 开发中，当前使用程序中的默认规则";
}

// 不记录日志
// void Widget::logMessageHandler(QtMsgType type,
//                                const QMessageLogContext& context,
//                                const QString& msg) {
//   Q_UNUSED(context);
//   QString logMessage =
//       QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz ");
//   switch (type) {
//     case QtDebugMsg:
//       logMessage += "[DEBUG] ";
//       break;
//     case QtInfoMsg:
//       logMessage += "[INFO] ";
//       break;
//     case QtWarningMsg:
//       logMessage += "[WARNING] ";
//       break;
//     case QtCriticalMsg:
//       logMessage += "[CRITICAL] ";
//       break;
//     case QtFatalMsg:
//       logMessage += "[FATAL] ";
//       break;
//   }
//   logMessage += msg;
//
//   if (globalLogArea) {
//     QMutexLocker locker(&logMutex);
//     globalLogArea->append(logMessage);
//   }
// }

// 记录日志
void Widget::logMessageHandler(QtMsgType type,
                               const QMessageLogContext& context,
                               const QString& msg) {
  Q_UNUSED(context);

  // 获取当前时间
  QString logMessage =
      QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz ");

  // 添加日志类型标签
  switch (type) {
    case QtDebugMsg:
      logMessage += "[DEBUG] ";
      break;
    case QtInfoMsg:
      logMessage += "[INFO] ";
      break;
    case QtWarningMsg:
      logMessage += "[WARNING] ";
      break;
    case QtCriticalMsg:
      logMessage += "[CRITICAL] ";
      break;
    case QtFatalMsg:
      logMessage += "[FATAL] ";
      break;
  }
  logMessage += msg;

  // **线程安全：写入 UI 日志区域**
  if (globalLogArea) {
    QMutexLocker locker(&logMutex);
    globalLogArea->append(logMessage);
  }

  // **日志写入文件**
  static QString logDir = QCoreApplication::applicationDirPath() + "/logs";
  static QString logFile = logDir + "/app.log";

  // **确保日志目录存在**
  QDir dir(logDir);
  if (!dir.exists()) {
    dir.mkpath(logDir);
  }

  // **日志写入文件**
  QFile file(logFile);
  if (file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
    QTextStream out(&file);
    out << logMessage << "\n";
    file.close();
  }
}

/**
 * @brief 检查文件是否有差异
 *
 * @param src 源路径
 * @param dest 目标路径
 * @return void
 ***/
// void Widget::checkFileVersion(QString src, QString dest) {}

// TODO
// void Widget::writeErrLog(QString msg) {
//  // 获取当前路径，保存日志到 err/task.log
// }

void Widget::onSyncActionClicked() const {
  // 若已有任务在运行，则不允许重复点击
  if (syncAction->isEnabled()) return;

  // 禁用按钮，防止重复点击
  syncAction->setEnabled(false);
  // logWidget->appendPlainText("开始同步任务...");
  qDebug() << "开始同步任务...onSyncActionClicked";
  // 立即触发同步任务（通过 TriggerMonitor 接口）
  // monitor->triggerSync();
}

void Widget::onTaskCompleted(const QString& source,
                             const QString& target) const {
  // logWidget->appendPlainText(QString("同步任务完成：%1 -> %2").arg(source,
  // target));
  qDebug() << QString("同步任务完成：%1 -> %2").arg(source, target);
  // 同步任务完成后重新启用同步按钮
  syncAction->setEnabled(true);
}

void Widget::onSyncTriggered(const QString& source, const QString& target) {
  // logWidget->appendPlainText(QString("已触发同步任务：%1 -> %2").arg(source,
  // target));
  qDebug() << QString("已触发同步任务：%1 -> %2").arg(source, target);
}

// 所有按键禁用？
void Widget::setSyncActionEnabled(bool status) const {
  syncAction->setEnabled(status);
  triggerTimeAction->setEnabled(status);
}
