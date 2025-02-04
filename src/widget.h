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

 protected:
  void closeEvent(QCloseEvent* event) override;

 private:
  // UI 组件
  Ui::Widget* ui;
  QTextEdit* logArea;                          // 日志显示区域
  QVBoxLayout* workspaceLayout;                // 工作空间布局
  QScopedPointer<QProgressBar> progressBar;    // 进度条
  QScopedPointer<QListWidget> taskListWidget;  // 任务列表
  QScopedPointer<QSystemTrayIcon> trayIcon;    // 系统托盘图标

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
                        const QString& dest);  // 检查文件版本
  void writeErrLog(const QString& msg);        // 写入错误日志
  // static bool checkFileIsDir(QString const& file1,
  //                            QString const& file2);  // 检查文件是否是目录

  QAction* syncAction;         // 同步动作
  QAction* triggerTimeAction;  // 触发时间动作
  // 初始化监控器
  TriggerMonitor* monitor;

  // 配置文件操作
  void loadWorkspaceConfig();   // 加载工作空间配置
  void saveWorkspaceConfig();   // 保存工作空间配置
  void clearWorkspaceConfig();  // 清除工作空间配置

  // 系统状态
  void isTaskRunning();

  std::atomic<int> completedTasks{0};  // 原子计数器，用于记录完成的任务数量

 private slots:
  // 槽函数
  void onNewProject();         // 新建配置
  void onOpenConfiguration();  // 打开配置文件
  void onSaveConfiguration();  // 保存配置文件
  void onExit();               // 退出程序
  void onAbout();              // 关于信息
  void onSync();               // 立即同步
  void onSystemInfo();         // 系统信息
  void onUpdateLog();          // 更新日志
  void onGetLatestVersion();   // 获取最新版本
  void onRules();              // 规则管理

  // 点击同步按钮时触发
  void onSyncActionClicked() const;
  // 当同步任务完成时的回调
  void onTaskCompleted(const QString& source, const QString& target) const;
  // 当触发同步任务时的通知（可用于日志记录）
  static void onSyncTriggered(const QString& source, const QString& target);
};

#endif  // WIDGET_H
