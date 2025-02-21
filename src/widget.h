#ifndef WIDGET_H
#define WIDGET_H

#include <QApplication>
#include <QCloseEvent>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QGroupBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QMessageBox>
#include <QMutex>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QShortcut>
#include <QSystemTrayIcon>
#include <QTextEdit>
#include <QThreadPool>
#include <QVBoxLayout>

#include "monitor/TriggerMonitor.h"
#include "ui_Widget.h"

class Widget : public QMainWindow {
  Q_OBJECT

 public:
  explicit Widget(QWidget* parent = nullptr);
  ~Widget() override;
  void updateProgressBar(int progress) const;  // 更新进度条

 public slots:
  void setSyncActionEnabled(
      bool status) const;  // 允许外部调用以控制 syncAction 启用
  void onStopTriggerTimeActionClicked() const;
  void onStartTriggerTimeActionClicked() const;

 protected:
  void closeEvent(QCloseEvent* event) override;

 signals:
  void rulesUpdated();

 private:
  // UI 组件
  Ui::Widget* ui;
  QTextEdit* logArea;            // 日志显示区域
  QVBoxLayout* workspaceLayout;  // 工作空间布局
  QGroupBox* workspaceGroup;     // 工作空间组
  QScrollArea* scrollArea;       // 工作空间组 -- 滚动区域
  QWidget* scrollContent;        // 工作空间组 -- 滚动
  // QPushButton* addRowsButton;                  // 工作空间组 -- 添加
  QVBoxLayout* groupLayout;                    // 工作空间组 -- 添加
  QHBoxLayout* logControlLayout;               // 日志区域 -- 布局
  QScopedPointer<QProgressBar> progressBar;    // 进度条
  QScopedPointer<QListWidget> taskListWidget;  // 任务列表
  QScopedPointer<QSystemTrayIcon> trayIcon;    // 系统托盘图标
  TriggerMonitor* monitor;                     // 初始化监控器
  QWidget* centralWidget;                      // 界面布局
  QVBoxLayout* mainLayout;                     // 界面布局

  /**
   * 0. 日志
   * 1. 进度
   */
  std::vector<std::unique_ptr<QLabel>> labels;

  /**
   * 0. 工作空间组件 -- 添加新行
   * 1. 日志区域 -- 清空日志 clearLogButton
   */
  std::vector<std::unique_ptr<QPushButton>> buttons;

  // 配置文件路径
  // const QString configFilePath;    // 同步文件路径
  QString configFilePath;          // 同步文件路径
  const QString configSystemPath;  // 系统配置路径

  // 日志处理
  static QTextEdit* globalLogArea;  // 全局日志区域指针
  static QMutex logMutex;           // 日志互斥锁
  static void logMessageHandler(QtMsgType type,
                                const QMessageLogContext& context,
                                const QString& msg);

  // 初始化函数
  void setupMenuBar();                              // 设置菜单栏
  void setupWorkspace(QVBoxLayout* mainLayout);     // 设置工作空间
  void setupLogArea(QVBoxLayout* mainLayout);       // 设置日志区域
  void setupProgressArea(QVBoxLayout* mainLayout);  // 设置进度条区域
  // int totalTasks = 0;                               // 总任务数
  // int completedTasks = 0;                           // 已完成任务数

  // 文件操作
  void addFileRow();  // 添加文件行
  // bool checkDocCompare(const QString& src,
  //                      const QString& dest);  // 检查路径是否相同
  void checkFileVersion(const QString& src,
                        const QString& dest) const;  // 检查文件版本
  // static bool checkFileIsDir(QString const& file1,
  //                            QString const& file2);  // 检查文件是否是目录

  QMenuBar* menuBar;  // 菜单栏

  /**
   * 0. 配置
   * 1. 同步
   * 2. 帮助
   */
  std::vector<std::unique_ptr<QMenu>> menus;
  /**
   * 0. 配置 快捷键 C
   * 1. 同步 快捷键 S
   * 2. 帮助 快捷键 H
   */
  std::vector<std::unique_ptr<QShortcut>> menuShortcuts;

  /**
   * 配置action
   */
  std::vector<std::unique_ptr<QAction>> confActions;

  /**
   * 同步action
   * 0. 同步action
   * 1. 触发时间action triggerTimeAction
   * 2. 触发时间停止action stopTriggerTimeAction
   * 3. 触发时间开始action startTriggerTimeAction
   */
  std::vector<std::unique_ptr<QAction>> syncActions;

  /**
   * 帮助action
   */
  std::vector<std::unique_ptr<QAction>> helpActions;

  /**
   * 0. 文件校验
   * 1. 手动同步
   * 3. 开启服务
   * 4. 关闭服务
   * 5. 注册系统开机自启
   */
  std::vector<std::unique_ptr<QAction>> advanceActions;

  // 配置文件操作
  void loadWorkspaceConfig();         // 加载工作空间配置
  void saveWorkspaceConfig() const;   // 保存工作空间配置
  void clearWorkspaceConfig() const;  // 清除工作空间配置

  // 系统状态
  bool isTaskRunning = false;

  std::atomic<int> completedTasks{0};  // 原子计数器，用于记录完成的任务数量

  QAction* autoStartAction;         // 可勾选的菜单动作
  bool isAutoStartEnabled() const;  // 检查自启状态
  void setAutoStart(bool enable);   // 设置自启状态

  // 平台特定实现
#ifdef Q_OS_WIN
  const QString winRegPath =
      "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run";
  const QString winAppKey = "FileSyncManager";
#endif

 private slots:
  // 槽函数
  void onNewProject() const;         // 新建配置
  void onOpenConfiguration();        // 打开配置文件
  void onSaveConfiguration() const;  // 保存配置文件
  void onExit();                     // 退出程序
  void onAbout();                    // 关于信息
  void onSync() const;               // 立即同步
  void onSystemInfo() const;         // 系统信息
  QString getRuleFilePath() const;
  void createDefaultRules() const;
  void onUpdateLog();         // 更新日志
  void onGetLatestVersion();  // 获取最新版本
  void onRules();             // 规则管理
  void reloadRules();

  // 点击同步按钮时触发
  void onSyncActionClicked() const;
  // 当同步任务完成时的回调
  void onTaskCompleted(const QString& source, const QString& target) const;
  // 当触发同步任务时的通知（可用于日志记录）
  static void onSyncTriggered(const QString& source, const QString& target);

  void toggleAutoStart(bool checked);  // 响应勾选状态变化
};

#endif  // WIDGET_H
