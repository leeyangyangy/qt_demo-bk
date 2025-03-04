#ifndef RULEEDITDIALOG_H
#define RULEEDITDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <memory>

class RuleHighlighter;

class RuleEditDialog : public QDialog {
  Q_OBJECT
 public:
  explicit RuleEditDialog(const QString& content, QWidget* parent = nullptr);
  QString getEditedContent() const;

 private slots:
  void validateAndAccept();
  void importRules();
  void exportRules();

 private:
  QVBoxLayout* layout;
  QTextEdit* textEdit;
  QDialogButtonBox* buttonBox;
  RuleHighlighter* highlighter;
  std::vector<std::unique_ptr<QPushButton>> buttons;
};
#endif  // RULEEDITDIALOG_H
