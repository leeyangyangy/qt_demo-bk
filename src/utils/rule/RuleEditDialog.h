#ifndef RULEEDITDIALOG_H
#define RULEEDITDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QTextEdit>

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
  QTextEdit* textEdit;
  QDialogButtonBox* buttonBox;
  RuleHighlighter* highlighter;
};
#endif  // RULEEDITDIALOG_H
