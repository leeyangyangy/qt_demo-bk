#include "RulesParser.h"

#include <QDebug>
#include <QFile>
#include <QMessageBox>
#include <QRegularExpression>
#include <QTextStream>

RulesParser::RulesParser(const QString &configPath) : filePath(configPath) {
  parseConfig();
}

void RulesParser::parseConfig() {
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning() << "无法打开配置文件:" << filePath;
    return;
  }

  QTextStream in(&file);
  while (!in.atEnd()) {
    QString line = in.readLine().trimmed();

    // 忽略空行和注释行
    if (line.isEmpty() || line.startsWith("#")) continue;

    // 忽略文件头部标识行，如 [FileSync Rules v1.0.1]
    if (line.startsWith('[') && line.endsWith(']')) continue;

    // 根据关键字分别解析排除目录和排除文件扩展名规则
    if (line.startsWith("exclude_dirs")) {
      parseList(line, excludeDirs);
    } else if (line.startsWith("exclude_exts")) {
      parseList(line, excludeExts);
    }
  }

  file.close();
}

const QStringList &RulesParser::getExcludeDirs() const { return excludeDirs; }

const QStringList &RulesParser::getExcludeExts() const { return excludeExts; }

void RulesParser::parseList(const QString &line, QStringList &list) {
  // 使用正则表达式匹配 "key = value" 格式，其中 key 为非空格字符串，value
  // 为任意字符
  const QRegularExpression regex(R"(^(\S+)\s*=\s*(.+)$)");
  const QRegularExpressionMatch match = regex.match(line);
  if (!match.hasMatch()) {
    qWarning() << "配置行格式错误:" << line;
    return;
  }

  // 获取等号右侧的值
  QString values = match.captured(2);
  // 如果存在内联注释（#号后面的部分），则移除
  if (const int commentIndex = values.indexOf('#'); commentIndex != -1) {
    values = values.left(commentIndex).trimmed();
  }

  // 按逗号拆分字符串，拆分后去除多余空白并过滤空项
  QStringList items = values.split(",", QString::SkipEmptyParts);
  for (QString &item : items) {
    item = item.trimmed();
    if (!item.isEmpty()) {
      list.append(item);
    }
  }
}
