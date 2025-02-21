#include "RuleHighlighter.h"

RuleHighlighter::RuleHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
  // 注释格式（灰色斜体）
  commentFormat.setForeground(Qt::gray);
  commentFormat.setFontItalic(true);
  highlightingRules.append({
      QRegularExpression("#[^\n]*"),
      commentFormat
  });

  // 章节格式（蓝色加粗）
  sectionFormat.setForeground(Qt::blue);
  sectionFormat.setFontWeight(QFont::Bold);
  highlightingRules.append({
      QRegularExpression("\\[[^\\]]+\\]"),
      sectionFormat
  });

  // 键格式（深绿色）
  keyFormat.setForeground(QColor(0, 128, 0));
  highlightingRules.append({
      QRegularExpression("^\\s*[a-zA-Z_]+(?=\\s*=)"),
      keyFormat
  });

  // 值格式（深红色）
  valueFormat.setForeground(Qt::darkRed);
  highlightingRules.append({
      QRegularExpression("=\\s*\\K[^#\n]+(?=\\s*(#|$))"),
      valueFormat
  });
}

void RuleHighlighter::highlightBlock(const QString& text) {
  for (const auto& rule : highlightingRules) {
    QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
    while (matchIterator.hasNext()) {
      QRegularExpressionMatch match = matchIterator.next();
      setFormat(match.capturedStart(), match.capturedLength(), rule.format);
    }
  }
}