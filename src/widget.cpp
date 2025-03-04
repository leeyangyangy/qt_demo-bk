#include "widget.h"

#include <qpointer.h>

// #include "task/FileMonitor.h"
#include <qprocess.h>

#include <QCheckBox>
#include <QHostInfo>
#include <QSettings>

#include "init/Env.h"
#include "task/SyncTask.h"
#include "utils/SyncUtils.h"
#include "utils/rule/RuleEditDialog.h"

// 定义静态成员变量
QTextEdit* Widget::globalLogArea = nullptr;
QMutex Widget::logMutex;

Widget::Widget(QWidget* parent)
    : QMainWindow(parent),
      ui(new Ui::Widget),
      logArea(nullptr),
      workspaceLayout(new QVBoxLayout()),
      configFilePath(QString("%1/%2/%3")
                         .arg(QCoreApplication::applicationDirPath())
                         .arg("etc")
                         .arg("files.json")),
      configSystemPath(QString("%1/%2/%3")
                           .arg(QCoreApplication::applicationDirPath())
                           .arg("etc")
                           .arg("etc.json")) {
  setWindowTitle(QString("%1%2%3").arg("-- 文件同步工具 x",
                                       std::to_string(sizeof(int*) * 8).data(),
                                       " -- V1.0.3.25-0303 制作：LEEYANGY"));

  setWindowIcon(QIcon(":/svg/files.png"));
  Env::instance().checkAndInit();

  // 在获取布局时候初始化监控进程
  monitor = new TriggerMonitor(this, this);
  monitor->startMonitoring(configFilePath);

  // 初始化 trayIcon
  trayIcon.reset(new QSystemTrayIcon(this));
  // 设置托盘图标
  trayIcon->setIcon(QIcon(":/png/filesync.png"));

  trayIcon->setToolTip("FilesSync");

  auto* trayMenu = new QMenu(this);

  trayMenu->addAction(QIcon(":/svg/files.svg"), "显示主窗口", this,
                      &Widget::showNormal);

  trayMenu->addAction(QIcon(":/svg/exit.svg"), "退出", this, &Widget::onExit);

  trayIcon->setContextMenu(trayMenu);
  trayIcon->setVisible(true);

  // 设置托盘图标
  trayIcon->setIcon(QIcon(":/png/filesync.png"));

  // 处理托盘图标的点击事件，恢复窗口
  connect(trayIcon.data(), &QSystemTrayIcon::activated,
          [&](const QSystemTrayIcon::ActivationReason reason) {
            this->showNormal();
          });

  centralWidget = new QWidget(this);
  mainLayout = new QVBoxLayout(centralWidget);

  setupMenuBar();
  setupWorkspace(mainLayout);
  setupLogArea(mainLayout);
  setupProgressArea(mainLayout);

  setCentralWidget(centralWidget);

  globalLogArea = logArea;
  qInstallMessageHandler(logMessageHandler);

  loadWorkspaceConfig();
  // 连接信号槽
  connect(this, &Widget::rulesUpdated, this, &Widget::reloadRules);
}

Widget::~Widget() {
  saveWorkspaceConfig();
  monitor->stopMonitoring();
  delete monitor;
  // 释放内存？
  delete ui;
  // delete trayIcon;
  // delete progressBar;
  // delete taskListWidget;
}

void Widget::closeEvent(QCloseEvent* event) {
  saveWorkspaceConfig();
  // QMainWindow::closeEvent(event); // 关闭
  this->hide();     // 仅最小化
  event->ignore();  // 阻止关闭
}

void Widget::setupMenuBar() {
  menuBar = new QMenuBar(this);
  // -------  配置 -------
  menus.push_back(std::move(std::make_unique<QMenu>("配置(C)", this)));
  menuShortcuts.push_back(std::make_unique<QShortcut>(QKeySequence("C"), this));
  // TODO 应该是显示对应菜单下的按钮
  connect(menuShortcuts[0].get(), &QShortcut::activated, this,
          &Widget::onNewProject);
  // QIcon(":/svg/new_conf.svg")
  confActions.push_back(
      std::make_unique<QAction>(QIcon(":/svg/new_conf.svg"), "新建配置", this));
  confActions[0]->setShortcut(QKeySequence("Ctrl+N"));
  connect(confActions[0].get(), &QAction::triggered, this,
          &Widget::onNewProject);

  confActions.push_back(std::make_unique<QAction>(QIcon(":/svg/open_conf.svg"),
                                                  "打开配置", this));
  confActions[1]->setShortcut(QKeySequence("Ctrl+O"));
  connect(confActions[1].get(), &QAction::triggered, this,
          &Widget::onOpenConfiguration);

  confActions.push_back(
      std::make_unique<QAction>(QIcon(":/svg/save.svg"), "保存配置", this));
  confActions[2]->setShortcut(QKeySequence("Ctrl+S"));
  connect(confActions[2].get(), &QAction::triggered, this,
          &Widget::onSaveConfiguration);

  confActions.push_back(
      std::make_unique<QAction>(QIcon(":/svg/rules.svg"), "规则引擎", this));
  confActions[3]->setShortcut(QKeySequence("Ctrl+R"));
  connect(confActions[3].get(), &QAction::triggered, this, &Widget::onRules);

  confActions.push_back(
      std::make_unique<QAction>(QIcon(":/svg/exit.svg"), "退出", this));
  confActions[4]->setShortcut(QKeySequence("Esc"));
  connect(confActions[4].get(), &QAction::triggered, this, &Widget::onExit);

  for (const auto& conf : confActions) menus[0]->addAction(conf.get());

  // -------  配置 -------

  // -------  同步 -------
  menus.push_back(std::move(std::make_unique<QMenu>("同步(S)", this)));
  menuShortcuts.push_back(std::make_unique<QShortcut>(QKeySequence("S"), this));
  // TODO 可能需要修改
  connect(menuShortcuts[1].get(), &QShortcut::activated, this, [this] {
    // menus[1]->popup(QCursor::pos());  // 在鼠标当前位置弹出菜单
    // 获取工具栏右下角的全局坐标
    const QPoint toolbarGlobalPos =
        menuBar->mapToGlobal(menuBar->rect().bottomRight());
    // 偏移 10 像素避免遮挡
    menus[1]->popup(toolbarGlobalPos);
  });

  syncActions.push_back(
      std::make_unique<QAction>(QIcon(":/svg/sync.svg"), "完整同步 -- 很慢(:", this));
  syncActions[0]->setShortcut(QKeySequence("Ctrl+Shift+S"));
  connect(syncActions[0].get(), &QAction::triggered, this, &Widget::onFullSync);

  // 点击按钮时启动同步任务
  connect(syncActions[0].get(), &QAction::triggered, this,
          &Widget::onSyncActionClicked);

  connect(monitor, &TriggerMonitor::syncTriggered, this,
          &Widget::onSyncTriggered);

  // 同步任务完成时恢复按钮状态
  connect(monitor, &TriggerMonitor::syncTriggered, this,
          &Widget::onTaskCompleted);

  syncActions.push_back(std::make_unique<QAction>(QIcon(":/svg/increase.svg"),
                                                  "增量同步 -- 很快:)", this));
  syncActions[1]->setShortcut(QKeySequence("Ctrl+Shift+I"));
  connect(syncActions[1].get(), &QAction::triggered, this,
          &Widget::onIncreaseSync);

  syncActions.push_back(std::make_unique<QAction>(QIcon(":/svg/hourglass.svg"),
                                                  "触发时间", this));
  syncActions[2]->setShortcut(QKeySequence("Ctrl+Shift+T"));

  connect(syncActions[2].get(), &QAction::triggered, this,
          &Widget::onSystemInfo);

  syncActions.push_back(
      std::make_unique<QAction>(QIcon(":/svg/pause.svg"), "停止监控", this));
  syncActions[3]->setShortcut(QKeySequence("Ctrl+Shift+P"));
  connect(syncActions[3].get(), &QAction::triggered, this,
          &Widget::onStopTriggerTimeActionClicked);

  syncActions.push_back(
      std::make_unique<QAction>(QIcon(":/svg/start.svg"), "开始监控", this));
  syncActions[4]->setShortcut(QKeySequence("Ctrl+Shift+R"));
  connect(syncActions[4].get(), &QAction::triggered, this,
          &Widget::onStartTriggerTimeActionClicked);

  for (const auto& conf : syncActions) menus[1]->addAction(conf.get());
  // -------  同步 -------

  // -------  帮助 -------
  menus.push_back(std::move(std::make_unique<QMenu>("帮助(H)", this)));

  menuShortcuts.push_back(std::make_unique<QShortcut>(QKeySequence("H"), this));
  connect(menuShortcuts[2].get(), &QShortcut::activated, this,
          &Widget::onAbout);

  helpActions.push_back(
      std::make_unique<QAction>(QIcon(":/svg/author.svg"), tr("&关于"), this));
  helpActions[0]->setShortcut(QKeySequence("Ctrl+A"));
  connect(helpActions[0].get(), &QAction::triggered, this, &Widget::onAbout);

  helpActions.push_back(std::make_unique<QAction>(QIcon(":/svg/update-log.svg"),
                                                  tr("&更新日志"), this));
  helpActions[1]->setShortcut(QKeySequence("Ctrl+U"));
  connect(helpActions[1].get(), &QAction::triggered, this,
          &Widget::onUpdateLog);

  helpActions.push_back(std::make_unique<QAction>(QIcon(":/svg/update.svg"),
                                                  tr("&获取新版本"), this));
  helpActions[2]->setShortcut(QKeySequence("Ctrl+G"));
  connect(helpActions[2].get(), &QAction::triggered, this,
          &Widget::onGetLatestVersion);

  for (auto& conf : helpActions) menus[2]->addAction(conf.get());
  // -------  帮助 -------

  // ------- 高级功能 -------
  menus.push_back(std::make_unique<QMenu>(tr("高级功能(D)")));
  menuShortcuts.push_back(std::make_unique<QShortcut>(QKeySequence("D"), this));
  connect(menuShortcuts[3].get(), &QShortcut::activated, this, [this] {
    // menus[1]->popup(QCursor::pos());  // 在鼠标当前位置弹出菜单
    // 获取工具栏右下角的全局坐标
    const QPoint toolbarGlobalPos =
        menuBar->mapToGlobal(menuBar->rect().bottomRight());
    // 偏移 10 像素避免遮挡
    menus[3]->popup(toolbarGlobalPos);
  });

  advanceActions.push_back(std::make_unique<QAction>(
      QIcon(":/svg/autoStartOn.svg"), tr("&开机自启"), this));
  advanceActions[0]->setShortcut(QKeySequence("Ctrl+D"));
  advanceActions[0]->setCheckable(true);  // 设置可选状态
  // advanceActions[0].get()->setChecked(isAutoStartEnabled());  // 初始状态
  // connect(advanceActions[0].get(), &QAction::, this, [this] {
  //   this->setAutoStart(isAutoStartEnabled());
  // });

  // 连接信号槽
  connect(advanceActions[0].get(), &QAction::toggled, this,
          &Widget::toggleAutoStart);

  toggleAutoStart(isAutoStartEnabled());

  // TODO 文件对比，文件历史追踪，对比最近五次？

  advanceActions.push_back(std::make_unique<QAction>(
      QIcon(":/svg/history.svg"), tr("&文件历史追踪"), this));
  advanceActions[1]->setShortcut(QKeySequence("Ctrl+Shift+H"));
  connect(advanceActions[1].get(), &QAction::triggered, this, [this] {
    QMessageBox::warning(this, "提示", "文件历史追踪，功能开发中！");
  });
  advanceActions.push_back(std::make_unique<QAction>(
      QIcon(":/svg/put_out_fires.svg"), tr("&清理缓存"), this));

  advanceActions[2]->setShortcut(QKeySequence("Ctrl+Shift+C"));
  connect(advanceActions[2].get(), &QAction::triggered, this, [this] {
    // 获取 logs 和 conflicts 文件夹路径
    QString appDirPath = QCoreApplication::applicationDirPath();
    QDir logsDir(QString("%1/%2").arg(appDirPath, "logs"));
    QDir conflictDir(QString("%1/%2").arg(appDirPath, "conflicts"));
    // performance.svg
    // qDebug() << "Cleaning logs and conflicts directories...";

    // 检查 conflicts 文件夹是否存在
    if (!conflictDir.exists()) {
      // qDebug() << "ℹ️ Conflicts directory does not exist:" <<
      // conflictDir.path();
      return;
    }

    // 遍历 conflicts 文件夹中的所有子文件夹
    QStringList conflictSubDirs =
        conflictDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& subDirName : conflictSubDirs) {
      QString conflictSubDirPath = conflictDir.filePath(subDirName);
      QString logsSubDirPath = logsDir.filePath(subDirName);

      QDir conflictSubDir(conflictSubDirPath);
      QDir logsSubDir(logsSubDirPath);

      // 检查冲突子文件夹是否为空
      QStringList conflictEntries = conflictSubDir.entryList(
          QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);
      if (conflictEntries.isEmpty()) {
        // qDebug() << "✅ Conflict subdirectory is empty, proceeding to clean
        // up:" << conflictSubDirPath;

        // 删除冲突子文件夹
        if (conflictSubDir.removeRecursively()) {
          // qDebug() << "✅ Successfully removed conflict subdirectory:" <<
          // conflictSubDirPath;
        } else {
          qDebug() << "❌ Failed to remove conflict subdirectory:"
                   << conflictSubDirPath;
        }

        // 删除对应的 logs 子文件夹
        if (logsSubDir.exists()) {
          if (logsSubDir.removeRecursively()) {
            // qDebug() << "✅ Successfully removed logs subdirectory:" <<
            // logsSubDirPath;
          } else {
            qDebug() << "❌ Failed to remove logs subdirectory:"
                     << logsSubDirPath;
          }
        } else {
          qDebug() << "ℹ️ Logs subdirectory does not exist:" << logsSubDirPath;
        }
      } else {
        // qDebug() << "⚠️ Conflict subdirectory is not empty, skipping cleanup:"
        // << conflictSubDirPath; qDebug() << "Contents in conflict
        // subdirectory:" << conflictEntries;
      }
    }

    // 检查 logs 和 conflicts 文件夹是否为空，如果为空则删除
    if (logsDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)
            .isEmpty()) {
      if (logsDir.removeRecursively()) {
        // qDebug() << "✅ Successfully removed empty logs directory:" <<
        // logsDir.path();
      } else {
        qDebug() << "❌ Failed to remove empty logs directory:"
                 << logsDir.path();
      }
    }

    if (conflictDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)
            .isEmpty()) {
      if (conflictDir.removeRecursively()) {
        // qDebug() << "✅ Successfully removed empty conflicts directory:" <<
        // conflictDir.path();
      } else {
        qDebug() << "❌ Failed to remove empty conflicts directory:"
                 << conflictDir.path();
      }
    }
  });

  for (const auto& action : advanceActions) menus[3]->addAction(action.get());

  // ------- 高级功能 -------

  // 将所有菜单添加到菜单栏
  for (const auto& menu : menus) menuBar->addMenu(menu.get());
  menuBar->addSeparator();
  setMenuBar(menuBar);
}

void Widget::setupWorkspace(QVBoxLayout* mainLayout) {
  workspaceGroup = new QGroupBox("工作空间", this);
  workspaceLayout = new QVBoxLayout(workspaceGroup);

  scrollArea = new QScrollArea(this);
  scrollContent = new QWidget(this);
  scrollContent->setLayout(workspaceLayout);
  scrollArea->setWidget(scrollContent);
  scrollArea->setWidgetResizable(true);

  constexpr int fixedHeight = 200;
  scrollArea->setFixedHeight(fixedHeight);
  buttons.push_back(std::make_unique<QPushButton>("添加新行", this));
  connect(buttons[0].get(), &QPushButton::clicked, this,
          [this] { addFileRow(); });

  groupLayout = new QVBoxLayout(workspaceGroup);
  groupLayout->addWidget(scrollArea);
  groupLayout->addWidget(buttons[0].get());

  mainLayout->addWidget(workspaceGroup);
}

void Widget::setupLogArea(QVBoxLayout* mainLayout) {
  labels.push_back(std::make_unique<QLabel>("日志", this));

  logArea = new QTextEdit(this);
  logArea->setReadOnly(true);

  buttons.push_back(std::make_unique<QPushButton>("清空日志", this));
  connect(buttons[1].get(), &QPushButton::clicked, logArea, &QTextEdit::clear);

  logControlLayout = new QHBoxLayout();
  logControlLayout->addWidget(labels[0].get());
  logControlLayout->addStretch();
  logControlLayout->addWidget(buttons[1].get());

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
  labels.push_back(std::move(std::make_unique<QLabel>("任务进度", this)));

  progressBar.reset(new QProgressBar());  // 重新初始化
  progressBar->setRange(0, 100);          // 进度条范围 0-100
  progressBar->setValue(0);               // 初始值为 0
  mainLayout->addWidget(labels[1].get());
  mainLayout->addWidget(progressBar.get());  // 添加到布局

  // 存储进度条指针，方便后续更新
  // this->progressBar = progressBar;
}

/**
 * @brief 更新进度条
 *
 * @param progress 进度条值
 * @return void
 */
void Widget::updateProgressBar(const int progress) const {
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
  auto* rowLayout = new QHBoxLayout();

  // 使用 QPointer 管理控件指针
  QPointer fileDialogButton1 = new QPushButton("选择监听文件夹", this);
  QPointer fileInfo1 = new QLineEdit(this);
  fileInfo1->setReadOnly(true);

  QPointer fileDialogButton2 = new QPushButton("选择同步文件夹", this);
  QPointer fileInfo2 = new QLineEdit(this);
  fileInfo2->setReadOnly(true);

  QPointer deleteButton = new QPushButton("删除", this);

  // 连接信号槽
  connect(fileDialogButton1, &QPushButton::clicked, this,
          [fileInfo1, fileInfo2, this] {
            const QString filePath = QFileDialog::getExistingDirectory(
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
          [fileInfo1, fileInfo2, this] {
            const QString filePath = QFileDialog::getExistingDirectory(
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

  // connect(deleteButton, &QPushButton::clicked, this,
  //         [this, rowLayout, fileInfo1, fileInfo2, fileDialogButton1,
  //          fileDialogButton2, deleteButton] {
  //           // 从布局中移除控件
  //           rowLayout->removeWidget(fileDialogButton1);
  //           rowLayout->removeWidget(fileInfo1);
  //           rowLayout->removeWidget(fileDialogButton2);
  //           rowLayout->removeWidget(fileInfo2);
  //           rowLayout->removeWidget(deleteButton);
  //
  //           // 删除控件
  //           fileDialogButton1->deleteLater();
  //           fileInfo1->deleteLater();
  //           fileDialogButton2->deleteLater();
  //           fileInfo2->deleteLater();
  //           deleteButton->deleteLater();
  //
  //           // 删除布局
  //           workspaceLayout->removeItem(rowLayout);
  //           rowLayout->deleteLater();
  //           delWorkSpaceDB(
  //               SyncUtils::computeXXHash(fileInfo2->text().split("/").last()));
  //         });

  connect(deleteButton, &QPushButton::clicked, this,
          [this, rowLayout, fileInfo1, fileInfo2, fileDialogButton1,
           fileDialogButton2, deleteButton] {
            // 检查是否需要弹出提示
            QSettings settings("leeyangy", "FilesSync");
            const bool skipPrompt =
                settings.value("skipDeletePrompt", false).toBool();

            if (!skipPrompt) {
              // 创建确认对话框
              QMessageBox confirmBox;
              confirmBox.setIcon(QMessageBox::Warning);
              confirmBox.setWindowTitle(tr("确认删除"));
              confirmBox.setText(
                  tr("此操作将永久删除该条目，无法恢复。是否继续？"));

              // 添加复选框
              QCheckBox dontShowAgainCheckBox(tr("下次不再提示"), &confirmBox);
              confirmBox.setCheckBox(&dontShowAgainCheckBox);

              // 添加按钮
              confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
              confirmBox.setDefaultButton(QMessageBox::No);

              // 显示对话框并获取用户选择
              if (const int result = confirmBox.exec();
                  result == QMessageBox::No) {
                return;  // 用户取消操作
              }

              // 保存用户选择
              if (dontShowAgainCheckBox.isChecked()) {
                settings.setValue("skipDeletePrompt", true);
              }
            }

            // 执行删除操作
            rowLayout->removeWidget(fileDialogButton1);
            rowLayout->removeWidget(fileInfo1);
            rowLayout->removeWidget(fileDialogButton2);
            rowLayout->removeWidget(fileInfo2);
            rowLayout->removeWidget(deleteButton);

            fileDialogButton1->deleteLater();
            fileInfo1->deleteLater();
            fileDialogButton2->deleteLater();
            fileInfo2->deleteLater();
            deleteButton->deleteLater();

            workspaceLayout->removeItem(rowLayout);
            rowLayout->deleteLater();

            delWorkSpaceDB(
                SyncUtils::computeXXHash(fileInfo2->text().split("/").last()));
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

  const QByteArray data = configFile.readAll();
  configFile.close();

  const QJsonDocument doc = QJsonDocument::fromJson(data);
  if (!doc.isObject()) {
    qWarning() << "配置文件格式错误，创建默认配置";
    saveWorkspaceConfig();
    return;
  }

  QJsonArray rows = doc.object().value("workspace").toArray();
  for (const auto& value : rows) {
    QJsonObject row = value.toObject();

    QString file1 = row.value("file1").toString();
    QString file2 = row.value("file2").toString();

    addFileRow();
    QLayoutItem* item = workspaceLayout->itemAt(workspaceLayout->count() - 1);
    QHBoxLayout* rowLayout =
        item ? qobject_cast<QHBoxLayout*>(item->layout()) : nullptr;
    if (rowLayout) {
      auto* fileInfo1 =
          qobject_cast<QLineEdit*>(rowLayout->itemAt(1)->widget());
      auto* fileInfo2 =
          qobject_cast<QLineEdit*>(rowLayout->itemAt(3)->widget());
      if (fileInfo1 && fileInfo2) {
        fileInfo1->setText(file1);
        fileInfo2->setText(file2);
      }
    }
  }
}

void Widget::saveWorkspaceConfig() const {
  QJsonArray rows;
  for (int i = 0; i < workspaceLayout->count(); ++i) {
    QLayoutItem* item = workspaceLayout->itemAt(i);
    const QHBoxLayout* rowLayout =
        item ? qobject_cast<QHBoxLayout*>(item->layout()) : nullptr;
    if (!rowLayout) continue;

    const auto* fileInfo1 =
        qobject_cast<QLineEdit*>(rowLayout->itemAt(1)->widget());
    const auto* fileInfo2 =
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

  const QJsonDocument doc(config);
  QFile configFile(configFilePath);
  if (!configFile.open(QIODevice::WriteOnly)) {
    qWarning() << "无法写入配置文件";
    return;
  }

  configFile.write(doc.toJson());
  configFile.close();
}

void Widget::clearWorkspaceConfig() const {
  if (!workspaceLayout) {
    qDebug() << "workspaceLayout 为空，无需清理";
    return;
  }

  // 遍历 workspaceLayout
  while (workspaceLayout->count() > 0) {
    QLayoutItem* item = workspaceLayout->takeAt(0);
    if (!item) continue;

    auto* rowLayout = qobject_cast<QHBoxLayout*>(item->layout());
    if (!rowLayout) {
      delete item;
      continue;
    }

    // 遍历 QHBoxLayout 并删除控件
    while (rowLayout->count() > 0) {
      QLayoutItem* rowItem = rowLayout->takeAt(0);
      if (!rowItem) continue;

      if (QWidget* widget = rowItem->widget()) {
        widget->hide();
        widget->deleteLater();
      }

      delete rowItem;
    }

    // 删除行布局
    delete rowLayout;
  }
}

void Widget::onNewProject() const { clearWorkspaceConfig(); }

void Widget::onOpenConfiguration() {
  qInfo() << "加载配置...";

  const QString selectedFilePath = QFileDialog::getOpenFileName(
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

void Widget::onExit() {
  // TODO 任务进行中，原则上不允许退出程序，syncAction，后续需要修改
  if (!syncActions[0].get()->isEnabled()) {
    QMessageBox::warning(this, "警告", "任务进行中，不允许退出程序！");
  } else {
    QApplication::quit();
  }
}

void Widget::onAbout() {
  QDialog dialog(this);
  dialog.setWindowTitle(QStringLiteral("关于文件同步管理器"));
  dialog.setFixedSize(800, 480);

  // 使用 unique_ptr 管理布局，后续通过 setLayout 转移所有权
  auto layout = std::make_unique<QVBoxLayout>();
  layout->setSpacing(20);
  layout->addStretch();

  // Lambda 返回 unique_ptr 以自动管理临时对象
  auto createLabel = [&dialog](const QString& text, bool isLink = false) {
    auto label = std::make_unique<QLabel>(&dialog);  // 直接设置父对象
    label->setText(text);
    if (isLink) {
      label->setTextInteractionFlags(Qt::TextSelectableByMouse |
                                     Qt::LinksAccessibleByMouse);
      label->setOpenExternalLinks(true);
    }
    return label;
  };

  // 创建标签并转移所有权到父对象 dialog
  auto softwareInfo =
      createLabel(QStringLiteral("软件简介：基于C++&Qt开发的文件同步管理器。"));
  layout->addWidget(softwareInfo.release());  // release() 转移所有权到布局

  auto author = createLabel(QStringLiteral("作者：LEEYANGY"));
  layout->addWidget(author.release());

  auto link =
      createLabel(QStringLiteral("<a "
                                 "href=\"https://github.com/leeyangyangy/"
                                 "qt_demo-bk/tree/filesync-cmake-clion\">"
                                 "https://github.com/leeyangyangy/qt_demo-bk/"
                                 "tree/filesync-cmake-clion</a>"),
                  true);
  layout->addWidget(link.release());

  // 分隔线
  auto sepLine = std::make_unique<QFrame>(&dialog);
  sepLine->setFrameShape(QFrame::HLine);
  sepLine->setFrameShadow(QFrame::Sunken);
  // 获取 link 的索引需在 release 前操作（此处因已 release 需调整逻辑）
  const int linkIndex = layout->count() - 1;  // 假设 link 是最后添加的控件
  layout->insertWidget(linkIndex + 1, sepLine.release());

  // 确定按钮
  auto okButton =
      std::make_unique<QPushButton>(QStringLiteral("确定"), &dialog);
  connect(okButton.get(), &QPushButton::clicked, &dialog, &QDialog::accept);
  layout->addWidget(okButton.release(), 0, Qt::AlignBottom | Qt::AlignCenter);

  layout->addStretch();

  // 将布局所有权转移给 dialog
  dialog.setLayout(layout.release());
  dialog.exec();
}

void Widget::onFullSync() const { monitor->triggerSync(configFilePath); }
void Widget::onIncreaseSync() const { qDebug() << "onIncreaseSync"; }

// TODO
void Widget::onUpdateLog() {
  QMessageBox::about(
      this, "更新日志",
      "开发计划--> 1.2 支持局域网文件互传\n"
      "当前版本-->1.0.3\n"
      "当前开发进度-->优化代码逻辑，添加持hdfs等传输?\n"
      "1.0.2-->"
      "优化文件校验过程，提升软件响应速度，修复开机自启读取配置路径错误\n"
      "1.0.1-->优化可能导致内存泄漏代码\n"
      "1.0-->挂载磁盘文件之间相互传输");
}

// TODO
void Widget::onGetLatestVersion() {
  QMessageBox::about(this, "获取新版本", "已经是最新版本了\n");
}

void Widget::onSaveConfiguration() const {
  qDebug() << "保存配置文件";
  saveWorkspaceConfig();
}

void Widget::onSystemInfo() const { monitor->showSettingsDialog(); }

/**
 * @brief 获取存储路径
 * @return QString
 */
QString Widget::getRuleFilePath() const {
  return QString("%1/%2/%3")
      .arg(QCoreApplication::applicationDirPath(), "etc", "FileSyncRules.conf");
}

/**
 * @brief 创建默认规则
 */
void Widget::createDefaultRules() const {
  const QString defaultContent =
      "[FileSync Rules v1.0.3]\n"
      "# 本机标识名称(如xxx电脑-x)\n"
      "machine_name = " +
      QHostInfo::localHostName() +
      " \n\n"
      "# 排除目录(全局共享同一规则)\n"
      "exclude_dirs = /temp/, /backup/\n\n"
      "# 排除文件类型(全局共享同一规则)\n"
      "exclude_exts = *.log, *.tmp\n\n"
      "# 同步间隔（分钟）{目前不可用}\n"
      "# sync_interval = 30\n\n"
      "# 最大历史版本数 {目前不可用}\n"
      "# max_history = 5";

  QFile file(getRuleFilePath());
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream(&file) << defaultContent;
    file.close();
  }
}

void Widget::onRules() {
  // 确保文件存在
  const QString rulePath = getRuleFilePath();
  if (!QFile::exists(rulePath)) {
    createDefaultRules();
    QMessageBox::information(this, tr("测试功能 部分能用"),
                             tr("已创建默认规则文件：\n") + rulePath);
  }

  // 读取内容
  QFile file(rulePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QMessageBox::critical(this, tr("错误"), tr("无法读取规则文件"));
    return;
  }
  const QString content = QTextStream(&file).readAll();
  file.close();

  // 显示编辑对话框
  if (RuleEditDialog dialog(content, this);
      dialog.exec() == QDialog::Accepted) {
    // 保存修改
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream(&file) << dialog.getEditedContent();
      file.close();
      QMessageBox::information(this, tr("成功"), tr("规则已保存"));

      // 触发规则重载
      emit rulesUpdated();
    } else {
      QMessageBox::critical(this, tr("错误"), tr("无法保存规则文件"));
    }
  }
}

void Widget::reloadRules() {
  qDebug() << "重新加载规则配置...";
  // TODO
  // qApp->exit(777);
  // qApp->quit();   // 或者   aApp->closeAllWindows();
  //
  // QProcess::startDetached(qApp->applicationFilePath(), QStringList());
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

  static QString logDir =
      QString("%1/%2").arg(QCoreApplication::applicationDirPath(), "logs");
  static QString logFile = logDir + "/app.log";

  // **确保日志目录存在**
  if (QDir dir(logDir); !dir.exists()) {
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
void Widget::checkFileVersion(const QString& src, const QString& dest) const {
  // TODO 提供界面供向用户直接发起同步检查要求
}

void Widget::onSyncActionClicked() const {
  // 若已有任务在运行，则不允许重复点击
  if (syncActions[0].get()->isEnabled()) return;

  // 禁用按钮，防止重复点击
  syncActions[0].get()->setEnabled(false);
  // logWidget->appendPlainText("开始同步任务...");
  // qDebug() << "开始同步任务...onSyncActionClicked";
  // 立即触发同步任务（通过 TriggerMonitor 接口）
  // monitor->triggerSync();
}

void Widget::onTaskCompleted(const QString& source,
                             const QString& target) const {
  // logWidget->appendPlainText(QString("同步任务完成：%1 -> %2").arg(source,
  // target));
  qDebug() << QString("同步任务完成：%1 -> %2").arg(source, target);
  // 同步任务完成后重新启用同步按钮
  syncActions[0].get()->setEnabled(true);
}

void Widget::onSyncTriggered(const QString& source, const QString& target) {
  // logWidget->appendPlainText(QString("已触发同步任务：%1 -> %2").arg(source,
  // target));
  qDebug() << QString("已触发同步任务：%1 -> %2").arg(source, target);
}

// 所有按键禁用？
void Widget::setSyncActionEnabled(const bool status) const {
  for (auto& conf : confActions) conf->setEnabled(status);
  for (auto& syncAction : syncActions) syncAction->setEnabled(status);
  // buttons[0].get()->setEnabled(status);
  // TODO 工作空间的按键也需要禁用？
}

void Widget::onStopTriggerTimeActionClicked() const {
  monitor->stopMonitoring();
}

void Widget::onStartTriggerTimeActionClicked() const {
  monitor->startMonitoring(configFilePath);
}

void Widget::toggleAutoStart(const bool checked) {
  qDebug() << "开机自启状态: " << checked;
  setAutoStart(checked);

  // 验证实际状态是否生效
  if (const bool actualState = isAutoStartEnabled(); actualState != checked) {
    QMessageBox::warning(this, tr("设置失败"),
                         tr("无法修改启动项，请检查程序权限"));
    advanceActions[0].get()->setChecked(actualState);  // 恢复正确状态
  }
  // 更新菜单项文本
  advanceActions[0].get()->setText(checked ? tr("✔ 开机自启动")
                                           : tr("× 开机自启动"));
}

// ======================= 平台相关实现 ========================
bool Widget::isAutoStartEnabled() const {
#ifdef Q_OS_WIN
  const QSettings settings(
      "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
      QSettings::NativeFormat);
  return settings.contains(winAppKey);
#elif defined(Q_OS_LINUX)
  return QFile::exists(
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      "/autostart/myapp.desktop");
#endif
  return false;
}

void Widget::setAutoStart(bool enable) const {
#ifdef Q_OS_WIN
  QSettings settings(
      "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
      QSettings::NativeFormat);
  if (enable) {
    const QString appPath =
        QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    settings.setValue(winAppKey, "\"" + appPath + "\"");  // 处理路径空格
  } else {
    settings.remove(winAppKey);
  }
#elif defined(Q_OS_LINUX
  QString desktopPath =
      QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) +
      "/autostart/myapp.desktop";
  if (enable) {
    QFile file(desktopPath);
    if (file.open(QIODevice::WriteOnly)) {
      QTextStream stream(&file);
      stream << "[Desktop Entry]\n"
             << "Type=Application\n"
             << "Name=MyApp\n"
             << "Exec=" << QCoreApplication::applicationFilePath() << "\n";
    }
  } else {
    QFile::remove(desktopPath);
  }
#endif
}

void Widget::delWorkSpaceDB(const QString& dbPath) const {
  if (QDir dbDir =
          QString("%1/%2/%3")
              .arg(QCoreApplication::applicationDirPath(), "db", dbPath);
      dbDir.exists()) {
    dbDir.removeRecursively();
  }
}
