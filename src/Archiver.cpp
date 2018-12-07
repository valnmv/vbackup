// Archiver - wires FileIndexer, Compressor, BlockWriter, ProgressIndicator objects and runs processing

#include "pch.h"
#include "Archiver.h"
#include "FileIndexer.h"
#include "BlockWriter.h"
#include "Compressor.h"
#include "ProgressIndicator.h"

void Archiver::Run(const std::wstring &src, const std::wstring &dest)
{
    compressorCount = std::thread::hardware_concurrency();

    FileIndexer indexer;
    Compressor compressor;
    BlockWriter writer;
    ProgressIndicator indicator;

    auto writeJobFinished = [&indexer, &indicator](const Job& job) { indexer.WriteJobFinished(job);
        indicator.Update(static_cast<float>(static_cast<long double>(indexer.Statistics().bytesCompressed) 
            / indexer.Statistics().totalBytes)); };
    auto setFileOffset = [&indexer](size_t blockNo, size_t recNo, uintmax_t offset) {
        indexer.SetFileOffset(blockNo, recNo, offset); };
    auto enqueueCompressorJob = [&compressor](Job &job) { compressor.Enqueue(job); };

    compressor.Start(compressorCount - 1, [&writer](Job &job) { writer.Enqueue(job); });
    writer.Start(dest, setFileOffset, writeJobFinished);
    indexer.Start(src, dest, enqueueCompressorJob);
    writer.Complete(indexer.Statistics().jobsCreated);
    compressor.Complete();
}
