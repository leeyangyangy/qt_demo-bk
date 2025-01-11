#ifndef WIDGET_H
#define WIDGET_H

#include "ui_Widget.h"

#include <QMainWindow>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QGroupBox>
#include <QMenuBar>
#include <QScrollArea>
#include <QPushButton>
#include <QMutex>
#include <QLabel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>
#include <QMessageBox>
#include <QDateTime>
#include <QCloseEvent>
#include <QDebug>
#include <QApplication>
#include <QFileDialog>
#include <QShortcut>
#include <QApplication>
#include <qfilesystemwatcher.h>
#include <QThreadPool>
#include <QFuture>
#include <QtConcurrent>
#include <QProgressBar>
#include <QListWidget>
#include <QSystemTrayIcon>

class Widget : public QMainWindow  // 继承自 QMainWindow
{
    Q_OBJECT

public:
    explicit Widget(QWidget* parent = nullptr);
    ~Widget() override;

signals:

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    Ui::Widget* ui;

    // 处理日志
    static void logMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    QTextEdit *logArea; // 用于显示日志的文本区域
    static QTextEdit *globalLogArea; // 静态日志区域指针
    static QMutex logMutex;          // 多线程保护

    // void addFileRow(QVBoxLayout *layout); // 动态添加文件信息行
    // void addFileRow(const QString& file1 = "", const QString& file2 = "");
    void addFileRow();
    void setupMenuBar();                 // 设置菜单栏
    void setupWorkspace(QVBoxLayout *mainLayout); // 设置工作空间
    void setupLogArea(QVBoxLayout *mainLayout);   // 设置日志区域

    const QString configFilePath; // 同步文件路径
    const QString configSystemPath; // 系统配置路径
    void loadWorkspaceConfig();
    void saveWorkspaceConfig();
    QVBoxLayout* workspaceLayout;

    // 文件core
    bool checkDocCompare(QString src,QString dest);
    void checkFileVersion(QString src,QString dest);
    void writeErrLog(QString msg);

    QProgressBar *progressBar; // 进度条
    QListWidget *taskListWidget; // 任务列表

private slots:
    void onNewProject(); // 新建配置
    void onOpenConfiguration(); // 打开配置文件
    void onSaveConfiguration(); // 保存配置文件
    void onExit();       // 退出程序
    void onAbout();      // 关于信息
    void onSync();      // 立即同步
    void onSystemInfo(); // 系统信息
    void onUpdateLog(); // 更新日志
    void onGetLatestVersion(); // 获取最新版本
    void onRules(); // 规则

};

#endif // WIDGET_H
