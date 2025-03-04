#ifndef ENV_H
#define ENV_H
#include <QString>

class Env {
 public:
  QString appDir;          // 应用程序目录
  static Env& instance();  // 单例
  void checkAndInit() const;
  bool isAppDirExists() const;  // 应用程序目录是否存在

 private:
  Env();
  Env(const Env&) = delete;
  Env& operator=(const Env&) = delete;
};

#endif  // ENV_H
