#include "System.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QDebug>

// 构造函数
System::System(QObject* parent) : QObject(parent)
{
}

// 获取触发时间
QString System::triggerTime() const
{
    return m_triggerTime;
}

// 设置触发时间
void System::setTriggerTime(const QString& time)
{
    m_triggerTime = time;
}

// 序列化为 JSON
QJsonObject System::toJson() const
{
    QJsonObject jsonObj;
    jsonObj["triggerTime"] = m_triggerTime; // 存储触发时间字符串
    return jsonObj;
}

// 从 JSON 反序列化
void System::fromJson(const QJsonObject& jsonObj)
{
    if (jsonObj.contains("triggerTime") && jsonObj["triggerTime"].isString())
    {
        m_triggerTime = jsonObj["triggerTime"].toString();
    }
}

// 选择触发时间
void System::chooseTriggerTime()
{
    bool ok;
    QStringList items;

    // 定义选项列表
    items << "1 分钟" << "2 分钟" << "5 分钟" << "10 分钟" << "30 分钟";

    // 使用 QInputDialog 显示选择对话框
    QString item = QInputDialog::getItem(nullptr, "选择触发时间",
                                         "请选择触发时间:", items, 0, false, &ok);

    if (ok && !item.isEmpty())
    {
        setTriggerTime(item); // 设置触发时间为用户选择的值
        QMessageBox::information(nullptr, "设置成功",
                                 QString("触发时间已设置为: %1").arg(m_triggerTime));
        // connect(this, &System::chooseTriggerTime, this, &System::onSaveSystem(m_triggerTime));
        // 发出 ok 信号以触发槽函数
        emit onSaveSystem(m_triggerTime); // 发出信号，传递当前的 m_triggerTime
    }
    else
    {
        QMessageBox::warning(nullptr, "选择无效", "未选择有效的触发时间。");
    }
}

// 保存触发时间的槽函数
void System::onSaveSystem(const QString time)
{
    setTriggerTime(time);
    qInfo() << "触发时间已保存为:" << time;
}

// TODO 待完成  优先级 1
void System::loadSystemConfig()
{
}

void System::saveSystemConfig()
{
}
