#include "TriggerMonitor.h"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "../task/SyncTask.h"
#include "../utils/SyncUtils.h"
#include "../widget.h"
// TODO 加载配置和保存配置内容可以优化，应该从其它地方获取加载进来的，先这样写死吧
// 定义配置文件名
const QString TriggerMonitor::CONFIG_FILE = "trigger_config.json";
// 最小线程数
const int TriggerMonitor::MIN_THREADS = 1;

TriggerMonitor::TriggerMonitor(QObject *parent, Widget *widget)
    : QObject(parent),
      configFilePath(QString("%1/%2/%3")
                         .arg(QCoreApplication::applicationDirPath(), "etc",
                              "files.json")),
      m_triggerMinutes(60),
      m_maxThreads(QThread::idealThreadCount()),
      m_isTaskRunning(false),
      m_currentTask(nullptr),  // 轮询定时器超时回调，检查是否满足触发同步的条件
      m_widget(widget) {
  // 这里建议为 pollingTimer 指定 this 为父对象，确保定时器正常工作
  pollingTimer.setParent(this);
  connect(&pollingTimer, &QTimer::timeout, this,
          &TriggerMonitor::checkSyncTime);
  loadConfig();

  for (const auto& row : loadWorkspacesFromConfig(configFilePath)) {
    setupFileWatcher(row.first);
  }
  // QThreadPool::globalInstance()->setMaxThreadCount(m_maxThreads);
}

void TriggerMonitor::setWidget(Widget *widget) { m_widget = widget; }

TriggerMonitor::~TriggerMonitor() { stopMonitoring(); }

void TriggerMonitor::loadConfig() {
  const QString configPath =
      QString("%1/%2/%3")
          .arg(QCoreApplication::applicationDirPath(), "etc", CONFIG_FILE);
  QFile configFile(configPath);

  if (!configFile.exists()) {
    qDebug() << "配置文件不存在，创建默认配置";
    saveConfig();  // 先保存默认配置
  }

  if (configFile.open(QIODevice::ReadOnly)) {
    QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
    QJsonObject config = doc.object();
    m_triggerMinutes = config["triggerMinutes"].toInt(5);
    m_maxThreads = qMax(
        MIN_THREADS, config["maxThreads"].toInt(QThread::idealThreadCount()));
    m_sourcePath = config["sourcePath"].toString();
    m_targetPath = config["targetPath"].toString();
    syncTime = QTime::fromString(config["syncTime"].toString(), "HH:mm");
    m_triggerFrequencyMinutes = config["triggerFrequency"].toInt(5);

    // 读取每周触发日（存为数组）
    QJsonArray days = config["triggerWeekDays"].toArray();
    m_triggerWeekDays.clear();
    for (const QJsonValue &v : days) {
      int day = v.toInt();
      m_triggerWeekDays.insert(static_cast<Qt::DayOfWeek>(day));
    }

    // 读取上次同步时间
    QString lastSyncStr = config["lastSyncTime"].toString();
    m_lastSyncTime = QDateTime::fromString(lastSyncStr, Qt::ISODate);

    configFile.close();
  } else {
    qWarning() << "无法打开配置文件，使用默认值";
  }

  emit configChanged();
}

void TriggerMonitor::saveConfig() const {
  QJsonObject config;
  config["triggerMinutes"] = m_triggerMinutes;
  config["maxThreads"] = m_maxThreads;
  config["sourcePath"] = m_sourcePath;
  config["targetPath"] = m_targetPath;
  config["syncTime"] = syncTime.toString("HH:mm");
  config["triggerFrequency"] = m_triggerFrequencyMinutes;
  // 将每周触发日存为数组（整数值对应 Qt::DayOfWeek）
  QJsonArray days;
  for (Qt::DayOfWeek d : m_triggerWeekDays) {
    days.append(static_cast<int>(d));
  }
  config["triggerWeekDays"] = days;
  config["lastSyncTime"] = m_lastSyncTime.toString(Qt::ISODate);

  const auto configPath =
      QString("%1/%2/%3")
          .arg(QCoreApplication::applicationDirPath(), "etc", CONFIG_FILE);
  if (QFile configFile(configPath); configFile.open(QIODevice::WriteOnly)) {
    const QJsonDocument doc(config);
    configFile.write(doc.toJson());
    configFile.close();
  }
}

void TriggerMonitor::showSettingsDialog() {
  QDialog dialog;
  dialog.setWindowTitle("同步设置");
  layout = new QFormLayout(&dialog);

  // 触发间隔设置（旧版）
  timeCombo = new QComboBox(&dialog);
  // 测试阶段，暂时使用小时间
  timeCombo->addItems({"1", "2", "5", "10", "30", "60", "120", "300", "600",
                       "900", "1800", "3600"});
  // 1 * 1000 * 60 = 1分钟
  timeCombo->setCurrentText(QString::number(m_triggerMinutes * 1000 * 60));
  layout->addRow("软件扫描器时间间隔(分钟):", timeCombo);

  // 最大线程数设置
  threadSpin = new QSpinBox(&dialog);
  threadSpin->setRange(MIN_THREADS, QThread::idealThreadCount() * 2);
  threadSpin->setValue(m_maxThreads);
  layout->addRow("最大线程数:", threadSpin);

  // 新增：上次同步时间设置（允许用户修改）
  lastSyncEdit = new QDateTimeEdit(
      m_lastSyncTime.isNull() ? QDateTime::currentDateTime() : m_lastSyncTime,
      &dialog);
  lastSyncEdit->setCalendarPopup(true);
  layout->addRow("上次同步时间:", lastSyncEdit);

  // 新增：触发频率（单位：分钟）控制
  frequencySpin = new QSpinBox(&dialog);
  frequencySpin->setRange(1, 1440);  // 1 分钟到 24 小时
  frequencySpin->setValue(m_triggerFrequencyMinutes);
  layout->addRow("触发频率(分钟):", frequencySpin);

  // 新增：每周触发日设置
  weekDaysGroup = new QGroupBox("每周触发日(可选)", &dialog);
  weekLayout = new QHBoxLayout(weekDaysGroup);
  // 定义周一到周日的复选框和对应的标签
  QCheckBox *weekCheckBoxes[7] = {new QCheckBox("周一", weekDaysGroup),
                                  new QCheckBox("周二", weekDaysGroup),
                                  new QCheckBox("周三", weekDaysGroup),
                                  new QCheckBox("周四", weekDaysGroup),
                                  new QCheckBox("周五", weekDaysGroup),
                                  new QCheckBox("周六", weekDaysGroup),
                                  new QCheckBox("周日", weekDaysGroup)};

  // 添加复选框到布局中
  for (int i = 0; i < 7; ++i) {
    weekLayout->addWidget(weekCheckBoxes[i]);
  }
  layout->addRow(weekDaysGroup);

  // 如果 m_triggerWeekDays 为空，则默认选中工作日
  for (int i = 0; i < 7; ++i) {
    if (m_triggerWeekDays.isEmpty()) {
      weekCheckBoxes[i]->setChecked(i < 5);  // 默认选中周一至周五
    } else {
      weekCheckBoxes[i]->setChecked(
          m_triggerWeekDays.contains(static_cast<Qt::DayOfWeek>(i + 1)));
    }
  }

  // 同步时间设置（原先用于每天某个时间触发，可留作备用）
  timeEdit = new QTimeEdit(syncTime, &dialog);
  layout->addRow("同步时间(可选):", timeEdit);

  // 使用 unique_ptr 来管理 QDialogButtonBox
  auto buttons = std::make_unique<QDialogButtonBox>(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
  layout->addRow(buttons.get());  // 获取原始指针传给布局

  connect(buttons.get(), &QDialogButtonBox::accepted, &dialog,
          &QDialog::accept);
  connect(buttons.get(), &QDialogButtonBox::rejected, &dialog,
          &QDialog::reject);

  if (dialog.exec() == QDialog::Accepted) {
    // 更新基本设置
    m_triggerMinutes = timeCombo->currentText().toInt();
    m_maxThreads = threadSpin->value();
    syncTime = timeEdit->time();

    // 更新上次同步时间与触发频率
    m_lastSyncTime = lastSyncEdit->dateTime();
    m_triggerFrequencyMinutes = frequencySpin->value();

    // 更新每周触发日
    m_triggerWeekDays.clear();
    for (int i = 0; i < 7; ++i) {
      if (weekCheckBoxes[i]->isChecked()) {
        m_triggerWeekDays.insert(static_cast<Qt::DayOfWeek>(i + 1));
      }
    }

    // 保存配置
    saveConfig();
    emit configChanged();
    qDebug() << "配置已更新：" << m_triggerMinutes << "分钟, 最大线程数："
             << m_maxThreads << ", 同步时间：" << syncTime.toString()
             << ", 上次同步时间：" << m_lastSyncTime.toString()
             << ", 触发频率：" << m_triggerFrequencyMinutes
             << "分钟, 每周触发日：" << m_triggerWeekDays;

    // 重新启动定时器，更新检查间隔（例如，间隔可以采用较短的时间以便精细扫描）
    restartTimer();

    // 根据用户意愿
    // triggerSync(configFilePath);
  }
}

void TriggerMonitor::startMonitoring(const QString &configFilePath) {
  if (!pollingTimer.isActive()) {
    this->configFilePath = configFilePath;
    // 每分钟检查一次
    pollingTimer.start(m_triggerMinutes);
    qDebug() << "轮询监控已启动，每" << m_triggerMinutes << "分钟检查一次";
  }
}

void TriggerMonitor::stopMonitoring() {
  pollingTimer.stop();
  cleanupTask();
  qDebug() << "轮询监控已停止";
}

QList<QPair<QString, QString>> TriggerMonitor::loadWorkspacesFromConfig(const QString &configFilePath) {
  QList<QPair<QString, QString>> workspaces;

  QFile configFile(configFilePath);
  if (!configFile.open(QIODevice::ReadOnly)) {
    qWarning() << "无法打开配置文件，请检查路径：" << configFilePath;
    return workspaces;
  }

  const QByteArray data = configFile.readAll();
  configFile.close();

  const QJsonDocument doc = QJsonDocument::fromJson(data);
  if (!doc.isObject()) {
    qWarning() << "配置文件格式错误，请检查：" << configFilePath;
    return workspaces;
  }

  QJsonArray rows = doc.object().value("workspace").toArray();
  if (rows.isEmpty()) {
    qWarning() << "配置文件中没有工作目录，无法进行同步：" << configFilePath;
    return workspaces;
  }

  for (const QJsonValue &value : rows) {
    QJsonObject row = value.toObject();
    QString file1 = row.value("file1").toString();
    QString file2 = row.value("file2").toString();

    if (file1.isEmpty() || file2.isEmpty()) {
      qWarning() << "配置文件中的文件夹路径为空，跳过该任务：" << file1 << file2;
      continue;
    }

    if (SyncUtils::checkDocCompare(file1, file2)) {
      qWarning() << "监听路径和目标路径相同，跳过该任务：" << file1 << file2;
      continue;
    }

    if (!SyncUtils::checkFileIsDir(file1, file2)) {
      qWarning() << "监听路径或目标路径不是文件夹，跳过该任务：" << file1 << file2;
      continue;
    }

    workspaces.append(qMakePair(file1, file2));
  }

  return workspaces;
}

void TriggerMonitor::triggerSync(const QString &configFilePath) {
  if (m_isTaskRunning) {
    qDebug()
        << "TriggerMonitor::startMonitoring():前一任务仍在运行，跳过本次触发";
    return;
  }

  // 从配置文件中加载工作目录
  QList<QPair<QString, QString>> rows = loadWorkspacesFromConfig(configFilePath);
  if (rows.isEmpty()) {
    qWarning() << "没有有效的同步任务";
    m_isTaskRunning = false;
    return;
  }

  int taskCount = 0;
  completedTasks.store(0);  // 任务开始前重置计数器
  m_isTaskRunning = true;

  // 初始化进度条
  if (m_widget) {
    m_widget->updateProgressBar(0);
    m_widget->setSyncActionEnabled(false);  // 任务开始禁用 syncAction
  }

  int totalTasks = rows.size();

  for (const auto &row : rows) {
    QString file1 = row.first;
    QString file2 = row.second;

    ++taskCount;
    const auto task = new SyncTask(file1, file2);

    connect(task, &SyncTask::taskCompleted, this,
            [this, taskCount, totalTasks](const QString &, const QString &) {
                this->completedTasks.fetch_add(1, std::memory_order_relaxed);

                // 计算进度百分比
                const int progress = static_cast<double>(completedTasks) / totalTasks * 100;
                if (m_widget) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));  // 睡眠100ms
                    m_widget->updateProgressBar(progress);
                }

                if (totalTasks == taskCount) {
                    m_isTaskRunning = false;
                    emit allTasksCompleted();
                }
            });

    QThreadPool::globalInstance()->start(task);
  }

  if (taskCount == 0) {
    qWarning() << "TriggerMonitor::startMonitoring():没有有效的同步任务";
    m_isTaskRunning = false;
  }
}

void TriggerMonitor::checkSyncTime() {
  if (m_isTaskRunning) return;

  const QDateTime now = QDateTime::currentDateTime();
  int currentDay = now.date().dayOfWeek();  // Monday = 1, ..., Sunday = 7
  // 如果今天不是用户允许的触发日，则跳过
  if (!m_triggerWeekDays.contains(static_cast<Qt::DayOfWeek>(currentDay))) {
    return;
  }
  // 如果上次同步时间为空或间隔达到设定频率，则触发同步
  if (m_lastSyncTime.isNull() ||
      m_lastSyncTime.secsTo(now) >= m_triggerFrequencyMinutes * 60) {
    qDebug() << "触发同步任务，当前时间：" << now.toString() << "上次同步时间："
             << m_lastSyncTime.toString();
    triggerSync(configFilePath);
    m_lastSyncTime = now;  // 更新上次同步时间
    saveConfig();
  }
}

void TriggerMonitor::onTaskCompleted(const QString &source,
                                     const QString &target) {
  m_isTaskRunning = false;
  emit taskCompleted(source, target);
}

void TriggerMonitor::cleanupTask() {
  if (m_currentTask) {
    m_currentTask->deleteLater();
    m_currentTask = nullptr;
  }
  m_isTaskRunning = false;
}

void TriggerMonitor::restartTimer() {
  pollingTimer.stop();
  // 这里建议设置为较短的间隔以便精细扫描，例如 30000 毫秒（30 秒）
  pollingTimer.start(m_triggerMinutes);
  m_isTaskRunning = false;
}

void TriggerMonitor::allTasksCompleted() const {
  if (m_widget) {
    m_widget->setSyncActionEnabled(true);  // 任务完成后启用 syncAction
  }
}

/**
 * @brief 初始化监听文件
 * 设计思路：
 *   1. 使用
 * @param path
 */
void TriggerMonitor::setupFileWatcher(const QString &path) {
  if (!fileWatcher.addPath(path)) {
    qWarning() << "Failed to add path to file watcher:" << path;
    return;
  }

  connect(&fileWatcher, &QFileSystemWatcher::fileChanged, this, [this](const QString &filePath) {
      qDebug() << "File changed:" << filePath;
      syncQueue.enqueue(filePath);

      // 如果队列达到阈值，触发同步
      if (syncQueue.size() >= SYNC_QUEUE_THRESHOLD) {
          processSyncQueue();
      }
  });

  connect(&fileWatcher, &QFileSystemWatcher::directoryChanged, this, [this](const QString &dirPath) {
      qDebug() << "Directory changed:" << dirPath;

      // 扫描目录变化，获取新增或修改的文件
      const QDir dir(dirPath);
      for (const QFileInfo &fileInfo : dir.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot)) {
          syncQueue.enqueue(fileInfo.filePath());
      }

      // 如果队列达到阈值，触发同步
      if (syncQueue.size() >= SYNC_QUEUE_THRESHOLD) {
          processSyncQueue();
      }
  });
}

void TriggerMonitor::processSyncQueue() {
  // TODO: 增量逻辑
  qDebug() << "Processing sync queue with" << syncQueue.size() << "items...";

  while (!syncQueue.isEmpty()) {
    QString filePath = syncQueue.dequeue();
    qDebug() << "Syncing file:" << filePath;

    // 调用同步逻辑（可以复用现有的 SyncTask）
    // 示例：triggerSyncForFile(filePath);
  }
}