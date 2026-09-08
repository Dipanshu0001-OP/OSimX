#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    
    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    
    QVBoxLayout* layout = new QVBoxLayout(central);
    layout->setContentsMargins(5, 5, 5, 5);
    
    tabWidget = new QTabWidget(this);
    tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #ccc; }"
        "QTabBar::tab { padding: 8px 20px; font-weight: bold; }"
        "QTabBar::tab:selected { background-color: #2196F3; color: white; }"
    );
    
    // Scheduling Tab
    schedulingWidget = new SchedulingWidget(this);
    tabWidget->addTab(schedulingWidget, "CPU Scheduling");
    
    layout->addWidget(tabWidget);
    
    setWindowTitle("OSimX - Operating System Simulator");
    resize(1200, 800);
}

MainWindow::~MainWindow() {}