#ifndef RULEHIGHLIGHTER_H
#define RULEHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QRegularExpression>

class RuleHighlighter : public QSyntaxHighlighter {
  Q_OBJECT
public:
  explicit RuleHighlighter(QTextDocument* parent = nullptr);

protected:
  void highlightBlock(const QString& text) override;

private:
  struct HighlightRule {
    QRegularExpression pattern;
    QTextCharFormat format;
  };
  QVector<HighlightRule> highlightingRules;

  QTextCharFormat commentFormat;
  QTextCharFormat keyFormat;
  QTextCharFormat valueFormat;
  QTextCharFormat sectionFormat;
};

#endif //RULEHIGHLIGHTER_H
