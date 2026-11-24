
#ifndef OSIMX_AUTH_LAYOUT_H
#define OSIMX_AUTH_LAYOUT_H

#include <QColor>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QLayout>
#include <QPixmap>
#include <QScrollArea>
#include <QSizePolicy>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

namespace AuthLayout {

inline QLabel* makeText(const QString& text,
                        const QString& objectName)
{
    auto* label = new QLabel(text);
    label->setObjectName(objectName);
    label->setTextFormat(Qt::PlainText);
    label->setWordWrap(true);
    label->setMargin(0);

    // Preferred height follows the wrapped text.
    // AlignTop prevents labels from spreading vertically.
    label->setSizePolicy(
        QSizePolicy::Preferred,
        QSizePolicy::Minimum
    );

    return label;
}

inline void addFeature(QVBoxLayout* layout,
                       const QString& number,
                       const QString& title,
                       const QString& description)
{
    auto* card = new QFrame;
    card->setObjectName("featureSection");
    card->setSizePolicy(
        QSizePolicy::Preferred,
        QSizePolicy::Maximum
    );

    auto* content = new QVBoxLayout(card);
    content->setContentsMargins(22, 18, 22, 18);
    content->setSpacing(7);
    content->setAlignment(Qt::AlignTop);

    content->addWidget(makeText(number, "featureNumber"));
    content->addWidget(makeText(title, "featureTitle"));
    content->addWidget(makeText(description, "featureBody"));

    layout->addWidget(card);
    layout->addSpacing(8);
}

inline QVBoxLayout* create(QWidget* window,
                           int cardMaximumWidth = 540)
{
    window->setObjectName("authWindow");
    window->setMinimumSize(980, 640);
    window->setWindowIcon(QIcon(":/resources/logo.png"));

    window->setStyleSheet(R"QSS(
        QWidget#authWindow {
            background-color: #101A16;
            color: #F5F7F2;
            font-family: "Segoe UI";
            font-size: 13px;
        }

        QScrollArea#introScroll,
        QWidget#introContent {
            background-color: #F3F4EE;
            border: none;
        }

        QScrollArea#formScroll,
        QWidget#formContent {
            background-color: #101A16;
            border: none;
        }

        QLabel#brandName {
            color: #172F22;
            font-size: 43px;
            font-weight: 800;
        }

        QLabel#brandSubtitle {
            color: #53675A;
            font-size: 14px;
        }

        QLabel#introEyebrow {
            color: #356B4A;
            font-size: 11px;
            font-weight: 700;
        }

        QLabel#introQuote {
            color: #172F22;
            font-size: 23px;
            font-weight: 700;
        }

        QLabel#introBody {
            color: #4D6254;
            font-size: 14px;
        }

        QFrame#introDivider {
            background-color: #CCD8CA;
            border: none;
            min-height: 1px;
            max-height: 1px;
        }

        QFrame#featureSection {
            background-color: #E5EDE2;
            border: 1px solid #D3E0D1;
            border-radius: 12px;
        }

        QFrame#featureSection QLabel {
            background: transparent;
        }

        QLabel#featureNumber {
            color: #356B4A;
            font-size: 11px;
            font-weight: 700;
        }

        QLabel#featureTitle {
            color: #193729;
            font-size: 16px;
            font-weight: 700;
        }

        QLabel#featureBody {
            color: #4D6254;
            font-size: 13px;
        }

        QLabel#introFooter {
            color: #66796B;
            font-size: 12px;
        }

        QLabel#rightBrand {
            color: #F4F7F1;
            font-size: 17px;
            font-weight: 700;
        }

        QLabel#rightBrandSubtitle {
            color: #A9C5AC;
            font-size: 11px;
        }

        QLabel#rightFooter {
            color: #A9C5AC;
            font-size: 11px;
        }

        QFrame#rightAccent {
            background-color: #A5C7A5;
            border: none;
            border-radius: 3px;
        }

        QFrame#authCard {
            background-color: #1A3829;
            border: 1px solid #365B44;
            border-radius: 20px;
        }

        QFrame#authCard QLabel {
            background: transparent;
        }

        QLabel#authEyebrow {
            color: #B2D5B3;
            font-size: 11px;
            font-weight: 700;
        }

        QLabel#authTitle {
            color: #FFFFFF;
            font-size: 33px;
            font-weight: 700;
        }

        QLabel#authSubtitle {
            color: #D1E1D2;
            font-size: 14px;
        }

        QLabel#fieldLabel {
            color: #F1F7EF;
            font-size: 12px;
            font-weight: 700;
        }

        QLabel#statusLabel {
            color: #D1E1D2;
            font-size: 12px;
        }

        QLabel#formHint {
            color: #B6CDB9;
            font-size: 11px;
        }

        QLabel#authFooterText {
            color: #D1E1D2;
            font-size: 13px;
        }

        QLineEdit#authField {
            background-color: #F7F9F5;
            color: #193729;
            border: 1px solid #C9D8C9;
            border-radius: 9px;
            padding: 11px 13px;
            min-height: 27px;
            font-size: 14px;
            selection-background-color: #356B4A;
            selection-color: #FFFFFF;
        }

        QLineEdit#authField:hover {
            border-color: #9EBDA3;
        }

        QLineEdit#authField:focus {
            background-color: #FFFFFF;
            border: 2px solid #A5C7A5;
        }

        QPushButton#primaryButton {
            background-color: #A5C7A5;
            color: #172F22;
            border: none;
            border-radius: 9px;
            padding: 11px 15px;
            min-height: 28px;
            font-size: 14px;
            font-weight: 700;
        }

        QPushButton#primaryButton:hover {
            background-color: #BEDBBE;
        }

        QPushButton#primaryButton:pressed {
            background-color: #8CB58F;
        }

        QPushButton#primaryButton:disabled {
            background-color: #718D78;
            color: #E0E9DF;
        }

        QPushButton#linkButton {
            background: transparent;
            color: #BDE0BD;
            border: none;
            padding: 4px;
            font-weight: 700;
        }

        QPushButton#linkButton:hover {
            color: #FFFFFF;
            text-decoration: underline;
        }

        QCheckBox {
            background: transparent;
            color: #D1E1D2;
            spacing: 8px;
        }

        QCheckBox::indicator {
            width: 16px;
            height: 16px;
            border: 1px solid #9AB49E;
            border-radius: 4px;
            background-color: #F7F9F5;
        }

        QCheckBox::indicator:checked {
            background-color: #A5C7A5;
            border-color: #A5C7A5;
        }

        QScrollArea {
            border: none;
        }

        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 4px 2px;
        }

        QScrollBar::handle:vertical {
            background: #9FB8A1;
            min-height: 32px;
            border-radius: 3px;
        }

        QScrollBar::handle:vertical:hover {
            background: #7FA184;
        }

        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0px;
            border: none;
        }

        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical {
            background: transparent;
        }
    )QSS");

    auto* root = new QHBoxLayout(window);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // =====================================================
    // LEFT PANEL
    // =====================================================

    auto* introScroll = new QScrollArea;
    introScroll->setObjectName("introScroll");
    introScroll->setWidgetResizable(true);
    introScroll->setFrameShape(QFrame::NoFrame);
    introScroll->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff
    );

    auto* introContent = new QWidget;
    introContent->setObjectName("introContent");

    auto* intro = new QVBoxLayout(introContent);

    // Keep all content together at the top.
    intro->setContentsMargins(48, 38, 48, 38);
    intro->setSpacing(8);
    intro->setAlignment(Qt::AlignTop);
    intro->setSizeConstraint(QLayout::SetMinimumSize);

    auto* logo = new QLabel;
    logo->setObjectName("introLogo");
    logo->setAlignment(Qt::AlignLeft);
    logo->setSizePolicy(
        QSizePolicy::Preferred,
        QSizePolicy::Fixed
    );

    const QPixmap logoPixmap(":/resources/logo.png");

    if (!logoPixmap.isNull()) {
        logo->setPixmap(logoPixmap.scaled(
            82, 82,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
        ));
    }

    intro->addWidget(logo);
    intro->addSpacing(12);

    intro->addWidget(makeText("OSimX", "brandName"));
    intro->addWidget(makeText(
        "Operating System Simulator",
        "brandSubtitle"
    ));

    intro->addSpacing(14);

    auto* divider = new QFrame;
    divider->setObjectName("introDivider");
    divider->setFixedHeight(1);
    intro->addWidget(divider);

    intro->addSpacing(14);

    intro->addWidget(makeText(
        "Operating systems, made easier to understand.",
        "introQuote"
    ));

    intro->addSpacing(8);

    intro->addWidget(makeText(
        "Explore how operating systems schedule processes, "
        "manage memory, handle disk requests, coordinate "
        "resources, and organize files. Run algorithms with "
        "your own inputs and examine the results through "
        "interactive simulations.",
        "introBody"
    ));

    intro->addSpacing(18);

    intro->addWidget(makeText(
        "WHAT YOU CAN EXPLORE",
        "introEyebrow"
    ));

    intro->addSpacing(8);

    addFeature(
        intro,
        "01 / CPU SCHEDULING",
        "Understand process execution",
        "Run FCFS, SJF, SRTF, Round Robin, and Priority "
        "Scheduling. Explore execution order, waiting "
        "time, turnaround time, and scheduling results."
    );

    addFeature(
        intro,
        "02 / DISK SCHEDULING",
        "Explore disk-head movement",
        "Compare FCFS, SSTF, SCAN, C-SCAN, LOOK, and "
        "C-LOOK. Follow the order of disk requests "
        "and visualize head movement."
    );

    addFeature(
        intro,
        "03 / MEMORY MANAGEMENT",
        "See how memory is managed",
        "Explore First Fit, Best Fit, and Worst Fit "
        "allocation, alongside FIFO, LRU, and Optimal "
        "page replacement. Examine memory usage "
        "and page faults."
    );

    addFeature(
        intro,
        "04 / DEADLOCK",
        "Analyze resource allocation",
        "Work with Banker's Algorithm, safe sequences, "
        "resource requests, and deadlock detection "
        "to understand how processes share resources."
    );

    addFeature(
        intro,
        "05 / FILE SYSTEM",
        "Explore file organization",
        "Create files and directories, inspect disk "
        "blocks, and explore contiguous, linked, "
        "and indexed file allocation."
    );

    addFeature(
        intro,
        "06 / SIMULATION HISTORY",
        "Return to previous work",
        "Review saved simulations and revisit "
        "the results of earlier experiments."
    );

    addFeature(
        intro,
        "07 / ALGORITHM COMPARISON",
        "Compare different approaches",
        "Examine supported algorithm comparisons "
        "and revisit previously saved results."
    );

    addFeature(
        intro,
        "08 / USER ACCOUNTS",
        "Your OSimX workspace",
        "Create an account, sign in, and access "
        "your saved simulations and comparisons."
    );

    intro->addSpacing(14);

    auto* bottomDivider = new QFrame;
    bottomDivider->setObjectName("introDivider");
    bottomDivider->setFixedHeight(1);
    intro->addWidget(bottomDivider);

    intro->addSpacing(10);

    intro->addWidget(makeText(
        "Built to make Operating Systems easier to understand.",
        "introFooter"
    ));

    // No stretch inside the left panel.
    // Content stays at the top and scrolls naturally.
    introScroll->setWidget(introContent);

    // =====================================================
    // RIGHT PANEL
    // =====================================================

    auto* formScroll = new QScrollArea;
    formScroll->setObjectName("formScroll");
    formScroll->setWidgetResizable(true);
    formScroll->setFrameShape(QFrame::NoFrame);
    formScroll->setHorizontalScrollBarPolicy(
        Qt::ScrollBarAlwaysOff
    );

    auto* formContent = new QWidget;
    formContent->setObjectName("formContent");

    auto* formOuter = new QVBoxLayout(formContent);
    formOuter->setContentsMargins(42, 32, 42, 30);
    formOuter->setSpacing(0);

    auto* header = new QHBoxLayout;
    header->setSpacing(12);

    auto* accent = new QFrame;
    accent->setObjectName("rightAccent");
    accent->setFixedSize(4, 38);

    auto* brandColumn = new QVBoxLayout;
    brandColumn->setSpacing(2);

    brandColumn->addWidget(makeText(
        "OSimX", "rightBrand"
    ));

    brandColumn->addWidget(makeText(
        "OPERATING SYSTEM SIMULATOR",
        "rightBrandSubtitle"
    ));

    header->addWidget(accent);
    header->addLayout(brandColumn);
    header->addStretch();

    formOuter->addLayout(header);
    formOuter->addStretch(1);

    auto* card = new QFrame;
    card->setObjectName("authCard");
    card->setMaximumWidth(cardMaximumWidth);
    card->setMinimumWidth(0);

    card->setSizePolicy(
        QSizePolicy::Expanding,
        QSizePolicy::Maximum
    );

    auto* shadow = new QGraphicsDropShadowEffect(card);
    shadow->setBlurRadius(30);
    shadow->setOffset(0, 12);
    shadow->setColor(QColor(0, 0, 0, 65));
    card->setGraphicsEffect(shadow);

    auto* cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(42, 38, 42, 38);
    cardLayout->setSpacing(11);

    formOuter->addWidget(card, 0, Qt::AlignHCenter);
    formOuter->addStretch(1);

    auto* footer = makeText(
        "EXPLORE  /  SIMULATE  /  UNDERSTAND",
        "rightFooter"
    );

    footer->setAlignment(Qt::AlignCenter);
    formOuter->addWidget(footer);

    formScroll->setWidget(formContent);

    root->addWidget(introScroll, 42);
    root->addWidget(formScroll, 58);

    root->setStretch(0, 42);
    root->setStretch(1, 58);

    return cardLayout;
}

} // namespace AuthLayout

#endif
