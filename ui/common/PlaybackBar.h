#ifndef OSIMX_PLAYBACK_BAR_H
#define OSIMX_PLAYBACK_BAR_H

#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QTimer>
#include <QWidget>

class PlaybackBar : public QWidget {
    Q_OBJECT
public:
    explicit PlaybackBar(QWidget* parent = nullptr);
    void setTotal(int total);
    void setIndex(int index);
    void startPlayback();
    int index() const { return m_index; }
    int total() const { return m_total; }

signals:
    void play();
    void pause();
    void reset();
    void previous();
    void next();
    void indexChanged(int index);

private:
    QPushButton* resetButton;
    QPushButton* previousButton;
    QPushButton* playButton;
    QPushButton* nextButton;
    QSlider* progress;
    QComboBox* speed;
    QLabel* stepLabel;
    QTimer timer;
    int m_total = 0;
    int m_index = 0;
    bool m_playing = false;

    void setPlaying(bool playing);
    void stepForward();
    void stepBackward();
};

#endif
