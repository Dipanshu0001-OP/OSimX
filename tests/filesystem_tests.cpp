#include "modules/file_system/backend/FileSystem.h"
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

int main() {
    int checks = 0;
    auto check = [&](bool condition, const char* label) {
        ++checks;
        if (!condition) {
            std::cerr << "FAIL: " << label << '\n';
            std::exit(EXIT_FAILURE);
        }
    };
    auto sameBlocks = [](const std::vector<DiskBlockInfo>& a, const std::vector<DiskBlockInfo>& b) {
        if (a.size() != b.size()) return false;
        for (std::size_t i = 0; i < a.size(); ++i) {
            if (a[i].index != b[i].index || a[i].allocated != b[i].allocated ||
                a[i].isIndexBlock != b[i].isIndexBlock || a[i].ownerPath != b[i].ownerPath ||
                a[i].nextBlock != b[i].nextBlock || a[i].indexEntries != b[i].indexEntries)
                return false;
        }
        return true;
    };
    for (auto allocation : {AllocationMethod::Contiguous, AllocationMethod::Linked, AllocationMethod::Indexed}) {
        FileSystem fs(8, 4);
        check(fs.createDirectory("/docs").success, "create directory");
        check(fs.createFile("/first", "ABCDEFGH", allocation).success, "first file");
        check(fs.createFile("/docs/second", "IJKLMNOP", allocation).success, "second file");
        check(fs.deleteFile("/first").success, "delete first file");
        const FileSystemSnapshot before = fs.exportSnapshot();
        check(!fs.writeFile("/docs/second", std::string(80, 'z')).success, "oversized write fails");
        const FileSystemSnapshot after = fs.exportSnapshot();
        check(sameBlocks(before.blocks, after.blocks), "failed write restores exact disk map");
        check(before.files.size() == after.files.size() &&
                  before.files[0].metadata.dataBlocks == after.files[0].metadata.dataBlocks &&
                  before.files[0].metadata.indexBlock == after.files[0].metadata.indexBlock &&
                  before.files[0].content == after.files[0].content, "failed write preserves content and metadata");
        check(!fs.createFile("/bad", "12345678", allocation, 1).success, "reject too few requested blocks on create");
        check(!fs.writeFile("/docs/second", "12345678", 1).success, "reject too few requested blocks on write");
        check(sameBlocks(before.blocks, fs.getDiskBlocks()), "invalid write leaves blocks unchanged");
        FileSystem restored(2, 1);
        std::string error;
        check(restored.importSnapshot(before, error), "import valid snapshot");
        check(sameBlocks(before.blocks, restored.getDiskBlocks()), "snapshot restores exact disk geometry");
        std::string content;
        check(restored.readFile("/docs/second", content).success && content == "IJKLMNOP", "snapshot preserves file content");
        auto corrupted = before;
        corrupted.blocks[0].allocated = !corrupted.blocks[0].allocated;
        const auto originalRestored = restored.exportSnapshot();
        check(!restored.importSnapshot(corrupted, error), "corrupt snapshot rejected");
        check(sameBlocks(originalRestored.blocks, restored.getDiskBlocks()), "corrupt snapshot does not partially modify disk");
        check(restored.writeFile("/docs/second", "OK").success, "valid overwrite succeeds");
        check(restored.readFile("/docs/second", content).success && content == "OK", "successful write updates content");
    }
    std::cout << checks << " filesystem checks passed\n";
    return EXIT_SUCCESS;
}
