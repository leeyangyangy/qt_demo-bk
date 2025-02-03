// #include "TriggerMonitor.h"
//
// #include <QComboBox>
// #include <QDebug>
// #include <QDialogButtonBox>
// #include <QFile>
// #include <QFormLayout>
// #include <QJsonDocument>
// #include <QJsonObject>
// #include <QMessageBox>
// #include <QSpinBox>
// #include <QStandardPaths>
// #include <QThread>
//
// #include "../task/SyncTask.h"
//
// // 定义配置文件名
// const QString TriggerMonitor::CONFIG_FILE = "trigger_config.json";
// // 定义最小线程数（请在此处给出具体值，例如 1）
// const int TriggerMonitor::MIN_THREADS = 1;
//
// // 内部设置对话框实现
// class TriggerMonitor::SettingsDialog : public QDialog {
//  public:
//   SettingsDialog(QWidget* parent, int currentMinutes, int currentThreads)
//       : QDialog(parent) {
//     setWindowTitle("同步设置");
//     QFormLayout* layout = new QFormLayout(this);
//
//     // 触发间隔设置
//     timeCombo = new QComboBox(this);
//     timeCombo->addItems({"1", "2", "5", "10", "30"});
//     timeCombo->setCurrentText(QString::number(currentMinutes));
//     layout->addRow("触发间隔（分钟）:", timeCombo);
//
//     // 线程数设置
//     const int maxThreads = qMax(MIN_THREADS, QThread::idealThreadCount() *
//     2); threadSpin = new QSpinBox(this); threadSpin->setRange(MIN_THREADS,
//     maxThreads); threadSpin->setValue(currentThreads);
//     layout->addRow("最大线程数:", threadSpin);
//
//     // 按钮组
//     QDialogButtonBox* buttons = new QDialogButtonBox(
//         QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
//     connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
//     connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
//     layout->addRow(buttons);
//   }
//
//   int selectedMinutes() const { return timeCombo->currentText().toInt(); }
//   int selectedThreads() const { return threadSpin->value(); }
//
//  private:
//   QComboBox* timeCombo;
//   QSpinBox* threadSpin;
// };
//
// // TriggerMonitor 构造函数
// TriggerMonitor::TriggerMonitor(QObject* parent)
//     : QObject(parent),
//       m_triggerMinutes(5),
//       m_maxThreads(QThread::idealThreadCount()),
//       m_isTaskRunning(false),
//       m_currentTask(nullptr) {
//   connect(&m_syncTimer, &QTimer::timeout, this,
//   &TriggerMonitor::onSyncTimeout); loadConfig();
// }
//
// TriggerMonitor::~TriggerMonitor() { stopMonitoring(); }
//
// void TriggerMonitor::loadConfig() {
//   QString configPath =
//       QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/"
//       + CONFIG_FILE;
//   QFile configFile(configPath);
//   if (configFile.open(QIODevice::ReadOnly)) {
//     QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
//     QJsonObject config = doc.object();
//     m_triggerMinutes = config["triggerMinutes"].toInt(5);
//     m_maxThreads = qMax(
//         MIN_THREADS,
//         config["maxThreads"].toInt(QThread::idealThreadCount()));
//     m_sourcePath = config["sourcePath"].toString();
//     m_targetPath = config["targetPath"].toString();
//     configFile.close();
//   }
//   m_syncTimer.setInterval(m_triggerMinutes * 60 * 1000);
//   emit configChanged();
// }
//
// void TriggerMonitor::saveConfig() {
//   QJsonObject config;
//   config["triggerMinutes"] = m_triggerMinutes;
//   config["maxThreads"] = m_maxThreads;
//   config["sourcePath"] = m_sourcePath;
//   config["targetPath"] = m_targetPath;
//   QString configPath =
//       QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/"
//       + CONFIG_FILE;
//   QFile configFile(configPath);
//   if (configFile.open(QIODevice::WriteOnly)) {
//     QJsonDocument doc(config);
//     configFile.write(doc.toJson());
//     configFile.close();
//   }
// }
//
// void TriggerMonitor::showSettingsDialog() {
//   SettingsDialog dialog(nullptr, m_triggerMinutes, m_maxThreads);
//   if (dialog.exec() == QDialog::Accepted) {
//     bool changed = false;
//     int newMinutes = dialog.selectedMinutes();
//     if (newMinutes != m_triggerMinutes) {
//       m_triggerMinutes = newMinutes;
//       changed = true;
//     }
//     int newThreads = dialog.selectedThreads();
//     if (newThreads != m_maxThreads) {
//       m_maxThreads = qMax(MIN_THREADS, newThreads);
//       changed = true;
//     }
//     if (changed) {
//       saveConfig();
//       QMessageBox::information(
//           nullptr, "设置成功",
//           QString("配置已更新：\n触发间隔：%1分钟\n最大线程：%2")
//               .arg(m_triggerMinutes)
//               .arg(m_maxThreads));
//       emit configChanged();
//       restartTimer();
//     }
//   }
// }
//
// void TriggerMonitor::startMonitoring() {
//   if (!m_syncTimer.isActive()) {
//     m_syncTimer.start();
//     qDebug() << "监控已启动，触发间隔：" << m_triggerMinutes << "分钟";
//   }
// }
//
// void TriggerMonitor::stopMonitoring() {
//   m_syncTimer.stop();
//   cleanupTask();
//   qDebug() << "监控已停止";
// }
//
// void TriggerMonitor::triggerSync() {
//   // 外部调用接口：立即触发同步任务
//   onSyncTimeout();
// }
//
// void TriggerMonitor::onTaskCompleted(const QString& source,
//                                      const QString& target) {
//   qDebug() << "任务完成：" << source << "->" << target;
//   cleanupTask();
//   restartTimer();
// }
//
// void TriggerMonitor::onSyncTimeout() {
//   if (m_isTaskRunning) {
//     qDebug() << "前一任务仍在运行，跳过本次触发";
//     return;
//   }
//   // if (m_sourcePath.isEmpty() || m_targetPath.isEmpty()) {
//   //   qDebug() << "错误：未配置同步路径";
//   //   return;
//   // }
//   m_currentTask = new SyncTask(m_sourcePath, m_targetPath);
//   connect(m_currentTask, &SyncTask::taskCompleted, this,
//           &TriggerMonitor::onTaskCompleted);
//   connect(m_currentTask, &SyncTask::taskCompleted, this,
//           &TriggerMonitor::syncTriggered);
//   // 创建独立线程运行同步任务
//   QThread* taskThread = new QThread();
//   m_currentTask->moveToThread(taskThread);
//   connect(taskThread, &QThread::started, m_currentTask, &SyncTask::run);
//   connect(m_currentTask, &SyncTask::taskCompleted, taskThread,
//   &QThread::quit); connect(taskThread, &QThread::finished, taskThread,
//   &QThread::deleteLater); m_isTaskRunning = true; m_lastTaskTime =
//   QDateTime::currentDateTime(); taskThread->start(); qDebug() <<
//   "新任务已启动，使用线程数：" << m_maxThreads; emit
//   syncTriggered(m_sourcePath, m_targetPath);
// }
//
// void TriggerMonitor::restartTimer() {
//   m_syncTimer.stop();
//   const int remaining = m_triggerMinutes * 60 * 1000 -
//                   m_lastTaskTime.msecsTo(QDateTime::currentDateTime());
//   if (remaining > 0) {
//     m_syncTimer.start(remaining);
//   } else {
//     m_syncTimer.start();
//   }
//   m_isTaskRunning = false;
// }
//
// void TriggerMonitor::cleanupTask() {
//   if (m_currentTask) {
//     m_currentTask->deleteLater();
//     m_currentTask = nullptr;
//   }
//   m_isTaskRunning = false;
// }
//

#include "TriggerMonitor.h"
#include "../task/SyncTask.h"

#include <QComboBox>
#include <QDebug>
#include <QDialogButtonBox>
#include <QFile>
#include <QFormLayout>
#include <QJsonDocument>
#include <QMessageBox>
#include <QSpinBox>
#include <QTimeEdit>

// 定义配置文件名
const QString TriggerMonitor::CONFIG_FILE = "trigger_config.json";
// 最小线程数
const int TriggerMonitor::MIN_THREADS = 1;

TriggerMonitor::TriggerMonitor(QObject *parent)
    : QObject(parent),
      m_triggerMinutes(5),
      m_maxThreads(QThread::idealThreadCount()),
      m_isTaskRunning(false),
      m_currentTask(nullptr)
{
    connect(&pollingTimer, &QTimer::timeout, this, &TriggerMonitor::checkSyncTime);
    loadConfig();
}

TriggerMonitor::~TriggerMonitor()
{
    stopMonitoring();
}

void TriggerMonitor::loadConfig()
{
    QString configPath =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" +
        CONFIG_FILE;
    QFile configFile(configPath);
    if (configFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
        QJsonObject config = doc.object();
        m_triggerMinutes = config["triggerMinutes"].toInt(5);
        m_maxThreads = qMax(MIN_THREADS, config["maxThreads"].toInt(QThread::idealThreadCount()));
        m_sourcePath = config["sourcePath"].toString();
        m_targetPath = config["targetPath"].toString();
        syncTime = QTime::fromString(config["syncTime"].toString(), "HH:mm");
        configFile.close();
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

    QString configPath =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" +
        CONFIG_FILE;
    QFile configFile(configPath);
    if (configFile.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(config);
        configFile.write(doc.toJson());
        configFile.close();
    }
}

void TriggerMonitor::showSettingsDialog()
{
    QDialog dialog;
    dialog.setWindowTitle("同步设置");
    QFormLayout *layout = new QFormLayout(&dialog);

    // 触发间隔设置
    QComboBox *timeCombo = new QComboBox(&dialog);
    timeCombo->addItems({"1", "2", "5", "10", "30"});
    timeCombo->setCurrentText(QString::number(m_triggerMinutes));
    layout->addRow("触发间隔（分钟）:", timeCombo);

    // 线程数设置
    QSpinBox *threadSpin = new QSpinBox(&dialog);
    threadSpin->setRange(MIN_THREADS, QThread::idealThreadCount() * 2);
    threadSpin->setValue(m_maxThreads);
    layout->addRow("最大线程数:", threadSpin);

    // 触发时间设置
    QTimeEdit *timeEdit = new QTimeEdit(syncTime, &dialog);
    layout->addRow("同步时间:", timeEdit);

    // 确认按钮
    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    layout->addRow(buttons);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        m_triggerMinutes = timeCombo->currentText().toInt();
        m_maxThreads = threadSpin->value();
        syncTime = timeEdit->time();
        saveConfig();
        emit configChanged();
        qDebug() << "配置已更新：" << m_triggerMinutes << "分钟, 最大线程数："
                 << m_maxThreads << ", 触发时间：" << syncTime.toString();
    }
}

void TriggerMonitor::startMonitoring()
{
    if (!pollingTimer.isActive()) {
        pollingTimer.start(60000); // 每分钟检查一次
        qDebug() << "轮询监控已启动，每分钟检查一次";
    }
}

void TriggerMonitor::stopMonitoring()
{
    pollingTimer.stop();
    cleanupTask();
    qDebug() << "轮询监控已停止";
}

void TriggerMonitor::triggerSync()
{
    if (m_isTaskRunning) {
        qDebug() << "前一任务仍在运行，跳过本次触发";
        return;
    }

    if (m_sourcePath.isEmpty() || m_targetPath.isEmpty()) {
        qDebug() << "错误：未配置同步路径";
        // return;
    }

    m_currentTask = new SyncTask(m_sourcePath, m_targetPath);
    connect(m_currentTask, &SyncTask::taskCompleted, this, &TriggerMonitor::onTaskCompleted);
    QThreadPool::globalInstance()->start(m_currentTask);
    m_isTaskRunning = true;
    emit syncTriggered(m_sourcePath, m_targetPath);
}

void TriggerMonitor::checkSyncTime()
{
    if (m_isTaskRunning) {
        return;
    }

    QTime currentTime = QTime::currentTime();
    if (currentTime.hour() == syncTime.hour() &&
        currentTime.minute() == syncTime.minute()) {
        triggerSync();
    }
}

void TriggerMonitor::onTaskCompleted(const QString &source, const QString &target)
{
    m_isTaskRunning = false;
    emit taskCompleted(source, target);
}

void TriggerMonitor::cleanupTask()
{
    if (m_currentTask) {
        m_currentTask->deleteLater();
        m_currentTask = nullptr;
    }
    m_isTaskRunning = false;
}

// TODO 定时器暂未正常工作，需要排查问题