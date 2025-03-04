#ifndef TRIGGERMONITOR_H
#define TRIGGERMONITOR_H

#include <qqueue.h>

#include <QComboBox>
#include <QDebug>
#include <QFileSystemWatcher>
#include <QFormLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QStandardPaths>
#include <QString>
#include <QTimeEdit>
#include <QTimer>

// 前向声明同步任务类
class SyncTask;
class Widget;

class TriggerMonitor : public QObject {
  Q_OBJECT
 public:
  explicit TriggerMonitor(QObject *parent = nullptr, Widget *widget = nullptr);
  ~TriggerMonitor() override;

  void setWidget(Widget *widget);  // 传递 Widget 指针

  // 开始/停止监控
  void startMonitoring(const QString &configFilePath);
  void stopMonitoring();
  static QList<QPair<QString, QString>> loadWorkspacesFromConfig(
      const QString &configFilePath);

  // 立即触发同步任务
  void triggerSync(const QString &configFilePath);

  // 显示设置对话框，允许用户修改触发间隔、最大线程数、同步时间、上次同步时间、触发频率以及每周允许触发的日子
  void showSettingsDialog();

 signals:
  // 配置发生变化时发出信号
  void configChanged();
  // 每次触发同步时发出信号，参数为源路径和目标路径
  void syncTriggered(const QString &sourcePath, const QString &targetPath);
  // 同步任务完成时发出信号
  void taskCompleted(const QString &source, const QString &target);
  // 更新进度条
  void progressUpdated(int progress);

 public slots:
  // 定时器超时回调，检查是否满足触发同步的条件
  void checkSyncTime();
  // 同步任务完成的槽函数
  void onTaskCompleted(const QString &source, const QString &target);
  void allTasksCompleted() const;

 private:
  QString configFilePath;  // 同步文件路径

  // 配置文件读写
  void loadConfig();
  void saveConfig() const;

  // 重启定时器（例如在修改设置后）
  void restartTimer();

  // 清理当前同步任务对象
  void cleanupTask();

  // 定时扫描使用的定时器
  QTimer pollingTimer;

  // 以下为配置参数
  int m_triggerMinutes;  // UI上设置的基本触发间隔（分钟），例如用于调整设置对话框的显示值
  int m_maxThreads;               // 最大线程数
  QString m_sourcePath;           // 同步源路径
  QString m_targetPath;           // 同步目标路径
  QTime syncTime;                 // 同步时间（备用，如果需要每天固定时间触发）
  QDateTime m_lastSyncTime;       // 上一次同步触发的时间
  int m_triggerFrequencyMinutes;  // 实际定时触发同步的频率（单位：分钟）
  QSet<Qt::DayOfWeek>
      m_triggerWeekDays;  // 允许触发同步的星期（例如：只在工作日触发）

  bool m_isTaskRunning;     // 当前是否有同步任务正在运行
  SyncTask *m_currentTask;  // 当前正在运行的同步任务
  Widget *m_widget;         // 确保 Widget* 不是 nullptr

  // 配置文件名称及最小线程数常量
  static const QString CONFIG_FILE;
  static const int MIN_THREADS;
  std::atomic<int> completedTasks{0};  // 原子计数器，用于记录完成的任务数量

  QGroupBox *weekDaysGroup;
  QFormLayout *layout;
  QComboBox *timeCombo;
  QSpinBox *threadSpin;
  QDateTimeEdit *lastSyncEdit;
  QHBoxLayout *weekLayout;
  QTimeEdit *timeEdit;
  QSpinBox *frequencySpin;

 QFileSystemWatcher fileWatcher; // 文件系统监控器
 QQueue<QString> syncQueue;      // 缓存队列
 const int SYNC_QUEUE_THRESHOLD = 4; // 队列阈值

 void setupFileWatcher(const QString &path); // 初始化文件监控
 void processSyncQueue();                    // 处理缓存队列
};

#endif  // TRIGGERMONITOR_H
