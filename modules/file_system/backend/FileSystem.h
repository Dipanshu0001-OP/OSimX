
#ifndef OSIMX_FILE_SYSTEM_H
#define OSIMX_FILE_SYSTEM_H

#include <cstddef>
#include <string>
#include <vector>

// In-memory filesystem simulator.
// No operations access the host operating system's filesystem.

enum class AllocationMethod {
    Contiguous,
    Linked,
    Indexed
};

struct FilePermissions {
    bool read = true;
    bool write = true;
    bool execute = false;
};

struct FileMetadata {
    std::string name;
    std::string path;

    std::size_t sizeBytes = 0;
    std::size_t dataBlockCount = 0;

    AllocationMethod allocationMethod =
        AllocationMethod::Contiguous;

    std::vector<int> dataBlocks;
    int indexBlock = -1;

    FilePermissions permissions;

    std::string createdAt;
    std::string modifiedAt;
};

struct DirectoryEntry {
    std::string name;
    std::string path;

    bool isDirectory = false;

    std::size_t sizeBytes = 0;
    std::size_t allocatedBlocks = 0;

    std::string modifiedAt;
};

struct DiskBlockInfo {
    int index = -1;

    bool allocated = false;
    bool isIndexBlock = false;

    std::string ownerPath;

    int nextBlock = -1;

    std::vector<int> indexEntries;
};

struct FileSystemResult {
    bool success = false;

    std::string message;
    std::vector<std::string> steps;
};

struct FileSystemStats {
    std::size_t totalBlocks = 0;
    std::size_t usedBlocks = 0;
    std::size_t freeBlocks = 0;

    std::size_t totalCapacityBytes = 0;
    std::size_t usedCapacityBytes = 0;
    std::size_t freeCapacityBytes = 0;
};

// A serializable representation of one virtual file.
struct FileSystemSnapshotFile {
    FileMetadata metadata;
    std::string content;
};

// Complete filesystem state.
//
// This stores actual disk allocation information,
// not merely filenames and contents.
struct FileSystemSnapshot {
    int version = 1;

    std::size_t totalBlocks = 0;
    std::size_t blockSizeBytes = 0;

    std::vector<DiskBlockInfo> blocks;

    std::vector<std::string> directories;
    std::vector<FileSystemSnapshotFile> files;

    std::string currentDirectory;
};

class FileSystem {
public:
    explicit FileSystem(
        std::size_t totalBlocks = 100,
        std::size_t blockSizeBytes = 100
    );

    // Directory operations

    FileSystemResult createDirectory(
        const std::string& path
    );

    FileSystemResult removeDirectory(
        const std::string& path
    );

    FileSystemResult changeDirectory(
        const std::string& path
    );

    // File operations

    FileSystemResult createFile(
        const std::string& path,
        const std::string& content,
        AllocationMethod method,
        int requestedDataBlocks = -1
    );

    FileSystemResult deleteFile(
        const std::string& path
    );

    FileSystemResult writeFile(
        const std::string& path,
        const std::string& content,
        int requestedDataBlocks = -1
    );

    FileSystemResult renameFile(
        const std::string& path,
        const std::string& newName
    );

    FileSystemResult moveFile(
        const std::string& path,
        const std::string& destinationDirectory
    );

    FileSystemResult setPermissions(
        const std::string& path,
        const FilePermissions& permissions
    );

    FileSystemResult readFile(
        const std::string& path,
        std::string& content
    ) const;

    // Information and visualization

    bool getFileMetadata(
        const std::string& path,
        FileMetadata& metadata
    ) const;

    std::vector<DirectoryEntry> listDirectory(
        const std::string& path,
        std::string* error = nullptr
    ) const;

    std::vector<DiskBlockInfo> getDiskBlocks() const;

    FileSystemStats getStats() const;

    std::size_t blockSizeBytes() const;
    std::size_t totalBlocks() const;

    std::string currentDirectory() const;

    void reset();

    static std::string allocationMethodName(
        AllocationMethod method
    );

    // Save/restore support

    FileSystemSnapshot exportSnapshot() const;

    // Returns false without changing the existing
    // filesystem if the snapshot is invalid.
    bool importSnapshot(
        const FileSystemSnapshot& snapshot,
        std::string& errorMessage
    );

private:
    struct FileRecord {
        FileMetadata metadata;
        std::string content;
    };

    std::size_t m_totalBlocks;
    std::size_t m_blockSizeBytes;

    std::vector<DiskBlockInfo> m_blocks;
    std::vector<std::string> m_directories;
    std::vector<FileRecord> m_files;

    std::string m_currentDirectory;

    static std::string timestampNow();

    static std::string baseName(
        const std::string& path
    );

    static std::string parentPath(
        const std::string& path
    );

    static std::string normalizePath(
        const std::string& path,
        const std::string& base
    );

    static bool isValidName(
        const std::string& name
    );

    int findFile(
        const std::string& normalizedPath
    ) const;

    bool directoryExists(
        const std::string& normalizedPath
    ) const;

    std::size_t countDirectoryChildren(
        const std::string& normalizedPath
    ) const;

    void releaseFileBlocks(
        const FileRecord& file
    );

    bool allocateBlocks(
        const std::string& ownerPath,
        AllocationMethod method,
        std::size_t dataBlockCount,
        std::vector<int>& dataBlocks,
        int& indexBlock,
        std::vector<std::string>& steps
    );

    void clearBlock(int index);
};

#endif // OSIMX_FILE_SYSTEM_H
