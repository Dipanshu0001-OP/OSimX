#ifndef LOADINGWINDOW_H
#define LOADINGWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QVBoxLayout>

class LoadingWindow : public QWidget
{
    Q_OBJECT

public:
    explicit LoadingWindow(QWidget *parent = nullptr);

signals:
    void loadingFinished(bool isLoggedIn);

private slots:
    void updateProgress();

private:
    QLabel *logoLabel;
    QProgressBar *progressBar;
    QTimer *timer;
    int progressValue = 0;

    void setupUI();
    void checkLoginStatus();
};

#endif // LOADINGWINDOW_H