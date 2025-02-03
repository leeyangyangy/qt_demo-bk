// #ifndef TRIGGERMONITOR_H
// #define TRIGGERMONITOR_H
//
// #include <QDateTime>
// #include <QObject>
// #include <QTimer>
//
// // 前置声明：SyncTask 定义在 SyncTask.h 中
// class SyncTask;
//
// class TriggerMonitor : public QObject {
//   Q_OBJECT
//  public:
//   // 定义最小线程数和配置文件名
//   static const int MIN_THREADS;
//   static const QString CONFIG_FILE;  // 例如 "trigger_config.json"
//
//   explicit TriggerMonitor(QObject *parent = nullptr);
//   ~TriggerMonitor() override;
//
//   // 开始或停止监控同步任务
//   void startMonitoring();
//   void stopMonitoring();
//
//   // 立即触发同步（例如由按钮点击触发）
//   void triggerSync();
//
//  signals:
//   // 当配置更新后发出信号
//   void configChanged();
//   // 同步任务启动时发出信号（传递源、目标路径）
//   void syncTriggered(const QString &sourcePath, const QString &targetPath);
//
//  public slots:
//   // 显示设置对话框（可修改触发间隔、最大线程数、同步路径）
//   void showSettingsDialog();
//   // 定时器超时回调，启动同步任务
//   void onSyncTimeout();
//   // 同步任务完成回调
//   void onTaskCompleted(const QString &source, const QString &target);
//
//  private:
//   // 内部设置对话框（嵌套类）
//   class SettingsDialog;
//
//   // 配置加载和保存
//   void loadConfig();
//   void saveConfig();
//
//   // 重启定时器
//   void restartTimer();
//   // 清理当前任务
//   void cleanupTask();
//
//   QTimer m_syncTimer;        // 定时器
//   int m_triggerMinutes;      // 触发间隔（分钟）
//   int m_maxThreads;          // 最大线程数
//   QString m_sourcePath;      // 同步源目录
//   QString m_targetPath;      // 同步目标目录
//   bool m_isTaskRunning;      // 标记任务是否正在运行
//   QDateTime m_lastTaskTime;  // 上次任务启动时间
//
//   SyncTask *m_currentTask;  // 当前同步任务指针
// };
//
// #endif  // TRIGGERMONITOR_H

#ifndef TRIGGERMONITOR_H
#define TRIGGERMONITOR_H

#include <QObject>
#include <QTimer>
#include <QTime>
#include <QThreadPool>
#include <QJsonObject>
#include <QStandardPaths>

class SyncTask;

class TriggerMonitor : public QObject
{
 Q_OBJECT
public:
 explicit TriggerMonitor(QObject *parent = nullptr);
 ~TriggerMonitor() override;

 // 配置管理
 void loadConfig();
 void saveConfig() const;
 void showSettingsDialog();

 // 监控控制
 void startMonitoring();
 void stopMonitoring();
 void triggerSync(); // 外部调用接口：立即触发同步任务

 signals:
     void configChanged(); // 配置发生变化时触发
 void syncTriggered(const QString& source, const QString& target); // 同步任务开始时触发
 void taskCompleted(const QString& source, const QString& target); // 同步任务完成时触发

 private slots:
     void checkSyncTime(); // 每分钟检查一次是否到达同步时间
 void onTaskCompleted(const QString& source, const QString& target); // 任务完成槽函数

private:
 // 配置相关
 static const QString CONFIG_FILE;
 static const int MIN_THREADS;

 int m_triggerMinutes; // 触发间隔（分钟）
 int m_maxThreads;     // 最大线程数
 QString m_sourcePath; // 源路径
 QString m_targetPath; // 目标路径
 QTime syncTime;       // 同步时间

 // 任务状态
 QTimer pollingTimer;  // 轮询定时器
 bool m_isTaskRunning; // 任务是否正在运行
 SyncTask* m_currentTask; // 当前任务

 // 工具函数
 void cleanupTask(); // 清理任务资源
};

#endif // TRIGGERMONITOR_H
