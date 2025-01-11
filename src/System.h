// #ifndef SYSTEM_H
// #define SYSTEM_H
//
// #include <QObject>
// #include <QDateTime>
// #include <QJsonObject>
// #include <QJsonDocument>
//
// class System : public QObject {
//     Q_OBJECT
//
// public:
//     explicit System(QObject *parent = nullptr);
//
//     // 获取和设置触发时间
//     QDateTime triggerTime() const;
//     void setTriggerTime(const QDateTime &time);
//
//     // 序列化为 JSON
//     QJsonObject toJson() const;
//
//     // 从 JSON 反序列化
//     void fromJson(const QJsonObject &jsonObj);
//
// private:
//     QDateTime m_triggerTime; // 触发时间成员变量
// };
//
// #endif // SYSTEM_H

#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonDocument>

class System : public QObject
{
    Q_OBJECT

public:
    explicit System(QObject* parent = nullptr);

    // 获取和设置触发时间
    QString triggerTime() const;
    void setTriggerTime(const QString& time);

    // 序列化为 JSON
    QJsonObject toJson() const;

    // 从 JSON 反序列化
    void fromJson(const QJsonObject& jsonObj);

    // 选择触发时间
    void chooseTriggerTime();

private:
    QString m_triggerTime; // 触发时间成员变量
    void loadSystemConfig();
    void saveSystemConfig();

private slots:
    void onSaveSystem(const QString time); // 保存触发时间
};

#endif // SYSTEM_H
