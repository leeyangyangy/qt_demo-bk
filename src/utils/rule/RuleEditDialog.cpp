#include "RuleEditDialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QTextStream>
#include <QVBoxLayout>

#include "RuleHighlighter.h"

RuleEditDialog::RuleEditDialog(const QString& content, QWidget* parent)
    : QDialog(parent) {
  setWindowTitle(tr("测试功能 -- 未对关键内容校验，请不要修改该信息！！！"));
  setMinimumSize(800, 600);

  layout = new QVBoxLayout(this);

  // 文本编辑框
  textEdit = new QTextEdit(this);
  textEdit->setPlainText(content);
  textEdit->setFont(QFont("Consolas", 10));
  highlighter = new RuleHighlighter(textEdit->document());
  layout->addWidget(textEdit);

  // 按钮区域 TODO 确认后保存配置信息
  buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);

  buttons.push_back(std::make_unique<QPushButton>(tr("导入"), this));
  buttons.push_back(std::make_unique<QPushButton>(tr("导出"), this));
  buttonBox->addButton(buttons[0].get(), QDialogButtonBox::ActionRole);
  buttonBox->addButton(buttons[1].get(), QDialogButtonBox::ActionRole);

  layout->addWidget(buttonBox);

  // 连接信号
  connect(buttonBox, &QDialogButtonBox::accepted, this,
          &RuleEditDialog::validateAndAccept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(buttons[0].get(), &QPushButton::clicked, this, &RuleEditDialog::importRules);
  connect(buttons[1].get(), &QPushButton::clicked, this, &RuleEditDialog::exportRules);
}

QString RuleEditDialog::getEditedContent() const {
  return textEdit->toPlainText();
}

void RuleEditDialog::validateAndAccept() {
  const QString content = textEdit->toPlainText();

  // 必要字段验证
  if (!content.contains("sync_interval")) {
    QMessageBox::warning(this, tr("验证错误"),
                         tr("必须包含同步间隔配置 (sync_interval)"));
    return;
  }

  if (!content.contains("exclude_dirs") && !content.contains("exclude_exts")) {
    QMessageBox::warning(
        this, tr("验证错误"),
        tr("必须包含至少一个排除规则 (exclude_dirs 或 exclude_exts)"));
    return;
  }

  accept();

  // TODO 版本校验
  // const QString defaultContent =
  //   "[FileSync Rules v1.0]\n"
  //   "...";
  //
  // // 在validateAndAccept中添加版本检查
  // if (!content.contains("[FileSync Rules v1.0]")) {
  //   QMessageBox::warning(this, tr("版本错误"),
  //       tr("不兼容的规则版本"));
  //   return;
  // }
}

void RuleEditDialog::importRules() {
  const QString path = QFileDialog::getOpenFileName(
      this, tr("导入规则"),
      QDir::current().filePath("etc"),
      tr("配置文件 (*.conf);;所有文件 (*.*)"));

  if (!path.isEmpty()) {
    QFile file(path);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
      textEdit->setPlainText(QTextStream(&file).readAll());
      file.close();
    } else {
      QMessageBox::critical(this, tr("错误"), tr("无法打开文件"));
    }
  }
}

void RuleEditDialog::exportRules() {
  QString path = QFileDialog::getSaveFileName(
      this, tr("导出规则"),
      QDir::current().filePath("etc"),
      tr("配置文件 (*.conf);;所有文件 (*.*)"));

  if (!path.isEmpty()) {
    QFile file(path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      QTextStream stream(&file);
      stream << textEdit->toPlainText();
      file.close();
      QMessageBox::information(this, tr("成功"), tr("规则已导出"));
    } else {
      QMessageBox::critical(this, tr("错误"), tr("无法保存文件"));
    }
  }
}