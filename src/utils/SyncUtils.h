#ifndef SYNCUTILS_H
#define SYNCUTILS_H

#include <QString>

class SyncUtils {
 public:
  // 禁止创建实例，只能使用静态方法
  SyncUtils() = delete;

  /**
   * @brief 检查两个文件夹路径是否相同
   *
   * @param src 监听路径
   * @param dest 目标路径
   * @return void
   */
  static bool checkDocCompare(QString const& src, QString const& dest);

  /**
   * @brief 检查路径是否为有效的目录
   *
   * @param src 监听路径
   * @param dest 目标路径
   * @return void
   */
  static bool checkFileIsDir(QString const& src, QString const& dest);
};

#endif  // SYNCUTILS_H
