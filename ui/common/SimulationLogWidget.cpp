#include "SimulationLogWidget.h"
#include <QFontDatabase>
#include <QTextCursor>
#include <QFont>

SimulationLogWidget::SimulationLogWidget(QWidget* parent) : QPlainTextEdit(parent)
{
    setReadOnly(true);
    setMinimumHeight(220);
    setLineWrapMode(QPlainTextEdit::WidgetWidth);
    setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
}

void SimulationLogWidget::setLogs(const QStringList& logs)
{
    m_logs = logs;
    revealUpTo(m_logs.isEmpty() ? -1 : 0);
}

void SimulationLogWidget::revealUpTo(int step)
{
    if (m_logs.isEmpty()) {
        clear();
        return;
    }

    const int last = qBound(0, step, m_logs.size() - 1);
    QStringList visible;
    for (int i = 0; i <= last; ++i)
        visible << QString("%1  %2").arg(i == last ? "▶" : "✓").arg(m_logs[i]);
    setPlainText(visible.join('\n'));
    moveCursor(QTextCursor::End);
}

void SimulationLogWidget::clearLogs()
{
    m_logs.clear();
    clear();
}
