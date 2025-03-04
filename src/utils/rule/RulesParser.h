#ifndef RULESPARSER_H
#define RULESPARSER_H

#include <QStringList>

/**
 * @brief RulesParser 类用于解析文件同步规则配置文件
 *
 * 该类负责解析位于可执行程序目录下 etc 文件夹中的 FileSyncRules.conf 配置文件，
 * 从中提取需要排除的目录（exclude_dirs）和排除的文件扩展名（exclude_exts）。
 */
class RulesParser
{
public:
  /**
   * @brief 构造函数
   * @param configPath 配置文件的完整路径
   */
  explicit RulesParser(const QString &configPath);

  /**
   * @brief 重新解析配置文件内容
   */
  void parseConfig();

  /**
   * @brief 获取排除目录列表
   * @return 排除目录的字符串列表
   */
  const QStringList &getExcludeDirs() const;

  /**
   * @brief 获取排除文件扩展名列表
   * @return 排除文件扩展名的字符串列表
   */
  const QStringList &getExcludeExts() const;

private:
  QString filePath;         ///< 配置文件的完整路径
  QStringList excludeDirs;  ///< 排除目录列表
  QStringList excludeExts;  ///< 排除文件扩展名列表

  /**
   * @brief 解析配置文件中的一行（格式：key = value）
   * @param line 待解析的配置行
   * @param list 用于存放解析结果的字符串列表
   */
  void parseList(const QString &line, QStringList &list);
  void validateAndAccept();
};

#endif // RULESPARSER_H
