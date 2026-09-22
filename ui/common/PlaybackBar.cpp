#include "PlaybackBar.h"
#include <algorithm>
#include <QFont>
#include <QHBoxLayout>

PlaybackBar::PlaybackBar(QWidget* parent) : QWidget(parent)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 6, 0, 6);
    layout->setSpacing(7);

    resetButton = new QPushButton("↺");
    previousButton = new QPushButton("‹");
    playButton = new QPushButton("▶");
    nextButton = new QPushButton("›");
    for (auto* button : {resetButton, previousButton, playButton, nextButton}) {
        button->setFixedSize(42, 38);
        button->setCursor(Qt::PointingHandCursor);
        button->setAccessibleName(button->text());
    }

    progress = new QSlider(Qt::Horizontal);
    progress->setRange(0, 0);
    progress->setMinimumWidth(220);
    progress->setCursor(Qt::PointingHandCursor);

    stepLabel = new QLabel("Step 0 / 0");
    stepLabel->setMinimumWidth(82);

    speed = new QComboBox();
    speed->addItem("0.5×", 1200);
    speed->addItem("1×", 650);
    speed->addItem("2×", 320);
    speed->setCurrentIndex(1);

    layout->addWidget(resetButton);
    layout->addWidget(previousButton);
    layout->addWidget(playButton);
    layout->addWidget(nextButton);
    layout->addWidget(progress, 1);
    layout->addWidget(stepLabel);
    layout->addWidget(new QLabel("Speed"));
    layout->addWidget(speed);

    connect(resetButton, &QPushButton::clicked, this, [this] {
        setPlaying(false);
        setIndex(0);
        emit reset();
    });
    connect(previousButton, &QPushButton::clicked, this, [this] {
        setPlaying(false);
        stepBackward();
        emit previous();
    });
    connect(nextButton, &QPushButton::clicked, this, [this] {
        stepForward();
        emit next();
    });
    connect(playButton, &QPushButton::clicked, this, [this] {
        setPlaying(!m_playing);
        emit m_playing ? play() : pause();
    });
    connect(progress, &QSlider::valueChanged, this, [this](int value) {
        setIndex(value);
    });
    connect(&timer, &QTimer::timeout, this, [this] {
        if (m_index >= m_total - 1) {
            setPlaying(false);
            return;
        }
        stepForward();
        emit next();
    });
    connect(speed, &QComboBox::currentIndexChanged, this, [this] {
        if (m_playing) timer.start(speed->currentData().toInt());
    });
}

void PlaybackBar::setTotal(int total)
{
    m_total = std::max(0, total);
    progress->setRange(0, std::max(0, m_total - 1));
    setIndex(0);
    setPlaying(false);
}

void PlaybackBar::setIndex(int index)
{
    m_index = qBound(0, index, qMax(0, m_total - 1));
    progress->blockSignals(true);
    progress->setValue(m_index);
    progress->blockSignals(false);
    stepLabel->setText(QString("Step %1 / %2").arg(m_total ? m_index + 1 : 0).arg(m_total));
    if (m_total && m_index == m_total - 1)
        setPlaying(false);
    emit indexChanged(m_index);
}

void PlaybackBar::startPlayback()
{
    if (m_total <= 1) {
        setPlaying(false);
        return;
    }

    setIndex(0);
    setPlaying(true);
}

void PlaybackBar::setPlaying(bool playing)
{
    m_playing = playing && m_total > 1;
    if (m_playing)
        timer.start(speed->currentData().toInt());
    else
        timer.stop();
    playButton->setText(m_playing ? "Ⅱ" : "▶");
}

void PlaybackBar::stepForward()
{
    setIndex(m_index + 1);
}

void PlaybackBar::stepBackward()
{
    setIndex(m_index - 1);
}
