/**
******************************************************************************  
* File Name      : test_main.cpp  
* Author         : 17872  
* E-mail         : liyangyang0713@foxmail.com  
* Create         : 2025/2/22 3:51  
******************************************************************************  
*/
// test/test_main.cpp
#include <QtTest>
#include "test_leveldb.h"

int main(int argc, char *argv[]) {
  QCoreApplication app(argc, argv);  // 如果无需 GUI 组件，使用 QCoreApplication
  int status = 0;

  status |= QTest::qExec(new TestLevelDB, argc, argv);
  return status;
}