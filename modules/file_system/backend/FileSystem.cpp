
#include "FileSystem.h"

#include <algorithm>
#include <set>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <utility>

FileSystem::FileSystem(std::size_t totalBlocks, std::size_t blockSizeBytes)
    : m_totalBlocks(std::max<std::size_t>(1, totalBlocks)),
      m_blockSizeBytes(std::max<std::size_t>(1, blockSizeBytes))
{
    reset();
}

std::string FileSystem::timestampNow()
{
    const std::time_t now = std::time(nullptr);
    std::tm local{};
#ifdef _WIN32
    localtime_s(&local, &now);
#else
    localtime_r(&now, &local);
#endif
    std::ostringstream out;
    out << std::put_time(&local, "%Y-%m-%d %H:%M:%S");
    return out.str();
}

std::string FileSystem::baseName(const std::string& path)
{
    if (path == "/") return "/";
    const auto pos = path.find_last_of('/');
    return pos == std::string::npos ? path : path.substr(pos + 1);
}

std::string FileSystem::parentPath(const std::string& path)
{
    if (path == "/") return "/";
    const auto pos = path.find_last_of('/');
    if (pos == 0 || pos == std::string::npos) return "/";
    return path.substr(0, pos);
}

std::string FileSystem::normalizePath(
    const std::string& path,
    const std::string& base
)
{
    std::vector<std::string> parts;
    const std::string source =
        (!path.empty() && path.front() == '/')
            ? path
            : (base + "/" + path);

    std::stringstream stream(source);
    std::string part;

    while (std::getline(stream, part, '/')) {
        if (part.empty() || part == ".") continue;
        if (part == "..") {
            if (!parts.empty()) parts.pop_back();
            continue;
        }
        parts.push_back(part);
    }

    std::string result = "/";
    for (std::size_t i = 0; i < parts.size(); ++i) {
        if (i) result += "/";
        result += parts[i];
    }
    return result;
}

bool FileSystem::isValidName(const std::string& name)
{
    return !name.empty() &&
           name != "." &&
           name != ".." &&
           name.find('/') == std::string::npos &&
           name.find('\\') == std::string::npos;
}

int FileSystem::findFile(const std::string& normalizedPath) const
{
    for (std::size_t i = 0; i < m_files.size(); ++i) {
        if (m_files[i].metadata.path == normalizedPath)
            return static_cast<int>(i);
    }
    return -1;
}

bool FileSystem::directoryExists(
    const std::string& normalizedPath
) const
{
    return std::find(
        m_directories.begin(),
        m_directories.end(),
        normalizedPath
    ) != m_directories.end();
}

std::size_t FileSystem::countDirectoryChildren(
    const std::string& normalizedPath
) const
{
    std::size_t count = 0;

    for (const auto& directory : m_directories)
        if (directory != normalizedPath &&
            parentPath(directory) == normalizedPath)
            ++count;

    for (const auto& file : m_files)
        if (parentPath(file.metadata.path) == normalizedPath)
            ++count;

    return count;
}

void FileSystem::reset()
{
    m_blocks.assign(m_totalBlocks, DiskBlockInfo{});

    for (std::size_t i = 0; i < m_blocks.size(); ++i)
        m_blocks[i].index = static_cast<int>(i);

    m_directories.clear();
    m_directories.push_back("/");
    m_files.clear();
    m_currentDirectory = "/";
}

FileSystemResult FileSystem::createDirectory(
    const std::string& path
)
{
    const std::string normalized =
        normalizePath(path, m_currentDirectory);

    if (normalized == "/")
        return {false, "The root directory already exists.", {}};

    if (directoryExists(normalized) ||
        findFile(normalized) >= 0)
        return {
            false,
            "A file or directory with that path already exists.",
            {}
        };

    if (!directoryExists(parentPath(normalized)))
        return {false, "The parent directory does not exist.", {}};

    m_directories.push_back(normalized);

    return {
        true,
        "Directory created: " + normalized,
        {
            "Resolve directory path: " + normalized,
            "Verify parent directory exists",
            "Create directory entry"
        }
    };
}

FileSystemResult FileSystem::removeDirectory(
    const std::string& path
)
{
    const std::string normalized =
        normalizePath(path, m_currentDirectory);

    if (normalized == "/")
        return {false, "The root directory cannot be deleted.", {}};

    const auto it = std::find(
        m_directories.begin(),
        m_directories.end(),
        normalized
    );

    if (it == m_directories.end())
        return {false, "Directory not found: " + normalized, {}};

    if (countDirectoryChildren(normalized) != 0)
        return {
            false,
            "Directory is not empty. Remove its contents first.",
            {}
        };

    m_directories.erase(it);

    if (m_currentDirectory == normalized ||
        (m_currentDirectory.size() > normalized.size() &&
         m_currentDirectory.compare(
             0, normalized.size(), normalized
         ) == 0 &&
         m_currentDirectory[normalized.size()] == '/'))
        m_currentDirectory = "/";

    return {
        true,
        "Directory deleted: " + normalized,
        {
            "Resolve directory path",
            "Check that the directory is empty",
            "Remove directory entry"
        }
    };
}

FileSystemResult FileSystem::changeDirectory(
    const std::string& path
)
{
    const std::string normalized =
        normalizePath(path, m_currentDirectory);

    if (!directoryExists(normalized))
        return {false, "Directory not found: " + normalized, {}};

    m_currentDirectory = normalized;

    return {
        true,
        "Current directory: " + m_currentDirectory,
        {
            "Resolve path",
            "Verify directory exists",
            "Update current directory"
        }
    };
}

void FileSystem::clearBlock(int index)
{
    if (index < 0 ||
        static_cast<std::size_t>(index) >= m_blocks.size())
        return;

    m_blocks[static_cast<std::size_t>(index)] = DiskBlockInfo{};
    m_blocks[static_cast<std::size_t>(index)].index = index;
}

void FileSystem::releaseFileBlocks(const FileRecord& file)
{
    for (const int block : file.metadata.dataBlocks)
        clearBlock(block);

    if (file.metadata.indexBlock >= 0)
        clearBlock(file.metadata.indexBlock);
}

bool FileSystem::allocateBlocks(
    const std::string& ownerPath,
    AllocationMethod method,
    std::size_t dataBlockCount,
    std::vector<int>& dataBlocks,
    int& indexBlock,
    std::vector<std::string>& steps
)
{
    dataBlocks.clear();
    indexBlock = -1;

    if (dataBlockCount == 0) {
        steps.push_back("Empty file requires no data blocks");
        return true;
    }

    if (method == AllocationMethod::Contiguous) {
        steps.push_back(
            "Search for " + std::to_string(dataBlockCount) +
            " consecutive free blocks"
        );

        std::size_t run = 0;
        std::size_t start = 0;
        bool found = false;

        for (std::size_t i = 0; i < m_blocks.size(); ++i) {
            if (!m_blocks[i].allocated) {
                if (run == 0) start = i;
                ++run;

                if (run == dataBlockCount) {
                    found = true;
                    break;
                }
            } else {
                run = 0;
            }
        }

        if (!found) {
            steps.push_back(
                "No sufficiently large consecutive free region was found"
            );
            return false;
        }

        for (std::size_t i = start;
             i < start + dataBlockCount; ++i)
            dataBlocks.push_back(static_cast<int>(i));
    } else {
        const std::size_t required =
            dataBlockCount +
            (method == AllocationMethod::Indexed ? 1 : 0);

        std::vector<int> freeBlocks;

        for (const auto& block : m_blocks)
            if (!block.allocated)
                freeBlocks.push_back(block.index);

        if (freeBlocks.size() < required) {
            steps.push_back(
                "Not enough free blocks: need " +
                std::to_string(required) +
                ", available " +
                std::to_string(freeBlocks.size())
            );
            return false;
        }

        if (method == AllocationMethod::Linked)
            steps.push_back(
                "Select free blocks and link them in logical file order"
            );
        else
            steps.push_back(
                "Reserve one index block, then select data blocks"
            );

        if (method == AllocationMethod::Indexed) {
            indexBlock = freeBlocks.front();
            freeBlocks.erase(freeBlocks.begin());
        }

        dataBlocks.assign(
            freeBlocks.begin(),
            freeBlocks.begin() +
                static_cast<std::ptrdiff_t>(dataBlockCount)
        );
    }

    for (std::size_t i = 0; i < dataBlocks.size(); ++i) {
        auto& block =
            m_blocks[static_cast<std::size_t>(dataBlocks[i])];

        block.allocated = true;
        block.ownerPath = ownerPath;
        block.isIndexBlock = false;

        block.nextBlock =
            (method == AllocationMethod::Linked &&
             i + 1 < dataBlocks.size())
                ? dataBlocks[i + 1]
                : -1;
    }

    if (indexBlock >= 0) {
        auto& block =
            m_blocks[static_cast<std::size_t>(indexBlock)];

        block.allocated = true;
        block.ownerPath = ownerPath;
        block.isIndexBlock = true;
        block.nextBlock = -1;
        block.indexEntries = dataBlocks;
    }

    steps.push_back(
        "Allocated data blocks: " +
        std::to_string(dataBlocks.size())
    );

    if (indexBlock >= 0)
        steps.push_back(
            "Index block: " + std::to_string(indexBlock)
        );

    return true;
}

FileSystemResult FileSystem::createFile(
    const std::string& path,
    const std::string& content,
    AllocationMethod method,
    int requestedDataBlocks
)
{
    const std::string normalized =
        normalizePath(path, m_currentDirectory);
    const std::string name = baseName(normalized);

    if (normalized == "/" || !isValidName(name))
        return {false, "Invalid file name or path.", {}};

    if (!directoryExists(parentPath(normalized)))
        return {false, "The parent directory does not exist.", {}};

    if (directoryExists(normalized) || findFile(normalized) >= 0)
        return {
            false,
            "A file or directory with that path already exists.",
            {}
        };

    if (requestedDataBlocks == 0 || requestedDataBlocks < -1)
        return {
            false,
            "Requested data blocks must be a positive number.",
            {}
        };

    const std::size_t blocksNeeded =
        requestedDataBlocks > 0
            ? static_cast<std::size_t>(requestedDataBlocks)
            : (content.empty()
                ? 0
                : (content.size() + m_blockSizeBytes - 1) /
                      m_blockSizeBytes);

    if (blocksNeeded < content.size() / m_blockSizeBytes +
            (content.size() % m_blockSizeBytes != 0))
        return {false, "Requested block count cannot hold file contents.", {}};

    FileRecord record;
    record.metadata.name = name;
    record.metadata.path = normalized;
    record.metadata.sizeBytes = content.size();
    record.metadata.dataBlockCount = blocksNeeded;
    record.metadata.allocationMethod = method;
    record.metadata.createdAt = timestampNow();
    record.metadata.modifiedAt = record.metadata.createdAt;
    record.content = content;

    std::vector<std::string> steps = {
        "Resolve file path: " + normalized,
        "Verify parent directory and unique name"
    };

    if (!allocateBlocks(
            normalized,
            method,
            blocksNeeded,
            record.metadata.dataBlocks,
            record.metadata.indexBlock,
            steps))
        return {
            false,
            "Insufficient space or allocation failed for " + normalized,
            steps
        };

    m_files.push_back(std::move(record));

    steps.push_back(
        "Create file metadata and store content in the virtual filesystem"
    );

    return {true, "File created: " + normalized, steps};
}

FileSystemResult FileSystem::deleteFile(
    const std::string& path
)
{
    const std::string normalized =
        normalizePath(path, m_currentDirectory);
    const int index = findFile(normalized);

    if (index < 0)
        return {false, "File not found: " + normalized, {}};

    const FileRecord record =
        m_files[static_cast<std::size_t>(index)];

    releaseFileBlocks(record);
    m_files.erase(m_files.begin() + index);

    return {
        true,
        "File deleted: " + normalized,
        {
            "Resolve file path",
            "Release allocated data and index blocks",
            "Remove file metadata"
        }
    };
}

FileSystemResult FileSystem::writeFile(
    const std::string& path,
    const std::string& content,
    int requestedDataBlocks
)
{
    const std::string normalized =
        normalizePath(path, m_currentDirectory);
    const int index = findFile(normalized);

    if (index < 0)
        return {false, "File not found: " + normalized, {}};

    if (!m_files[static_cast<std::size_t>(index)]
             .metadata.permissions.write)
        return {
            false,
            "Write permission denied for " + normalized,
            {}
        };

    if (requestedDataBlocks == 0 || requestedDataBlocks < -1)
        return {
            false,
            "Requested data blocks must be a positive number.",
            {}
        };

    FileRecord oldRecord =
        m_files[static_cast<std::size_t>(index)];

    const std::size_t blocksNeeded =
        requestedDataBlocks > 0
            ? static_cast<std::size_t>(requestedDataBlocks)
            : (content.empty()
                ? 0
                : (content.size() + m_blockSizeBytes - 1) /
                      m_blockSizeBytes);

    if (blocksNeeded < content.size() / m_blockSizeBytes +
            (content.size() % m_blockSizeBytes != 0))
        return {false, "Requested block count cannot hold file contents.", {}};

    // Roll back the exact original disk map if reallocation fails.
    // Re-running the allocator is not sufficient: it may pick different blocks.
    const std::vector<DiskBlockInfo> originalBlocks = m_blocks;
    releaseFileBlocks(oldRecord);

    FileRecord updated = oldRecord;
    updated.metadata.sizeBytes = content.size();
    updated.metadata.dataBlockCount = blocksNeeded;
    updated.metadata.dataBlocks.clear();
    updated.metadata.indexBlock = -1;
    updated.content = content;
    updated.metadata.modifiedAt = timestampNow();

    std::vector<std::string> steps = {
        "Resolve file path",
        "Check write permission",
        "Release old allocation before resizing"
    };

    if (!allocateBlocks(
            normalized,
            oldRecord.metadata.allocationMethod,
            blocksNeeded,
            updated.metadata.dataBlocks,
            updated.metadata.indexBlock,
            steps)) {
        m_blocks = originalBlocks;
        return {
            false,
            "Write failed: insufficient space. Original file was preserved.",
            steps
        };
    }

    m_files[static_cast<std::size_t>(index)] =
        std::move(updated);

    steps.push_back("Store updated content and metadata");

    return {true, "File updated: " + normalized, steps};
}

FileSystemResult FileSystem::renameFile(
    const std::string& path,
    const std::string& newName
)
{
    const std::string normalized =
        normalizePath(path, m_currentDirectory);
    const int index = findFile(normalized);

    if (index < 0)
        return {false, "File not found: " + normalized, {}};

    if (!isValidName(newName))
        return {false, "Invalid new file name.", {}};

    const std::string newPath =
        parentPath(normalized) == "/"
            ? "/" + newName
            : parentPath(normalized) + "/" + newName;

    if (findFile(newPath) >= 0 || directoryExists(newPath))
        return {
            false,
            "The destination name is already in use.",
            {}
        };

    auto& file = m_files[static_cast<std::size_t>(index)];

    file.metadata.name = newName;
    file.metadata.path = newPath;
    file.metadata.modifiedAt = timestampNow();

    for (const int block : file.metadata.dataBlocks)
        m_blocks[static_cast<std::size_t>(block)].ownerPath =
            newPath;

    if (file.metadata.indexBlock >= 0)
        m_blocks[static_cast<std::size_t>(
            file.metadata.indexBlock
        )].ownerPath = newPath;

    return {
        true,
        "File renamed to " + newPath,
        {
            "Resolve source",
            "Validate new name",
            "Update metadata and block ownership"
        }
    };
}

FileSystemResult FileSystem::moveFile(
    const std::string& path,
    const std::string& destinationDirectory
)
{
    const std::string normalized =
        normalizePath(path, m_currentDirectory);
    const int index = findFile(normalized);

    if (index < 0)
        return {false, "File not found: " + normalized, {}};

    const std::string destination =
        normalizePath(destinationDirectory, m_currentDirectory);

    if (!directoryExists(destination))
        return {
            false,
            "Destination directory does not exist.",
            {}
        };

    const std::string newPath =
        destination == "/"
            ? "/" + baseName(normalized)
            : destination + "/" + baseName(normalized);

    if (newPath == normalized)
        return {
            true,
            "File is already in that directory.",
            {}
        };

    if (findFile(newPath) >= 0 || directoryExists(newPath))
        return {
            false,
            "A destination entry with that name already exists.",
            {}
        };

    auto& file = m_files[static_cast<std::size_t>(index)];
    file.metadata.path = newPath;
    file.metadata.modifiedAt = timestampNow();

    for (const int block : file.metadata.dataBlocks)
        m_blocks[static_cast<std::size_t>(block)].ownerPath =
            newPath;

    if (file.metadata.indexBlock >= 0)
        m_blocks[static_cast<std::size_t>(
            file.metadata.indexBlock
        )].ownerPath = newPath;

    return {
        true,
        "File moved to " + newPath,
        {
            "Resolve source and destination",
            "Verify destination directory",
            "Update file path and block ownership"
        }
    };
}

FileSystemResult FileSystem::setPermissions(
    const std::string& path,
    const FilePermissions& permissions
)
{
    const std::string normalized =
        normalizePath(path, m_currentDirectory);
    const int index = findFile(normalized);

    if (index < 0)
        return {false, "File not found: " + normalized, {}};

    m_files[static_cast<std::size_t>(index)]
        .metadata.permissions = permissions;

    m_files[static_cast<std::size_t>(index)]
        .metadata.modifiedAt = timestampNow();

    return {
        true,
        "Permissions updated: " + normalized,
        {
            "Resolve file",
            "Update read/write/execute permission flags"
        }
    };
}

FileSystemResult FileSystem::readFile(
    const std::string& path,
    std::string& content
) const
{
    const std::string normalized =
        normalizePath(path, m_currentDirectory);
    const int index = findFile(normalized);

    if (index < 0)
        return {false, "File not found: " + normalized, {}};

    const auto& file =
        m_files[static_cast<std::size_t>(index)];

    if (!file.metadata.permissions.read)
        return {
            false,
            "Read permission denied for " + normalized,
            {}
        };

    content = file.content;

    return {
        true,
        "File read: " + normalized,
        {
            "Resolve file path",
            "Check read permission",
            "Read content from virtual storage"
        }
    };
}

bool FileSystem::getFileMetadata(
    const std::string& path,
    FileMetadata& metadata
) const
{
    const int index =
        findFile(normalizePath(path, m_currentDirectory));

    if (index < 0)
        return false;

    metadata =
        m_files[static_cast<std::size_t>(index)].metadata;

    return true;
}

std::vector<DirectoryEntry> FileSystem::listDirectory(
    const std::string& path,
    std::string* error
) const
{
    const std::string normalized =
        normalizePath(path, m_currentDirectory);

    if (!directoryExists(normalized)) {
        if (error)
            *error = "Directory not found: " + normalized;
        return {};
    }

    if (error)
        error->clear();

    std::vector<DirectoryEntry> entries;

    for (const auto& directory : m_directories) {
        if (directory != normalized &&
            parentPath(directory) == normalized)
            entries.push_back({
                baseName(directory),
                directory,
                true,
                0,
                0,
                ""
            });
    }

    for (const auto& file : m_files) {
        if (parentPath(file.metadata.path) == normalized)
            entries.push_back({
                file.metadata.name,
                file.metadata.path,
                false,
                file.metadata.sizeBytes,
                file.metadata.dataBlocks.size() +
                    (file.metadata.indexBlock >= 0 ? 1U : 0U),
                file.metadata.modifiedAt
            });
    }

    std::sort(
        entries.begin(),
        entries.end(),
        [](const DirectoryEntry& a, const DirectoryEntry& b) {
            if (a.isDirectory != b.isDirectory)
                return a.isDirectory > b.isDirectory;
            return a.name < b.name;
        }
    );

    return entries;
}

std::vector<DiskBlockInfo> FileSystem::getDiskBlocks() const
{
    return m_blocks;
}

FileSystemStats FileSystem::getStats() const
{
    FileSystemStats stats;

    stats.totalBlocks = m_totalBlocks;
    stats.totalCapacityBytes =
        m_totalBlocks * m_blockSizeBytes;

    for (const auto& block : m_blocks)
        if (block.allocated)
            ++stats.usedBlocks;

    stats.freeBlocks =
        stats.totalBlocks - stats.usedBlocks;

    stats.usedCapacityBytes =
        stats.usedBlocks * m_blockSizeBytes;

    stats.freeCapacityBytes =
        stats.freeBlocks * m_blockSizeBytes;

    return stats;
}

std::size_t FileSystem::blockSizeBytes() const
{
    return m_blockSizeBytes;
}

std::size_t FileSystem::totalBlocks() const
{
    return m_totalBlocks;
}

std::string FileSystem::currentDirectory() const
{
    return m_currentDirectory;
}

std::string FileSystem::allocationMethodName(
    AllocationMethod method
)
{
    switch (method) {
    case AllocationMethod::Contiguous:
        return "Contiguous";
    case AllocationMethod::Linked:
        return "Linked";
    case AllocationMethod::Indexed:
        return "Indexed";
    }

    return "Unknown";
}

FileSystemSnapshot FileSystem::exportSnapshot() const
{
    FileSystemSnapshot snapshot;

    snapshot.version = 1;
    snapshot.totalBlocks = m_totalBlocks;
    snapshot.blockSizeBytes = m_blockSizeBytes;
    snapshot.blocks = m_blocks;
    snapshot.directories = m_directories;
    snapshot.currentDirectory = m_currentDirectory;

    for (const auto& file : m_files)
        snapshot.files.push_back({
            file.metadata,
            file.content
        });

    return snapshot;
}

bool FileSystem::importSnapshot(
    const FileSystemSnapshot& snapshot,
    std::string& errorMessage
)
{
    auto fail = [&](const std::string& why) {
        errorMessage = why;
        return false;
    };

    if (snapshot.version != 1)
        return fail("Unsupported filesystem snapshot version.");

    if (snapshot.totalBlocks == 0 ||
        snapshot.blockSizeBytes == 0 ||
        snapshot.totalBlocks > 100000 ||
        snapshot.blockSizeBytes > 10000000 ||
        snapshot.blocks.size() != snapshot.totalBlocks)
        return fail("Invalid disk geometry or block count.");

    if (snapshot.directories.empty() ||
        snapshot.directories.front() != "/")
        return fail("Snapshot must contain the root directory.");

    std::set<std::string> directories;

    for (const auto& path : snapshot.directories) {
        if (path.empty() ||
            path[0] != '/' ||
            normalizePath(path, "/") != path ||
            !directories.insert(path).second)
            return fail(
                "Invalid or duplicate directory path: " + path
            );
    }

    for (const auto& path : snapshot.directories) {
        if (path != "/" &&
            directories.count(parentPath(path)) == 0)
            return fail("Directory has no parent: " + path);
    }

    if (directories.count(snapshot.currentDirectory) == 0)
        return fail("Current directory does not exist.");

    std::set<std::string> paths;
    std::vector<int> ownership(snapshot.totalBlocks, 0);

    for (const auto& file : snapshot.files) {
        const auto& meta = file.metadata;

        if (meta.path == "/" ||
            meta.path.empty() ||
            normalizePath(meta.path, "/") != meta.path ||
            baseName(meta.path) != meta.name ||
            !isValidName(meta.name) ||
            directories.count(parentPath(meta.path)) == 0 ||
            directories.count(meta.path) != 0 ||
            !paths.insert(meta.path).second)
            return fail(
                "Invalid or duplicate file path: " + meta.path
            );

        if (meta.sizeBytes != file.content.size() ||
            meta.dataBlockCount != meta.dataBlocks.size() ||
            meta.dataBlocks.size() <
                (file.content.size() / snapshot.blockSizeBytes +
                 (file.content.size() %
                      snapshot.blockSizeBytes != 0)))
            return fail(
                "Inconsistent file size or data block count: " +
                meta.path
            );

        if (meta.allocationMethod !=
                AllocationMethod::Contiguous &&
            meta.allocationMethod !=
                AllocationMethod::Linked &&
            meta.allocationMethod !=
                AllocationMethod::Indexed)
            return fail("Invalid allocation method.");

        if (meta.allocationMethod !=
                AllocationMethod::Indexed &&
            meta.indexBlock != -1)
            return fail("Unexpected index block.");

        if (meta.allocationMethod ==
                AllocationMethod::Indexed &&
            meta.dataBlocks.empty() &&
            meta.indexBlock != -1)
            return fail(
                "Empty indexed file has an index block."
            );

        if (meta.allocationMethod ==
                AllocationMethod::Indexed &&
            !meta.dataBlocks.empty() &&
            meta.indexBlock < 0)
            return fail(
                "Indexed file is missing its index block."
            );

        auto claim = [&](
            int id,
            bool isIndex,
            int next,
            const std::vector<int>& entries
        ) {
            if (id < 0 ||
                static_cast<std::size_t>(id) >=
                    snapshot.totalBlocks ||
                ++ownership[static_cast<std::size_t>(id)] != 1)
                return false;

            const auto& block =
                snapshot.blocks[static_cast<std::size_t>(id)];

            return block.index == id &&
                   block.allocated &&
                   block.ownerPath == meta.path &&
                   block.isIndexBlock == isIndex &&
                   block.nextBlock == next &&
                   block.indexEntries == entries;
        };

        for (std::size_t i = 0;
             i < meta.dataBlocks.size(); ++i) {
            const int next =
                meta.allocationMethod ==
                    AllocationMethod::Linked &&
                i + 1 < meta.dataBlocks.size()
                    ? meta.dataBlocks[i + 1]
                    : -1;

            if (!claim(
                    meta.dataBlocks[i],
                    false,
                    next,
                    {}))
                return fail(
                    "Invalid data block ownership or pointer: " +
                    meta.path
                );

            if (meta.allocationMethod ==
                    AllocationMethod::Contiguous &&
                i > 0 &&
                meta.dataBlocks[i] !=
                    meta.dataBlocks[i - 1] + 1)
                return fail(
                    "Noncontiguous blocks in contiguous file."
                );
        }

        if (meta.indexBlock >= 0 &&
            !claim(
                meta.indexBlock,
                true,
                -1,
                meta.dataBlocks))
            return fail(
                "Invalid index block: " + meta.path
            );
    }

    for (std::size_t i = 0;
         i < snapshot.blocks.size(); ++i) {
        const auto& block = snapshot.blocks[i];

        if (block.index != static_cast<int>(i) ||
            block.allocated != (ownership[i] == 1))
            return fail(
                "Disk allocation map is inconsistent."
            );

        if (!block.allocated &&
            (block.isIndexBlock ||
             !block.ownerPath.empty() ||
             block.nextBlock != -1 ||
             !block.indexEntries.empty()))
            return fail(
                "Free block contains stale allocation metadata."
            );
    }

    // Commit only after validation succeeds.
    std::vector<FileRecord> restored;
    restored.reserve(snapshot.files.size());

    for (const auto& file : snapshot.files)
        restored.push_back({
            file.metadata,
            file.content
        });

    m_totalBlocks = snapshot.totalBlocks;
    m_blockSizeBytes = snapshot.blockSizeBytes;
    m_blocks = snapshot.blocks;
    m_directories = snapshot.directories;
    m_files = std::move(restored);
    m_currentDirectory = snapshot.currentDirectory;

    errorMessage.clear();
    return true;
}
