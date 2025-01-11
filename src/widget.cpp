// #include "widget.h"
//
// // 定义静态成员变量
// QTextEdit* Widget::globalLogArea = nullptr;
// QMutex Widget::logMutex;
//
// Widget::Widget(QWidget* parent) :
//     QMainWindow(parent), ui(new Ui::Widget), logArea(nullptr)
// {
//     // setupUI();
//     ui->setupUi(this);
//     setWindowTitle("-- 文件同步管理器 -- 测试版 V1.0 24-1231 制作：LEEYANGY");
//
//     QWidget* centralWidget = new QWidget(this);
//     QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
//
//     // 设置菜单栏
//     setupMenuBar();
//
//     // 设置工作空间
//     setupWorkspace(mainLayout);
//
//     // 设置日志区域
//     setupLogArea(mainLayout);
//
//     setCentralWidget(centralWidget);
//
//     // 设置日志处理器
//     globalLogArea = logArea; // 将实例的 logArea 指针赋值给静态指针
//     qInstallMessageHandler(logMessageHandler); // 安装日志消息处理器
// }
//
// void Widget::setupMenuBar()
// {
//     QMenuBar* menuBar = new QMenuBar(this);
//
//     QMenu* fileMenu = new QMenu("文件", this);
//     fileMenu->addAction("新建", this, &Widget::onNewProject);
//     fileMenu->addAction("打开", this, &Widget::onOpenProject);
//     fileMenu->addAction("退出", this, &Widget::onExit);
//     menuBar->addMenu(fileMenu);
//
//     QMenu* helpMenu = new QMenu("帮助", this);
//     helpMenu->addAction("关于", this, &Widget::onAbout);
//     menuBar->addMenu(helpMenu);
//
//     setMenuBar(menuBar);
// }
//
// void Widget::setupWorkspace(QVBoxLayout* mainLayout)
// {
//     QGroupBox* workSpaceGroup = new QGroupBox("工作空间", this);
//     QVBoxLayout* workSpaceLayout = new QVBoxLayout(workSpaceGroup);
//
//     QWidget* scrollAreaContent = new QWidget(this);
//     QVBoxLayout* scrollLayout = new QVBoxLayout(scrollAreaContent);
//
//     QScrollArea* scrollArea = new QScrollArea(this);
//     scrollArea->setWidget(scrollAreaContent);
//     scrollArea->setWidgetResizable(true);
//
//     workSpaceLayout->addWidget(scrollArea);
//
//     // 添加初始文件信息行
//     addFileRow(scrollLayout);
//
//     // 添加按钮
//     QPushButton* addButton = new QPushButton("+ 添加新行", this);
//     connect(addButton, &QPushButton::clicked, [=]()
//     {
//         addFileRow(scrollLayout);
//     });
//     workSpaceLayout->addWidget(addButton);
//
//     mainLayout->addWidget(workSpaceGroup);
// }
//
// void Widget::setupLogArea(QVBoxLayout* mainLayout)
// {
//     // QLabel* logLabel = new QLabel("日志", this);
//     // QTextEdit* logArea = new QTextEdit(this);
//     // logArea->setReadOnly(true);
//     //
//     // QPushButton* clearLogButton = new QPushButton("清空日志", this);
//     // connect(clearLogButton, &QPushButton::clicked, [=]()
//     // {
//     //     logArea->clear();
//     // });
//     //
//     // QHBoxLayout* logControlLayout = new QHBoxLayout();
//     // logControlLayout->addWidget(logLabel);
//     // logControlLayout->addStretch();
//     // logControlLayout->addWidget(clearLogButton);
//     //
//     // mainLayout->addLayout(logControlLayout);
//     // mainLayout->addWidget(logArea);
//
//     QLabel* logLabel = new QLabel("日志", this);
//     logArea = new QTextEdit(this);
//     logArea->setReadOnly(true);
//
//     // 清空日志按钮
//     QPushButton* clearLogButton = new QPushButton("清空日志", this);
//     connect(clearLogButton, &QPushButton::clicked, [=]()
//     {
//         logArea->clear();
//     });
//
//     // 保存日志按钮
//     QPushButton* saveLogButton = new QPushButton("保存日志", this);
//     connect(saveLogButton, &QPushButton::clicked, [=]()
//     {
//         QString filePath = QFileDialog::getSaveFileName(this, "保存日志", "log.txt", "文本文件 (*.txt)");
//         if (!filePath.isEmpty()) {
//             QFile logFile(filePath);
//             if (logFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
//                 QTextStream out(&logFile);
//                 out << logArea->toPlainText();
//                 logFile.close();
//                 QMessageBox::information(this, "日志保存", "日志已保存到 " + filePath);
//             } else {
//                 QMessageBox::warning(this, "错误", "无法保存日志文件！");
//             }
//         }
//     });
//
//     // 搜索日志功能
//     QLineEdit* searchField = new QLineEdit(this);
//     searchField->setPlaceholderText("搜索日志...");
//     connect(searchField, &QLineEdit::textChanged, [=](const QString& searchText)
//     {
//         QTextCursor cursor = logArea->textCursor();
//         logArea->moveCursor(QTextCursor::Start);
//         QTextCharFormat highlightFormat;
//         highlightFormat.setBackground(Qt::yellow);
//
//         QTextCursor highlightCursor(logArea->document());
//         while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {
//             highlightCursor = logArea->document()->find(searchText, highlightCursor);
//             if (!highlightCursor.isNull()) {
//                 highlightCursor.mergeCharFormat(highlightFormat);
//             }
//         }
//     });
//
//     // 布局调整
//     QHBoxLayout* logControlLayout = new QHBoxLayout();
//     logControlLayout->addWidget(logLabel);
//     logControlLayout->addWidget(searchField);
//     logControlLayout->addStretch();
//     logControlLayout->addWidget(saveLogButton);
//     logControlLayout->addWidget(clearLogButton);
//
//     mainLayout->addLayout(logControlLayout);
//     mainLayout->addWidget(logArea);
//
// }
//
// void Widget::addFileRow(QVBoxLayout* layout)
// {
//     QHBoxLayout* fileRowLayout = new QHBoxLayout();
//     QLineEdit* fileInfo1 = new QLineEdit("文件信息1", this);
//     QLineEdit* fileInfo2 = new QLineEdit("文件信息2", this);
//
//     QPushButton* deleteButton = new QPushButton("删除", this);
//     connect(deleteButton, &QPushButton::clicked, [=]()
//     {
//         // 删除该行
//         fileInfo1->deleteLater();
//         fileInfo2->deleteLater();
//         deleteButton->deleteLater();
//         fileRowLayout->deleteLater();
//     });
//
//     fileRowLayout->addWidget(fileInfo1);
//     fileRowLayout->addWidget(fileInfo2);
//     fileRowLayout->addWidget(deleteButton);
//
//     layout->addLayout(fileRowLayout);
// }
//
// void Widget::onNewProject()
// {
//     qDebug("新建项目");
// }
//
// void Widget::onOpenProject()
// {
//     qDebug("打开项目");
// }
//
// void Widget::onExit()
// {
//     QApplication::quit();
// }
//
// void Widget::onAbout()
// {
//     QMessageBox::about(this, "关于文件同步管理器", "这是一个简单的文件同步管理器，使用 Qt 制作。\n作者：LEEYANGY");
// }
//
// // 自定义日志处理器
// void Widget::logMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
// {
//     Q_UNUSED(context)
//     qDebug() << "Custom log handler invoked"; // 确认是否触发了自定义处理器
//     QString logMessage = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz ");
//     qDebug() << "Log message: " << logMessage;
//     qDebug() << "Log type: " << type;
//     switch (type)
//     {
//     case QtDebugMsg:
//         logMessage += "[DEBUG] ";
//         break;
//     case QtInfoMsg:
//         logMessage += "[INFO] ";
//         break;
//     case QtWarningMsg:
//         logMessage += "[WARNING] ";
//         break;
//     case QtCriticalMsg:
//         logMessage += "[CRITICAL] ";
//         break;
//     case QtFatalMsg:
//         logMessage += "[FATAL] ";
//         break;
//     }
//     logMessage += msg;
//
//     // 输出到日志窗口
//     if (globalLogArea)
//     {
//         QMutexLocker locker(&logMutex);
//         globalLogArea->append(logMessage);
//     }
// }
//
// Widget::~Widget()
// {
//     delete ui;
// }
#include "widget.h"

#include "System.h"
#include "task/FileMonitor.h"
#include "task/SyncTask.h"

// 定义静态成员变量
QTextEdit* Widget::globalLogArea = nullptr;
QMutex Widget::logMutex;

// bool nativeEvent(const QByteArray &eventType, void *message, long *result) override {
//     MSG* msg = reinterpret_cast<MSG*>(message);
//     if (msg->message == WM_HOTKEY) {
//         switch (msg->wParam) {
//         case 1: // Ctrl + Left
//             previousTrack();
//             break;
//         case 2: // Ctrl + Right
//             nextTrack();
//             break;
//         case 3: // Ctrl + Space
//             playPauseTrack();
//             break;
//         }
//         return true; // 表示事件已处理
//     }
//     return QWidget::nativeEvent(eventType, message, result); // 传递给基类处理
// }

Widget::Widget(QWidget* parent)
    : QMainWindow(parent),
      workspaceLayout(new QVBoxLayout()),
      logArea(nullptr),
      configFilePath("files.json"),
      configSystemPath("etc.json"),
      ui(new Ui::Widget)
{
    setWindowTitle("-- 文件同步管理器 -- 测试版 V1.0 24-1231 制作：LEEYANGY");
    setWindowIcon(QIcon(":/svg/files.png"));

    QSystemTrayIcon trayIcon;
    QMenu *menu = new QMenu();
    QAction *exitAction = menu->addAction("Exit");
    QObject::connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    trayIcon.setContextMenu(menu);
    trayIcon.setVisible(true);

    // 创建系统托盘图标
    trayIcon.setIcon(QIcon(":/png/filesync.png"));

    // 处理托盘图标的点击事件
    QObject::connect(&trayIcon, &QSystemTrayIcon::activated, [&](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            QMessageBox::information(nullptr, "Tray Icon Clicked", "You clicked the tray icon.");
        }
    });






    QWidget* centralWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);

    setupMenuBar();
    setupWorkspace(mainLayout);
    setupLogArea(mainLayout);

    setCentralWidget(centralWidget);

    globalLogArea = logArea;
    qInstallMessageHandler(logMessageHandler);

    loadWorkspaceConfig();
}

Widget::~Widget()
{
    saveWorkspaceConfig();
}

void Widget::closeEvent(QCloseEvent* event)
{
    saveWorkspaceConfig();
    QMainWindow::closeEvent(event);
}

void Widget::setupMenuBar()
{
    QMenuBar* menuBar = new QMenuBar(this);

    QMenu* confMenu = new QMenu("配置(C)", this);
    // 设置快捷键，按下H键时显示帮助菜单
    QShortcut* confShortcut = new QShortcut(QKeySequence("C"), this);
    connect(confShortcut, &QShortcut::activated, confMenu, &QMenu::show);

    // TODO
    confMenu->addAction(QIcon(":/svg/new_conf.svg"), "新建配置", this, &Widget::onNewProject);
    confMenu->addAction(QIcon(":/svg/open_conf.svg"), "打开配置", this, &Widget::onOpenConfiguration);

    QAction* saveAction = confMenu->addAction(QIcon(":/svg/save.svg"), "保存配置", this, &Widget::onSaveConfiguration);
    saveAction->setShortcut(QKeySequence("Ctrl+S")); // 设置快捷键

    // 规则
    QAction* ruleAction = confMenu->addAction(QIcon(":/svg/rules.svg"), "规则引擎", this, &Widget::onRules);
    ruleAction->setShortcut(QKeySequence("Ctrl+Shift+R")); // 设置快捷键

    QAction* exitAction = confMenu->addAction(QIcon(":/svg/exit.svg"), "退出", this, &Widget::onExit);
    exitAction->setShortcut(QKeySequence("Esc")); // 设置快捷键

    menuBar->addMenu(confMenu);

    QMenu* syncMenu = new QMenu("同步(S)", this);
    QShortcut* syncShortcut = new QShortcut(QKeySequence("S"), this);
    connect(syncShortcut, &QShortcut::activated, syncMenu, &QMenu::show);
    QAction* syncAction = syncMenu->addAction(QIcon(":/svg/sync.svg"), "立即同步", this, &Widget::onSync);
    syncAction->setShortcut(QKeySequence("Ctrl+Shift+S")); // 设置快捷键
    syncMenu->addAction(syncAction);

    // 设置触发时间
    QAction* triggerTimeAction = syncMenu->
        addAction(QIcon(":/svg/hourglass.svg"), "触发时间", this, &Widget::onSystemInfo);
    triggerTimeAction->setShortcut(QKeySequence("Ctrl+Shift+T")); // 设置快捷键
    syncMenu->addAction(triggerTimeAction);
    menuBar->addMenu(syncMenu);

    QMenu* helpMenu = new QMenu("帮助(H)", this);
    // 设置快捷键，按下H键时显示帮助菜单
    QShortcut* helpShortcut = new QShortcut(QKeySequence("H"), this);
    connect(helpShortcut, &QShortcut::activated, helpMenu, &QMenu::show);

    QAction* aboutAction = new QAction(QIcon(":/svg/author.svg"), tr("&关于"), this);
    aboutAction->setShortcut(QKeySequence("Ctrl+A")); // 设置快捷键
    // 连接信号和槽
    connect(aboutAction, &QAction::triggered, this, &Widget::onAbout);
    helpMenu->addAction(aboutAction);

    // 更新日志
    QAction* updateLogAction = new QAction(QIcon(":/svg/update-log.svg"), tr("&更新日志"), this);
    updateLogAction->setShortcut(QKeySequence("Ctrl+U")); // 设置快捷键
    // 连接信号和槽
    connect(updateLogAction, &QAction::triggered, this, &Widget::onUpdateLog);
    helpMenu->addAction(updateLogAction);

    // 更新日志
    QAction* getatestVersionAction = new QAction(QIcon(":/svg/update.svg"), tr("&获取新版本"), this);
    getatestVersionAction->setShortcut(QKeySequence("Ctrl+G")); // 设置快捷键
    // 连接信号和槽
    connect(getatestVersionAction, &QAction::triggered, this, &Widget::onGetLatestVersion);
    helpMenu->addAction(getatestVersionAction);

    menuBar->addMenu(helpMenu);
    menuBar->addSeparator();
    setMenuBar(menuBar);
}

void Widget::setupWorkspace(QVBoxLayout* mainLayout)
{
    QGroupBox* workspaceGroup = new QGroupBox("工作空间", this);
    workspaceLayout = new QVBoxLayout(workspaceGroup);

    QScrollArea* scrollArea = new QScrollArea(this);
    QWidget* scrollContent = new QWidget(this);
    scrollContent->setLayout(workspaceLayout);
    scrollArea->setWidget(scrollContent);
    scrollArea->setWidgetResizable(true);

    // 设置固定高度
    int fixedHeight = 200; // 设置与日志区域相同的高度
    scrollArea->setFixedHeight(fixedHeight);

    // 设置滚动区域固定高度，使其刚好显示四行
    // int rowHeight = 40; // 每行控件高度
    // int visibleRows = 4; // 显示的行数
    // scrollArea->setFixedHeight(rowHeight * visibleRows + 10); // +10 是为了间距

    QPushButton* addButton = new QPushButton("+ 添加新行", this);
    connect(addButton, &QPushButton::clicked, this, [this]()
    {
        addFileRow();
    });

    QVBoxLayout* groupLayout = new QVBoxLayout(workspaceGroup);
    groupLayout->addWidget(scrollArea);
    groupLayout->addWidget(addButton);

    mainLayout->addWidget(workspaceGroup);

    // 添加任务进度控件
    // QLabel* progressLabel = new QLabel("任务进度", this);
    // progressBar = new QProgressBar(this);
    // progressBar->setRange(0, 100);
    // progressBar->setValue(0);
    //
    // // 添加任务列表控件
    // QLabel* taskLabel = new QLabel("任务列表", this);
    // taskListWidget = new QListWidget(this);
    //
    // mainLayout->addWidget(progressLabel);
    // mainLayout->addWidget(progressBar);
    // mainLayout->addWidget(taskLabel);
    // mainLayout->addWidget(taskListWidget);
    // mainLayout->addWidget(workspaceGroup);

}


void Widget::setupLogArea(QVBoxLayout* mainLayout)
{
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
 * @brief 比较路径是否移植
 * @param src 源路径
 * @param dest 目标路径
 * @return bool 布尔值
 */
bool Widget::checkDocCompare(QString src, QString dest)
{
    if (!src.compare(dest, Qt::CaseInsensitive))
    {
        qInfo("bool Widget::checkDocCompare 选择失败，路径相同");
        return true;
    }
    else
    {
        return false;
    }
}

void Widget::addFileRow()
{
    QHBoxLayout* rowLayout = new QHBoxLayout();

    QPushButton* fileDialogButton1 = new QPushButton("选择监听文件", this);
    QLineEdit* fileInfo1 = new QLineEdit(this);
    fileInfo1->setReadOnly(true);

    QPushButton* fileDialogButton2 = new QPushButton("选择同步文件夹", this);
    QLineEdit* fileInfo2 = new QLineEdit(this);
    fileInfo2->setReadOnly(true);

    connect(fileDialogButton1, &QPushButton::clicked, this, [fileInfo1, fileInfo2, this]()
    {
        // QString filePath = QFileDialog::getOpenFileName(nullptr, "选择文件1");
        QString filePath = QFileDialog::getExistingDirectory(nullptr, "选择监听原始文件夹");
        if (!filePath.isEmpty())
        {
            // qInfo("选择成功%d",666);
            qDebug() << "选择监听路径成功，路径：" << filePath;
            if (checkDocCompare(fileInfo2->text(), filePath))
            {
                QMessageBox::warning(this, "警告", "选择的监听文件夹不能与目标文件夹相同");
                fileInfo1->setText("");
                return;
            }
            fileInfo1->setText(filePath);
        }
        else
        {
            qWarning("选择失败 %s", "warning");
        }
    });

    connect(fileDialogButton2, &QPushButton::clicked, this, [fileInfo1, fileInfo2, this]()
    {
        // QString filePath = QFileDialog::getOpenFileName(nullptr, "选择文件2");
        QString filePath = QFileDialog::getExistingDirectory(nullptr, "选择目标路径文件夹");
        if (!filePath.isEmpty())
        {
            // 判断用户是否选择了相同文件夹
            if (checkDocCompare(fileInfo1->text(), filePath))
            {
                QMessageBox::warning(this, "警告", "选择的目标文件夹不能与监听文件夹相同");
                fileInfo2->setText("");
                return;
            }
            qDebug() << "选择目标路径成功，路径：" << filePath;
            fileInfo2->setText(filePath);
        }
    });

    QPushButton* deleteButton = new QPushButton("删除", this);
    connect(deleteButton, &QPushButton::clicked, this,
            [this, rowLayout, fileInfo1, fileInfo2, fileDialogButton1, fileDialogButton2, deleteButton]()
            {
                fileInfo1->deleteLater();
                fileInfo2->deleteLater();
                fileDialogButton1->deleteLater();
                fileDialogButton2->deleteLater();
                deleteButton->deleteLater();
                rowLayout->deleteLater();
            });

    rowLayout->addWidget(fileDialogButton1);
    rowLayout->addWidget(fileInfo1);
    rowLayout->addWidget(fileDialogButton2);
    rowLayout->addWidget(fileInfo2);
    rowLayout->addWidget(deleteButton);

    workspaceLayout->addLayout(rowLayout);
}

void Widget::loadWorkspaceConfig()
{
    QFile configFile(configFilePath);
    if (!configFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "无法打开配置文件，创建默认配置";
        saveWorkspaceConfig();
        return;
    }

    QByteArray data = configFile.readAll();
    configFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
    {
        qWarning() << "配置文件格式错误，创建默认配置";
        saveWorkspaceConfig();
        return;
    }

    QJsonArray rows = doc.object().value("workspace").toArray();
    for (const QJsonValue& value : rows)
    {
        QJsonObject row = value.toObject();

        QString file1 = row.value("file1").toString();
        QString file2 = row.value("file2").toString();

        addFileRow();
        QLayoutItem* item = workspaceLayout->itemAt(workspaceLayout->count() - 1);
        QHBoxLayout* rowLayout = item ? qobject_cast<QHBoxLayout*>(item->layout()) : nullptr;
        if (rowLayout)
        {
            QLineEdit* fileInfo1 = qobject_cast<QLineEdit*>(rowLayout->itemAt(1)->widget());
            QLineEdit* fileInfo2 = qobject_cast<QLineEdit*>(rowLayout->itemAt(3)->widget());
            if (fileInfo1 && fileInfo2)
            {
                fileInfo1->setText(file1);
                fileInfo2->setText(file2);
            }
        }
    }
}

void Widget::saveWorkspaceConfig()
{
    QJsonArray rows;
    for (int i = 0; i < workspaceLayout->count(); ++i)
    {
        QLayoutItem* item = workspaceLayout->itemAt(i);
        QHBoxLayout* rowLayout = item ? qobject_cast<QHBoxLayout*>(item->layout()) : nullptr;
        if (!rowLayout) continue;

        QLineEdit* fileInfo1 = qobject_cast<QLineEdit*>(rowLayout->itemAt(1)->widget());
        QLineEdit* fileInfo2 = qobject_cast<QLineEdit*>(rowLayout->itemAt(3)->widget());

        if (fileInfo1 && fileInfo2)
        {
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
    if (!configFile.open(QIODevice::WriteOnly))
    {
        qWarning() << "无法写入配置文件";
        return;
    }

    configFile.write(doc.toJson());
    configFile.close();
}

// TODO
void Widget::onNewProject()
{
    qDebug() << "新建项目";
}

// TODO
void Widget::onOpenConfiguration()
{
    qInfo() << "加载配置……";
    loadWorkspaceConfig();
    qInfo() << "加载配置完成……";
}

void Widget::onExit()
{
    QApplication::quit();
}

void Widget::onAbout()
{
    QMessageBox::about(this, "关于文件同步管理器", "基于Qt开发的文件同步管理器。\n作者：LEEYANGY");
}


void Widget::onSync()
{
    qInfo() << "开始执行任务";

    QFile configFile(configFilePath);
    if (!configFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "无法打开配置文件，创建默认配置";
        return;
    }

    QByteArray data = configFile.readAll();
    configFile.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject())
    {
        qWarning() << "配置文件格式错误，创建默认配置";
        return;
    }

    int taskCount = 0;        // 总任务数
    int completedTasks = 0;   // 已完成任务数

    QJsonArray rows = doc.object().value("workspace").toArray();
    for (const QJsonValue& value : rows)
    {
        QJsonObject row = value.toObject();

        QString file1 = row.value("file1").toString();
        QString file2 = row.value("file2").toString();

        if (checkDocCompare(file1, file2))
        {
            QMessageBox::about(this, "提示", "文件夹路径相同，请检查并修改");
            return;
        }
        if (file1.isEmpty() || file2.isEmpty())
        {
            QMessageBox::about(this, "提示", "文件夹选择有误，请检查并修改");
        }
        else
        {
            ++taskCount;

            // 创建任务
            SyncTask* task = new SyncTask(file1, file2);

            // 连接信号到主线程槽
            connect(task, &SyncTask::taskCompleted, this, [this, &completedTasks, taskCount](const QString& source, const QString& target) {
                completedTasks++;
                qInfo() << "同步完成:" << source << " -> " << target;
                qInfo() << "taskCount:" << taskCount << "completedTasks:" << completedTasks;

                if (completedTasks == taskCount)
                {
                    QMessageBox::about(this, "任务完成", "所有同步任务已完成！");
                } else
                {
                    // QMessageBox::about(this, "任务完成", "所有同步任务已完成！");

                }
            });

            QThreadPool::globalInstance()->start(task);
        }
    }
}

// void Widget::onSync()
// {
//     qInfo() << "开始执行任务";
//
//     QFile configFile(configFilePath);
//     if (!configFile.open(QIODevice::ReadOnly))
//     {
//         qWarning() << "无法打开配置文件，创建默认配置";
//         return;
//     }
//
//     QByteArray data = configFile.readAll();
//     configFile.close();
//
//     QJsonDocument doc = QJsonDocument::fromJson(data);
//     if (!doc.isObject())
//     {
//         qWarning() << "配置文件格式错误，创建默认配置";
//         return;
//     }
//
//     int count = 1;
//     QJsonArray rows = doc.object().value("workspace").toArray();
//
//     progressBar->setMaximum(rows.size());
//     progressBar->setValue(0);
//     taskListWidget->clear();
//
//     for (const QJsonValue& value : rows)
//     {
//         QJsonObject row = value.toObject();
//
//         QString file1 = row.value("file1").toString();
//         QString file2 = row.value("file2").toString();
//
//         if (file1.isEmpty() || file2.isEmpty())
//         {
//             qWarning() << "第" << count << "组文件夹路径有误，任务跳过";
//             taskListWidget->addItem("第 " + QString::number(count) + " 组任务失败：文件夹路径有误");
//             continue;
//         }
//
//         taskListWidget->addItem("第 " + QString::number(count) + " 组任务开始：正在同步...");
//         QListWidgetItem* item = taskListWidget->item(taskListWidget->count() - 1);
//
//         // 使用多线程执行同步任务
//         QtConcurrent::run([=]()
//         {
//             FileMonitor monitor;
//             monitor.addPath(file1);
//             monitor.syncDirectoryRecursively(file1, file2);
//
//             // 线程安全地更新任务进度
//             QMetaObject::invokeMethod(this, [=]()
//             {
//                 progressBar->setValue(progressBar->value() + 1);
//                 item->setText("第 " + QString::number(count) + " 组任务完成：同步成功");
//             });
//         });
//
//         ++count;
//     }
//
//     // 提示任务完成
//     connect(progressBar, &QProgressBar::valueChanged, this, [this, rows]()
//     {
//         if (progressBar->value() == rows.size())
//         {
//             QMessageBox::information(this, "提示", "所有任务已完成！");
//         }
//     });
// }


/**
 * @brief 检查文件是否有差异
 *
 * @param src 源路径
 * @param dest 目标路径
 * @return void
 ***/
void Widget::checkFileVersion(QString src, QString dest)
{
}

// TODO
void Widget::writeErrLog(QString msg)
{
    // 获取当前路径，保存日志到 err/task.log
}

// TODO
void Widget::onUpdateLog()
{
    QMessageBox::about(this, "更新日志",
                       "开发计划--> 1.2 支持局域网文件互传\n"
                       "当前开发进度-->即将支持ftp、hdfs等传输\n"
                       "1.0-->挂载磁盘文件之间相互传输");
}

// TODO
void Widget::onGetLatestVersion()
{
    QMessageBox::about(this, "获取新版本",
                       "已经是最新版本了\n");
}

// TODO
void Widget::onSaveConfiguration()
{
    qDebug() << "保存配置文件";
    saveWorkspaceConfig();
}

// TODO
void Widget::onSystemInfo()
{
    System system;
    system.setTriggerTime(QDateTime::currentDateTime().toString());

    // 序列化到 JSON
    QJsonObject jsonObj = system.toJson();
    qDebug() << "Serialized JSON:" << QJsonDocument(jsonObj).toJson();

    // 反序列化
    System newSystem;
    newSystem.fromJson(jsonObj);

    // 选择触发时间
    system.chooseTriggerTime();

    qDebug() << "Deserialized Trigger Time:" << newSystem.triggerTime();
}

// TODO
void Widget::onRules()
{
    qDebug() << "规则管理 -- 开发中，当前使用程序中的默认规则";
}

// 日志消息处理函数
void Widget::logMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    Q_UNUSED(context);
    // test();
    QString logMessage = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz ");
    switch (type)
    {
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

    if (globalLogArea)
    {
        QMutexLocker locker(&logMutex);
        globalLogArea->append(logMessage);
    }
}


