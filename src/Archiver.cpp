// Archiver - wires FileIndexer, Compressor, BlockWriter, ProgressIndicator objects and runs processing

#include "pch.h"
#include "Archiver.h"
#include "FileIndexer.h"
#include "BlockWriter.h"
#include "Compressor.h"
#include "ProgressIndicator.h"
#include "VssClient.h"
#include "CCoInitialize.h"

#include <filesystem>
namespace fs = std::experimental::filesystem;

void Archiver::Run(const std::wstring &src, const std::wstring &dest)
{
    FileIndexer indexer;
    Compressor compressor;
    BlockWriter writer;
    ProgressIndicator indicator;
    // use machine threads - 1 for compression
    int compressorCount = std::thread::hardware_concurrency() - 1;

    CCoInitialize coInit;
    VssClient vssClient;
    std::wstring rootPath = MakeRootPath(src);
    vssClient.CreateSnapshot(rootPath);
    //    vssClient.CopySnapshotFile(L"\\Users\\valyo\\NTUSER.DAT", L"c:/NTUSER-copy.DAT");

    const FileIndexerStatistics& stats = indexer.Statistics();
    auto writeJobFinished = [&indexer, &indicator, &stats](const Job& job) {
        indexer.WriteJobFinished(job);
        indicator.Update(static_cast<float>(static_cast<long double>(stats.bytesCompressed)
            / stats.totalBytes)); };
    auto setFileOffset = [&indexer](size_t blockNo, size_t recNo, uintmax_t offset) {
        indexer.SetFileOffset(blockNo, recNo, offset); };
    auto enqueueCompressorJob = [&compressor](Job &job) { compressor.Enqueue(job); };

    compressor.Start(compressorCount, [&writer](Job &job) { writer.Enqueue(job); });
    writer.Start(dest, setFileOffset, writeJobFinished);
    indexer.Start(src, dest, enqueueCompressorJob);
    writer.Complete(indexer.Statistics().jobsCreated);
    compressor.Complete();
}

// Make sure volume name has a trailing backslash
std::wstring Archiver::MakeRootPath(const std::wstring &path)
{
    fs::path rootPath(path);
    rootPath = rootPath.root_path();
    if (rootPath.empty())
        throw std::runtime_error("Invalid argument. Please specify valid volume for archiving, e.g. C:");

    if (!rootPath.has_root_directory())
        rootPath += fs::path::preferred_separator;

    return rootPath;
}