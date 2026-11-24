
#include "FileSystemWindow.h"
#include "database/SimulationRepository.h"

#include <QByteArray>
#include <QColor>
#include <QLineEdit>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonObject>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QScrollArea>
#include <QSignalBlocker>
#include <QSplitter>
#include <QTableWidget>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <algorithm>
#include <functional>
#include <limits>

namespace {

QString qs(const std::string& value)
{
    return QString::fromStdString(value);
}

QFrame* makePanel()
{
    auto* frame = new QFrame;
    frame->setObjectName("fsPanel");
    return frame;
}

QLabel* makeLabel(const QString& value)
{
    auto* label = new QLabel(value);
    label->setWordWrap(true);
    return label;
}

QJsonArray intArray(const std::vector<int>& values)
{
    QJsonArray result;

    for (int value : values)
        result.append(value);

    return result;
}

QJsonArray stringArray(
    const std::vector<std::string>& values
)
{
    QJsonArray result;

    for (const auto& value : values)
        result.append(qs(value));

    return result;
}

bool parseIntArray(
    const QJsonValue& value,
    std::vector<int>& result
)
{
    if (!value.isArray())
        return false;

    result.clear();

    for (const auto& entry : value.toArray()) {
        if (!entry.isDouble())
            return false;

        const double number = entry.toDouble();

        if (number < std::numeric_limits<int>::min() ||
            number > std::numeric_limits<int>::max() ||
            number != static_cast<int>(number))
            return false;

        result.push_back(static_cast<int>(number));
    }

    return true;
}

bool parseStringArray(
    const QJsonValue& value,
    std::vector<std::string>& result
)
{
    if (!value.isArray())
        return false;

    result.clear();

    for (const auto& entry : value.toArray()) {
        if (!entry.isString())
            return false;

        result.push_back(entry.toString().toStdString());
    }

    return true;
}

QJsonObject snapshotToJson(
    const FileSystemSnapshot& snapshot
)
{
    QJsonObject json;

    json["version"] = snapshot.version;
    json["totalBlocks"] =
        static_cast<qint64>(snapshot.totalBlocks);
    json["blockSizeBytes"] =
        static_cast<qint64>(snapshot.blockSizeBytes);
    json["currentDirectory"] =
        qs(snapshot.currentDirectory);

    json["directories"] =
        stringArray(snapshot.directories);

    QJsonArray blocks;

    for (const auto& block : snapshot.blocks) {
        QJsonObject item;

        item["index"] = block.index;
        item["allocated"] = block.allocated;
        item["isIndexBlock"] = block.isIndexBlock;
        item["ownerPath"] = qs(block.ownerPath);
        item["nextBlock"] = block.nextBlock;
        item["indexEntries"] =
            intArray(block.indexEntries);

        blocks.append(item);
    }

    json["blocks"] = blocks;

    QJsonArray files;

    for (const auto& file : snapshot.files) {
        const auto& meta = file.metadata;

        QJsonObject item;

        item["name"] = qs(meta.name);
        item["path"] = qs(meta.path);
        item["sizeBytes"] =
            static_cast<qint64>(meta.sizeBytes);
        item["dataBlockCount"] =
            static_cast<qint64>(meta.dataBlockCount);
        item["allocationMethod"] =
            static_cast<int>(meta.allocationMethod);
        item["dataBlocks"] = intArray(meta.dataBlocks);
        item["indexBlock"] = meta.indexBlock;
        item["createdAt"] = qs(meta.createdAt);
        item["modifiedAt"] = qs(meta.modifiedAt);

        // Base64 preserves arbitrary file bytes.
        item["contentBase64"] =
            QString::fromLatin1(
                QByteArray(
                    file.content.data(),
                    static_cast<qsizetype>(
                        file.content.size()
                    )
                ).toBase64()
            );

        QJsonObject permissions;
        permissions["read"] = meta.permissions.read;
        permissions["write"] = meta.permissions.write;
        permissions["execute"] = meta.permissions.execute;

        item["permissions"] = permissions;

        files.append(item);
    }

    json["files"] = files;

    return json;
}

bool jsonToSnapshot(
    const QJsonObject& json,
    FileSystemSnapshot& snapshot,
    QString& error
)
{
    auto fail = [&](const QString& message) {
        error = message;
        return false;
    };

    if (json.value("version").toInt(-1) != 1)
        return fail("Unsupported snapshot version.");

    const qint64 total =
        json.value("totalBlocks").toInteger(-1);

    const qint64 size =
        json.value("blockSizeBytes").toInteger(-1);

    if (total <= 0 || total > 100000 ||
        size <= 0 || size > 10000000)
        return fail("Invalid disk geometry.");

    if (!json.value("currentDirectory").isString())
        return fail("Missing current directory.");

    snapshot = FileSystemSnapshot{};

    snapshot.version = 1;
    snapshot.totalBlocks =
        static_cast<std::size_t>(total);
    snapshot.blockSizeBytes =
        static_cast<std::size_t>(size);
    snapshot.currentDirectory =
        json.value("currentDirectory")
            .toString().toStdString();

    if (!parseStringArray(
            json.value("directories"),
            snapshot.directories))
        return fail("Invalid directory list.");

    if (!json.value("blocks").isArray() ||
        !json.value("files").isArray())
        return fail("Missing block or file data.");

    const QJsonArray blocks =
        json.value("blocks").toArray();

    if (blocks.size() != total)
        return fail("Block count mismatch.");

    for (const auto& value : blocks) {
        if (!value.isObject())
            return fail("Invalid disk block.");

        const QJsonObject item = value.toObject();

        DiskBlockInfo block;

        block.index = item.value("index").toInt(-1);
        block.allocated =
            item.value("allocated").toBool();
        block.isIndexBlock =
            item.value("isIndexBlock").toBool();
        block.ownerPath =
            item.value("ownerPath")
                .toString().toStdString();
        block.nextBlock =
            item.value("nextBlock").toInt(-1);

        if (!parseIntArray(
                item.value("indexEntries"),
                block.indexEntries))
            return fail("Invalid index block entries.");

        snapshot.blocks.push_back(block);
    }

    for (const auto& value :
         json.value("files").toArray()) {

        if (!value.isObject())
            return fail("Invalid file record.");

        const QJsonObject item = value.toObject();

        FileSystemSnapshotFile file;
        FileMetadata& meta = file.metadata;

        meta.name =
            item.value("name").toString().toStdString();
        meta.path =
            item.value("path").toString().toStdString();

        const qint64 fileSize =
            item.value("sizeBytes").toInteger(-1);

        const qint64 blockCount =
            item.value("dataBlockCount").toInteger(-1);

        if (fileSize < 0 || blockCount < 0)
            return fail("Invalid file size.");

        meta.sizeBytes =
            static_cast<std::size_t>(fileSize);
        meta.dataBlockCount =
            static_cast<std::size_t>(blockCount);

        const int method =
            item.value("allocationMethod").toInt(-1);

        if (method < 0 || method > 2)
            return fail("Invalid allocation method.");

        meta.allocationMethod =
            static_cast<AllocationMethod>(method);

        if (!parseIntArray(
                item.value("dataBlocks"),
                meta.dataBlocks))
            return fail("Invalid data block list.");

        meta.indexBlock =
            item.value("indexBlock").toInt(-1);

        meta.createdAt =
            item.value("createdAt")
                .toString().toStdString();

        meta.modifiedAt =
            item.value("modifiedAt")
                .toString().toStdString();

        const QJsonObject permissions =
            item.value("permissions").toObject();

        meta.permissions.read =
            permissions.value("read").toBool(true);

        meta.permissions.write =
            permissions.value("write").toBool(true);

        meta.permissions.execute =
            permissions.value("execute").toBool(false);

        if (!item.value("contentBase64").isString())
            return fail("Missing file content.");

        const QByteArray encoded =
            item.value("contentBase64")
                .toString().toLatin1();

        const QByteArray decoded =
            QByteArray::fromBase64(
                encoded,
                QByteArray::AbortOnBase64DecodingErrors
            );

        if (decoded.isNull() && !encoded.isEmpty())
            return fail("Invalid Base64 file content.");

        file.content.assign(
            decoded.constData(),
            static_cast<std::size_t>(decoded.size())
        );

        snapshot.files.push_back(std::move(file));
    }

    error.clear();
    return true;
}

} // namespace

FileSystemWindow::FileSystemWindow(QWidget* parent)
    : QWidget(parent),
      m_fileSystem(100, 100)
{
    setObjectName("fileSystemWindow");

    setStyleSheet(R"QSS(
        QWidget#fileSystemWindow {
            background: #F3F4EE;
            color: #193729;
        }

        QWidget#fileSystemWindow QLabel {
            color: #193729;
        }

        QWidget#fileSystemWindow QFrame#fsPanel {
            background: white;
            border: 1px solid #DCE6D9;
            border-radius: 14px;
        }

        QWidget#fileSystemWindow QPushButton {
            background: white;
            color: #28583B;
            border: 1px solid #C7D8C7;
            border-radius: 9px;
            padding: 10px 15px;
            font-weight: 700;
        }

        QWidget#fileSystemWindow QPushButton:hover {
            background: #E8F1E6;
        }

        QWidget#fileSystemWindow QPushButton#fsPrimary {
            background: #356B4A;
            color: white;
            border-color: #356B4A;
        }

        QWidget#fileSystemWindow QPushButton#fsPrimary:hover {
            background: #28583B;
        }

        QWidget#fileSystemWindow QTreeWidget,
        QWidget#fileSystemWindow QTableWidget,
        QWidget#fileSystemWindow QListWidget {
            background: white;
            color: #193729;
            border: 1px solid #DCE6D9;
            border-radius: 8px;
            alternate-background-color: #F6F9F4;
            selection-background-color: #DCEBDD;
            selection-color: #193729;
        }

        QWidget#fileSystemWindow QHeaderView::section {
            background: #E8EFE5;
            color: #365843;
            padding: 8px;
            border: none;
            font-weight: 700;
        }

        QWidget#fileSystemWindow QComboBox {
            background: white;
            color: #193729;
            border: 1px solid #C8D7C8;
            border-radius: 8px;
            padding: 8px;
        }

        QWidget#fileSystemWindow QProgressBar {
            border: 1px solid #DCE6D9;
            border-radius: 7px;
            background: #E8EFE5;
            text-align: center;
            color: #193729;
        }

        QWidget#fileSystemWindow QProgressBar::chunk {
            background: #5E9770;
            border-radius: 6px;
        }
    )QSS");

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(26, 22, 26, 22);
    root->setSpacing(14);

    // HEADER

    auto* heading = new QHBoxLayout;

    auto* titleLayout = new QVBoxLayout;

    auto* eyebrow = makeLabel("STORAGE MANAGEMENT");
    eyebrow->setStyleSheet(
        "color:#356B4A;font-weight:700;font-size:11px;"
    );

    auto* title = makeLabel("File System Simulator");
    title->setStyleSheet(
        "font-size:27px;font-weight:800;color:#193729;"
    );

    auto* subtitle = makeLabel(
        "Explore directories, files and disk allocation."
    );

    subtitle->setStyleSheet("color:#607365;");

    titleLayout->addWidget(eyebrow);
    titleLayout->addWidget(title);
    titleLayout->addWidget(subtitle);

    heading->addLayout(titleLayout, 1);

    auto* backButton =
        new QPushButton("Back to Dashboard");

    heading->addWidget(
        backButton,
        0,
        Qt::AlignTop
    );

    connect(
        backButton,
        &QPushButton::clicked,
        this,
        &FileSystemWindow::backRequested
    );

    root->addLayout(heading);

    // STATISTICS

    auto* statistics = new QHBoxLayout;

    auto createStat = [&](const QString& name,
                          QLabel*& value) {
        auto* card = makePanel();
        auto* layout = new QVBoxLayout(card);

        auto* label = makeLabel(name);
        label->setStyleSheet(
            "color:#607365;font-size:12px;"
        );

        value = makeLabel("0");
        value->setStyleSheet(
            "font-size:23px;font-weight:800;"
        );

        layout->addWidget(label);
        layout->addWidget(value);

        statistics->addWidget(card, 1);
    };

    createStat("Used blocks", m_usedValue);
    createStat("Free blocks", m_freeValue);
    createStat("Disk usage", m_usageValue);

    root->addLayout(statistics);

    // ACTIONS

    auto* actions = makePanel();
    auto* actionLayout = new QVBoxLayout(actions);

    auto* firstRow = new QHBoxLayout;

    auto* newFileButton =
        new QPushButton("New File");

    auto* newFolderButton =
        new QPushButton("New Folder");

    m_readButton = new QPushButton("Read");
    m_writeButton = new QPushButton("Write");
    m_deleteButton = new QPushButton("Delete");

    m_saveButton = new QPushButton("Save Simulation");
    m_saveButton->setObjectName("fsPrimary");

    firstRow->addWidget(newFileButton);
    firstRow->addWidget(newFolderButton);
    firstRow->addWidget(m_readButton);
    firstRow->addWidget(m_writeButton);
    firstRow->addWidget(m_deleteButton);
    firstRow->addStretch();
    firstRow->addWidget(m_saveButton);

    actionLayout->addLayout(firstRow);

    auto* secondRow = new QHBoxLayout;

    secondRow->addWidget(
        makeLabel("Allocation method:")
    );

    m_allocation = new QComboBox;

    m_allocation->addItems({
        "Contiguous",
        "Linked",
        "Indexed"
    });

    secondRow->addWidget(m_allocation);

    m_allocationHelp = makeLabel(
        "Files occupy consecutive disk blocks."
    );

    secondRow->addWidget(m_allocationHelp, 1);

    actionLayout->addLayout(secondRow);

    root->addWidget(actions);

    // LOCATION

    auto* locationRow = new QHBoxLayout;

    auto* upButton = new QPushButton("Up");

    m_pathLabel = makeLabel("/");
    m_pathLabel->setStyleSheet(
        "font-size:14px;font-weight:700;"
    );

    locationRow->addWidget(upButton);
    locationRow->addWidget(
        makeLabel("Current directory:")
    );
    locationRow->addWidget(m_pathLabel, 1);

    root->addLayout(locationRow);

    // MAIN EXPLORER

    auto* splitter = new QSplitter(Qt::Horizontal);

    auto* explorerPanel = makePanel();
    auto* explorerLayout = new QVBoxLayout(explorerPanel);

    auto* explorerTitle = makeLabel("File Explorer");
    explorerTitle->setStyleSheet(
        "font-size:16px;font-weight:700;"
    );

    explorerLayout->addWidget(explorerTitle);

    m_tree = new QTreeWidget;

    m_tree->setColumnCount(3);
    m_tree->setHeaderLabels({
        "Name",
        "Type",
        "Size"
    });

    m_tree->setAlternatingRowColors(true);
    m_tree->setSelectionMode(
        QAbstractItemView::SingleSelection
    );

    explorerLayout->addWidget(m_tree, 1);

    m_selectionInfo = makeLabel(
        "Select a file or directory."
    );

    explorerLayout->addWidget(m_selectionInfo);

    splitter->addWidget(explorerPanel);

    // RIGHT SIDE

    auto* rightPanel = new QWidget;
    auto* rightLayout = new QVBoxLayout(rightPanel);

    rightLayout->setContentsMargins(0, 0, 0, 0);

    auto* diskPanel = makePanel();
    auto* diskLayout = new QVBoxLayout(diskPanel);

    auto* diskTitle = makeLabel("Disk Block Map");
    diskTitle->setStyleSheet(
        "font-size:16px;font-weight:700;"
    );

    diskLayout->addWidget(diskTitle);

    m_usageBar = new QProgressBar;
    m_usageBar->setRange(0, 100);

    diskLayout->addWidget(m_usageBar);

    m_statsLabel = makeLabel("");
    diskLayout->addWidget(m_statsLabel);

    m_blocks = new QTableWidget;
    m_blocks->setColumnCount(10);
    m_blocks->horizontalHeader()
        ->setSectionResizeMode(QHeaderView::Stretch);

    m_blocks->verticalHeader()->hide();

    m_blocks->setEditTriggers(
        QAbstractItemView::NoEditTriggers
    );

    m_blocks->setSelectionMode(
        QAbstractItemView::NoSelection
    );

    diskLayout->addWidget(m_blocks, 1);

    rightLayout->addWidget(diskPanel, 2);

    auto* historyPanel = makePanel();
    auto* historyLayout = new QVBoxLayout(historyPanel);

    auto* historyTitle = makeLabel("Operation History");
    historyTitle->setStyleSheet(
        "font-size:16px;font-weight:700;"
    );

    historyLayout->addWidget(historyTitle);

    m_log = new QListWidget;

    historyLayout->addWidget(m_log, 1);

    rightLayout->addWidget(historyPanel, 1);

    splitter->addWidget(rightPanel);

    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 3);

    root->addWidget(splitter, 1);

    // STATUS

    m_statusLabel = makeLabel(
        "Virtual filesystem ready."
    );

    m_statusLabel->setStyleSheet(
        "background:#E4EDE2;"
        "color:#28583B;"
        "border:1px solid #C9DBC9;"
        "border-radius:9px;"
        "padding:12px;"
        "font-weight:700;"
    );

    root->addWidget(m_statusLabel);

    // CONNECTIONS

    connect(
        newFileButton,
        &QPushButton::clicked,
        this,
        &FileSystemWindow::createFile
    );

    connect(
        newFolderButton,
        &QPushButton::clicked,
        this,
        &FileSystemWindow::createDirectory
    );

    connect(
        m_readButton,
        &QPushButton::clicked,
        this,
        &FileSystemWindow::readFile
    );

    connect(
        m_writeButton,
        &QPushButton::clicked,
        this,
        &FileSystemWindow::writeFile
    );

    connect(
        m_deleteButton,
        &QPushButton::clicked,
        this,
        &FileSystemWindow::deleteSelected
    );

    connect(
        m_saveButton,
        &QPushButton::clicked,
        this,
        &FileSystemWindow::saveSimulation
    );

    connect(
        upButton,
        &QPushButton::clicked,
        this,
        &FileSystemWindow::goUp
    );

    connect(
        m_tree,
        &QTreeWidget::currentItemChanged,
        this,
        [this](QTreeWidgetItem*, QTreeWidgetItem*) {
            updateSelection();
        }
    );

    connect(
        m_tree,
        &QTreeWidget::itemDoubleClicked,
        this,
        [this](QTreeWidgetItem*, int) {
            openSelected();
        }
    );

    connect(
        m_allocation,
        QOverload<int>::of(
            &QComboBox::currentIndexChanged
        ),
        this,
        [this](int index) {
            switch (index) {
            case 0:
                m_allocationHelp->setText(
                    "Files occupy consecutive disk blocks."
                );
                break;
            case 1:
                m_allocationHelp->setText(
                    "Each data block points to the next block."
                );
                break;
            case 2:
                m_allocationHelp->setText(
                    "An index block stores data-block addresses."
                );
                break;
            }
        }
    );

    refresh();
}

QString FileSystemWindow::selectedPath() const
{
    const auto* item = m_tree->currentItem();

    if (!item)
        return QString();

    return item->data(
        0,
        Qt::UserRole
    ).toString();
}

void FileSystemWindow::appendResult(
    const FileSystemResult& result
)
{
    m_statusLabel->setText(qs(result.message));

    m_log->addItem(
        QString("%1 %2")
            .arg(result.success ? "✓" : "✗")
            .arg(qs(result.message))
    );

    for (const auto& step : result.steps)
        m_log->addItem("    " + qs(step));

    m_log->scrollToBottom();

    if (!result.success) {
        QMessageBox::warning(
            this,
            "File System",
            qs(result.message)
        );
    }
}

void FileSystemWindow::refresh()
{
    const FileSystemStats stats =
        m_fileSystem.getStats();

    m_usedValue->setText(
        QString::number(
            static_cast<qulonglong>(stats.usedBlocks)
        )
    );

    m_freeValue->setText(
        QString::number(
            static_cast<qulonglong>(stats.freeBlocks)
        )
    );

    const int percent =
        stats.totalBlocks == 0
            ? 0
            : static_cast<int>(
                stats.usedBlocks * 100 /
                stats.totalBlocks
            );

    m_usageValue->setText(
        QString("%1%").arg(percent)
    );

    m_usageBar->setValue(percent);

    m_statsLabel->setText(
        QString("%1 of %2 blocks allocated")
            .arg(
                static_cast<qulonglong>(stats.usedBlocks)
            )
            .arg(
                static_cast<qulonglong>(stats.totalBlocks)
            )
    );

    m_pathLabel->setText(
        qs(m_fileSystem.currentDirectory())
    );

    refreshTree();
    refreshBlocks();
    updateSelection();
}

void FileSystemWindow::refreshTree()
{
    const QString oldSelection = selectedPath();

    QSignalBlocker blocker(m_tree);

    m_tree->clear();

    auto* rootItem = new QTreeWidgetItem(m_tree);

    rootItem->setText(0, "/");
    rootItem->setText(1, "Directory");
    rootItem->setData(0, Qt::UserRole, "/");
    rootItem->setData(
        0,
        Qt::UserRole + 1,
        true
    );

    std::function<void(
        QTreeWidgetItem*,
        const std::string&
    )> populate;

    populate = [&](QTreeWidgetItem* parent,
                   const std::string& path) {

        std::string error;

        const auto entries =
            m_fileSystem.listDirectory(path, &error);

        for (const auto& entry : entries) {
            auto* item = new QTreeWidgetItem(parent);

            item->setText(0, qs(entry.name));

            item->setText(
                1,
                entry.isDirectory
                    ? "Directory"
                    : "File"
            );

            if (!entry.isDirectory) {
                item->setText(
                    2,
                    QString("%1 B").arg(
                        static_cast<qulonglong>(
                            entry.sizeBytes
                        )
                    )
                );
            }

            item->setData(
                0,
                Qt::UserRole,
                qs(entry.path)
            );

            item->setData(
                0,
                Qt::UserRole + 1,
                entry.isDirectory
            );

            if (entry.isDirectory)
                populate(item, entry.path);
        }
    };

    populate(rootItem, "/");

    rootItem->setExpanded(true);

    if (!oldSelection.isEmpty())
        selectPath(oldSelection);
    else
        m_tree->setCurrentItem(rootItem);
}

void FileSystemWindow::selectPath(
    const QString& path
)
{
    std::function<QTreeWidgetItem*(
        QTreeWidgetItem*
    )> findItem;

    findItem = [&](QTreeWidgetItem* item)
        -> QTreeWidgetItem* {

        if (!item)
            return nullptr;

        if (item->data(
                0,
                Qt::UserRole
            ).toString() == path)
            return item;

        for (int i = 0;
             i < item->childCount();
             ++i) {

            if (auto* found =
                    findItem(item->child(i)))
                return found;
        }

        return nullptr;
    };

    if (auto* item =
            findItem(m_tree->topLevelItem(0))) {

        m_tree->setCurrentItem(item);

        auto* parent = item->parent();

        while (parent) {
            parent->setExpanded(true);
            parent = parent->parent();
        }
    }
}

void FileSystemWindow::refreshBlocks()
{
    const auto blocks =
        m_fileSystem.getDiskBlocks();

    const int columns = 10;

    const int rows =
        static_cast<int>(
            (blocks.size() + columns - 1) /
            columns
        );

    m_blocks->setRowCount(rows);
    m_blocks->setColumnCount(columns);

    for (int row = 0; row < rows; ++row) {
        for (int col = 0; col < columns; ++col) {
            const int index = row * columns + col;

            if (index >= static_cast<int>(
                    blocks.size())) {
                m_blocks->setItem(
                    row,
                    col,
                    new QTableWidgetItem("")
                );
                continue;
            }

            const auto& block =
                blocks[static_cast<std::size_t>(index)];

            auto* item = new QTableWidgetItem(
                QString::number(block.index)
            );

            if (!block.allocated) {
                item->setBackground(
                    QColor("#E7EFE5")
                );

                item->setForeground(
                    QColor("#45604B")
                );

                item->setToolTip(
                    "Free disk block"
                );
            } else if (block.isIndexBlock) {
                item->setBackground(
                    QColor("#D7C5E8")
                );

                item->setForeground(
                    QColor("#44305B")
                );

                item->setToolTip(
                    "Index block\nOwner: " +
                    qs(block.ownerPath)
                );
            } else {
                item->setBackground(
                    QColor("#73A887")
                );

                item->setForeground(
                    QColor("#FFFFFF")
                );

                QString tooltip =
                    "Allocated block\nOwner: " +
                    qs(block.ownerPath);

                if (block.nextBlock >= 0) {
                    tooltip +=
                        "\nNext block: " +
                        QString::number(
                            block.nextBlock
                        );
                }

                item->setToolTip(tooltip);
            }

            item->setTextAlignment(
                Qt::AlignCenter
            );

            m_blocks->setItem(row, col, item);
        }
    }

    m_blocks->resizeRowsToContents();
}

void FileSystemWindow::updateSelection()
{
    const QString path = selectedPath();

    const auto* item = m_tree->currentItem();

    const bool isDirectory =
        item &&
        item->data(
            0,
            Qt::UserRole + 1
        ).toBool();

    const bool isFile =
        !path.isEmpty() && !isDirectory;

    m_readButton->setEnabled(isFile);
    m_writeButton->setEnabled(isFile);

    m_deleteButton->setEnabled(
        !path.isEmpty() && path != "/"
    );

    if (path.isEmpty()) {
        m_selectionInfo->setText(
            "Select a file or directory."
        );
        return;
    }

    if (isDirectory) {
        m_selectionInfo->setText(
            "Directory: " + path
        );
        return;
    }

    FileMetadata metadata;

    if (!m_fileSystem.getFileMetadata(
            path.toStdString(),
            metadata)) {
        m_selectionInfo->setText(
            "File information unavailable."
        );
        return;
    }

    m_selectionInfo->setText(
        QString("%1 | %2 bytes | %3 | %4 data blocks")
            .arg(path)
            .arg(
                static_cast<qulonglong>(
                    metadata.sizeBytes
                )
            )
            .arg(qs(
                FileSystem::allocationMethodName(
                    metadata.allocationMethod
                )
            ))
            .arg(
                static_cast<qulonglong>(
                    metadata.dataBlockCount
                )
            )
    );
}

void FileSystemWindow::openSelected()
{
    const QString path = selectedPath();

    if (path.isEmpty())
        return;

    const auto* item = m_tree->currentItem();

    const bool directory =
        item &&
        item->data(
            0,
            Qt::UserRole + 1
        ).toBool();

    if (directory) {
        appendResult(
            m_fileSystem.changeDirectory(
                path.toStdString()
            )
        );

        refresh();
    } else {
        readFile();
    }
}

void FileSystemWindow::createFile()
{
    bool ok = false;

    const QString name =
        QInputDialog::getText(
            this,
            "Create File",
            "File name:",
            QLineEdit::Normal,
            "newfile.txt",
            &ok
        );

    if (!ok || name.trimmed().isEmpty())
        return;

    const QString content =
        QInputDialog::getMultiLineText(
            this,
            "File Content",
            "Enter file content:",
            "",
            &ok
        );

    if (!ok)
        return;

    const auto method =
        static_cast<AllocationMethod>(
            m_allocation->currentIndex()
        );

    const auto result =
        m_fileSystem.createFile(
            name.trimmed().toStdString(),
            content.toStdString(),
            method
        );

    appendResult(result);
    refresh();

    if (result.success) {
        const QString directory =
            qs(m_fileSystem.currentDirectory());

        const QString path =
            directory == "/"
                ? "/" + name.trimmed()
                : directory + "/" + name.trimmed();

        selectPath(path);
    }
}

void FileSystemWindow::createDirectory()
{
    bool ok = false;

    const QString name =
        QInputDialog::getText(
            this,
            "Create Directory",
            "Directory name:",
            QLineEdit::Normal,
            "New Folder",
            &ok
        );

    if (!ok || name.trimmed().isEmpty())
        return;

    appendResult(
        m_fileSystem.createDirectory(
            name.trimmed().toStdString()
        )
    );

    refresh();
}

void FileSystemWindow::readFile()
{
    const QString path = selectedPath();

    if (path.isEmpty())
        return;

    std::string content;

    const auto result =
        m_fileSystem.readFile(
            path.toStdString(),
            content
        );

    appendResult(result);

    if (result.success) {
        QMessageBox::information(
            this,
            "Read File - " + path,
            qs(content).isEmpty()
                ? "(Empty file)"
                : qs(content)
        );
    }
}

void FileSystemWindow::writeFile()
{
    const QString path = selectedPath();

    if (path.isEmpty())
        return;

    std::string content;

    const auto readResult =
        m_fileSystem.readFile(
            path.toStdString(),
            content
        );

    if (!readResult.success) {
        appendResult(readResult);
        return;
    }

    bool ok = false;

    const QString updated =
        QInputDialog::getMultiLineText(
            this,
            "Write File",
            "File content:",
            qs(content),
            &ok
        );

    if (!ok)
        return;

    appendResult(
        m_fileSystem.writeFile(
            path.toStdString(),
            updated.toStdString()
        )
    );

    refresh();
    selectPath(path);
}

void FileSystemWindow::deleteSelected()
{
    const QString path = selectedPath();

    if (path.isEmpty() || path == "/")
        return;

    const auto* item = m_tree->currentItem();

    const bool directory =
        item &&
        item->data(
            0,
            Qt::UserRole + 1
        ).toBool();

    const auto choice =
        QMessageBox::question(
            this,
            "Confirm Delete",
            "Delete " + path + "?",
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::No
        );

    if (choice != QMessageBox::Yes)
        return;

    const auto result =
        directory
            ? m_fileSystem.removeDirectory(
                path.toStdString()
            )
            : m_fileSystem.deleteFile(
                path.toStdString()
            );

    appendResult(result);
    refresh();
}

void FileSystemWindow::changeDirectory()
{
    bool ok = false;

    const QString path =
        QInputDialog::getText(
            this,
            "Change Directory",
            "Directory path:",
            QLineEdit::Normal,
            qs(m_fileSystem.currentDirectory()),
            &ok
        );

    if (!ok || path.trimmed().isEmpty())
        return;

    appendResult(
        m_fileSystem.changeDirectory(
            path.trimmed().toStdString()
        )
    );

    refresh();
}

void FileSystemWindow::goUp()
{
    appendResult(
        m_fileSystem.changeDirectory("..")
    );

    refresh();
}

// SAVE AND RESTORE

QJsonObject
FileSystemWindow::serializeFileSystemSnapshot() const
{
    return snapshotToJson(
        m_fileSystem.exportSnapshot()
    );
}

QJsonObject
FileSystemWindow::captureSimulationInput() const
{
    QJsonObject input;

    input["schemaVersion"] = 1;
    input["snapshot"] =
        serializeFileSystemSnapshot();

    input["selectedAllocation"] =
        m_allocation->currentIndex();

    input["selectedPath"] = selectedPath();

    return input;
}

QJsonObject
FileSystemWindow::captureSimulationOutput() const
{
    QJsonObject output;

    output["schemaVersion"] = 1;
    output["statusText"] =
        m_statusLabel->text();

    QJsonArray history;

    for (int i = 0; i < m_log->count(); ++i)
        history.append(
            m_log->item(i)->text()
        );

    output["operationHistory"] = history;

    const auto stats = m_fileSystem.getStats();

    output["totalBlocks"] =
        static_cast<qint64>(stats.totalBlocks);

    output["usedBlocks"] =
        static_cast<qint64>(stats.usedBlocks);

    output["freeBlocks"] =
        static_cast<qint64>(stats.freeBlocks);

    return output;
}

void FileSystemWindow::saveSimulation()
{
    const QJsonObject inputData =
        captureSimulationInput();

    const QJsonObject outputData =
        captureSimulationOutput();

    const QString algorithm =
        m_allocation->currentText();

    const auto stats = m_fileSystem.getStats();

    const QString summary =
        QString("%1 | %2/%3 blocks used | %4")
            .arg(algorithm)
            .arg(
                static_cast<qulonglong>(
                    stats.usedBlocks
                )
            )
            .arg(
                static_cast<qulonglong>(
                    stats.totalBlocks
                )
            )
            .arg(qs(
                m_fileSystem.currentDirectory()
            ));

    QJsonObject payload;

    payload["module"] = "File System";
    payload["algorithm"] = algorithm;
    payload["inputData"] = inputData;
    payload["outputData"] = outputData;

    QString error;

    const bool success =
        SimulationRepository::instance().save(
            "File System",
            algorithm,
            summary,
            inputData,
            outputData,
            payload,
            error
        );

    if (!success) {
        QMessageBox::warning(
            this,
            "Save Simulation Failed",
            error
        );
        return;
    }

    QMessageBox::information(
        this,
        "Simulation Saved",
        "File System simulation saved successfully."
    );

    emit simulationSaved();
}

bool FileSystemWindow::restoreFileSystemSnapshot(
    const QJsonObject& json,
    QString& errorMessage
)
{
    FileSystemSnapshot snapshot;

    if (!jsonToSnapshot(
            json,
            snapshot,
            errorMessage))
        return false;

    std::string backendError;

    if (!m_fileSystem.importSnapshot(
            snapshot,
            backendError)) {

        errorMessage = qs(backendError);
        return false;
    }

    errorMessage.clear();
    return true;
}

bool FileSystemWindow::restoreSimulation(
    const QJsonObject& inputData,
    const QJsonObject& outputData
)
{
    if (!inputData.value("snapshot").isObject())
        return false;

    if (!outputData.value("operationHistory").isArray())
        return false;

    const int allocationIndex =
        inputData.value("selectedAllocation").toInt(-1);

    if (allocationIndex < 0 ||
        allocationIndex >= m_allocation->count())
        return false;

    const QJsonArray history =
        outputData.value("operationHistory").toArray();

    for (const auto& item : history) {
        if (!item.isString())
            return false;
    }

    QString error;

    if (!restoreFileSystemSnapshot(
            inputData.value("snapshot").toObject(),
            error)) {

        QMessageBox::warning(
            this,
            "Restore Failed",
            error
        );

        return false;
    }

    m_allocation->setCurrentIndex(
        allocationIndex
    );

    m_log->clear();

    for (const auto& entry : history)
        m_log->addItem(entry.toString());

    m_statusLabel->setText(
        outputData.value("statusText").toString(
            "Saved simulation restored."
        )
    );

    refresh();

    const QString path =
        inputData.value("selectedPath").toString();

    if (!path.isEmpty())
        selectPath(path);

    return true;
}
