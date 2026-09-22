#ifndef OSIMX_SIMULATION_LOG_WIDGET_H
#define OSIMX_SIMULATION_LOG_WIDGET_H

#include <QPlainTextEdit>
#include <QStringList>

class SimulationLogWidget : public QPlainTextEdit {
    Q_OBJECT
public:
    explicit SimulationLogWidget(QWidget* parent = nullptr);
    void setLogs(const QStringList& logs);
    void revealUpTo(int step);
    void clearLogs();

private:
    QStringList m_logs;
};

#endif
