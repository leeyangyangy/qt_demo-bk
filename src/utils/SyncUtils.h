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

  /**
   * @brief 检查路某一位置是否具有读写权限
   *
   * @param filePath 磁盘路径
   * @return int 0 表示没有权限，1 表示有读权限，2 表示有写权限，3
   * 表示有读写权限
   */
  static int checkFilePathPermissions(const QString& filePath);

  /**
   * @brief 检查路某一位置是否具有读写权限 2
   *
   * @param filePath 磁盘路径
   * @return bool 0 表示没有权限，1 表示有读权限，2 表示有写权限，3
   * 表示有读写权限
   */
  static bool testFilePathWritablePermissions(const QString& filePath);

  /**
   * @brief 字符串生成hash值
   * @param input 输入字符串
   * @return 返回hash字符串
   */
  static QString computeXXHash(const QString& input);

  static QString fastFileHash(const QString &filePath);
  static QString fullFileHash(const QString &filePath);

};

#endif  // SYNCUTILS_H
