#ifndef OSIMX_MAIN_WINDOW_H
#define OSIMX_MAIN_WINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <functional>

class QLabel;
class QTableWidget;
class SchedulingWindow;
class DiskSchedulingWindow;
class MemoryWindow;
class FileSystemWindow;
class DeadlockWindow;

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    void refreshSession();

signals:
    void logoutRequested();

private:
    QStackedWidget* pages = nullptr;
    QWidget* hubPage = nullptr;
    SchedulingWindow* cpuPage = nullptr;
    DiskSchedulingWindow* diskPage = nullptr;
    MemoryWindow* memoryPage = nullptr;
    FileSystemWindow* fileSystemPage = nullptr;
    DeadlockWindow* deadlockPage = nullptr;
    QTableWidget* historyTable = nullptr;
    QLabel* historyStatus = nullptr;
    QTableWidget* comparisonTable = nullptr;
    QLabel* comparisonStatus = nullptr;
    QTableWidget* loginTable = nullptr;
    QLabel* loginStatus = nullptr;

    QWidget* createHubPage();
    QWidget* createSimulationCard(const QString& icon,
                                  const QString& title,
                                  const QString& description,
                                  const QString& status,
                                  bool enabled,
                                  const std::function<void()>& onClicked);
    void showHub();
    void reloadHistory();
    void reloadComparisons();
    void reloadLoginHistory();
    void viewSelectedSimulation();
    void removeSelectedSimulation();
    void viewSelectedComparison();
    void removeSelectedComparison();
};

#endif
