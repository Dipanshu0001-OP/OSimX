#ifndef OSIMX_MAIN_WINDOW_H
#define OSIMX_MAIN_WINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <functional>

class QLabel;
class SchedulingWindow;
class DiskSchedulingWindow;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);

signals:
    void logoutRequested();

private:
    QStackedWidget* pages = nullptr;
    QWidget* hubPage = nullptr;
    SchedulingWindow* cpuPage = nullptr;
    DiskSchedulingWindow* diskPage = nullptr;

    QWidget* createHubPage();
    QWidget* createSimulationCard(const QString& icon,
                                  const QString& title,
                                  const QString& description,
                                  const QString& status,
                                  bool enabled,
                                  const std::function<void()>& onClicked);
    void showHub();
};

#endif
