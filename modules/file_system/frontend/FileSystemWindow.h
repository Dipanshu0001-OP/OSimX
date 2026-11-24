
#ifndef OSIMX_FILE_SYSTEM_WINDOW_H
#define OSIMX_FILE_SYSTEM_WINDOW_H

#include <QWidget>
#include <QJsonObject>
#include <QString>

#include "modules/file_system/backend/FileSystem.h"

class QLabel;
class QListWidget;
class QTableWidget;
class QTreeWidget;
class QComboBox;
class QPushButton;
class QProgressBar;
class QTreeWidgetItem;

class FileSystemWindow : public QWidget
{
    Q_OBJECT

public:
    explicit FileSystemWindow(QWidget* parent = nullptr);

    // Restore a previously saved filesystem simulation.
    // Returns false if the saved data is invalid.
    bool restoreSimulation(
        const QJsonObject& inputData,
        const QJsonObject& outputData
    );

signals:
    void backRequested();

    // Notify MainWindow to refresh Saved Simulations.
    void simulationSaved();

private:
    // Virtual filesystem backend
    FileSystem m_fileSystem;

    // File explorer
    QTreeWidget* m_tree = nullptr;

    // Disk allocation visualization
    QTableWidget* m_blocks = nullptr;

    // Simulation operation history
    QListWidget* m_log = nullptr;

    // Allocation algorithm selection
    QComboBox* m_allocation = nullptr;

    // Labels
    QLabel* m_pathLabel = nullptr;
    QLabel* m_statsLabel = nullptr;
    QLabel* m_statusLabel = nullptr;

    QLabel* m_usedValue = nullptr;
    QLabel* m_freeValue = nullptr;
    QLabel* m_usageValue = nullptr;

    QLabel* m_selectionInfo = nullptr;
    QLabel* m_allocationHelp = nullptr;

    // Disk usage indicator
    QProgressBar* m_usageBar = nullptr;

    // File operation buttons
    QPushButton* m_readButton = nullptr;
    QPushButton* m_writeButton = nullptr;
    QPushButton* m_deleteButton = nullptr;

    // Save simulation button
    QPushButton* m_saveButton = nullptr;

    // Refresh UI
    void refresh();
    void refreshTree();
    void refreshBlocks();

    // Display operation results
    void appendResult(const FileSystemResult& result);

    // Explorer helpers
    QString selectedPath() const;
    void updateSelection();
    void selectPath(const QString& path);
    void openSelected();

    // File operations
    void createFile();
    void createDirectory();
    void readFile();
    void writeFile();
    void deleteSelected();

    // Directory navigation
    void changeDirectory();
    void goUp();

    // Save and restore
    void saveSimulation();

    QJsonObject captureSimulationInput() const;
    QJsonObject captureSimulationOutput() const;

    bool restoreFileSystemSnapshot(
        const QJsonObject& snapshot,
        QString& errorMessage
    );

    QJsonObject serializeFileSystemSnapshot() const;
};

#endif // OSIMX_FILE_SYSTEM_WINDOW_H
