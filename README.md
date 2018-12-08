# vbackup
File archive and restore application. Goal - to learn and practice C++ 11 constructs.

## Requirements
* Command line application
* Use STL
* Create archive of volume, e.g. C:
* Include all files
* Provide archive and restore commands
* Multi-thread
* Show progress bar
* Use ZLIB library for compression

## To do later / in the future
* Split data files in volumes of e.g. 4GB, like data.z.1, data.z.2, ...
* Archive and restore file attributes and permissions (access control lists)
* Much improved error handling is needed
* Special handling of soft/hard links?
* Restore on live system?

## Design
The program traverses the source path, creates index blocks for each directory 
with records for each file and subdirectory. The files are read in chunks and 
compression jobs created and placed in a compressor queue.

The program uses several threads for compression of files and a job queue, and one 
thread with a separate writing job queue for storing the compressed data blocks. 
The number of compression threads is equal to the number of machine threads available 
less one, as this is what I found to be optimal number.

The program uses several classes for archiving. Archiver is the main component,
FileIndexer traverses the directories, Compressor runs compression threads, BlockWriter thread
stores to disk, ProgressIndicator shows time elapsed and current % complete.

These objects are completely independant of each other, yet can communicate using function objects. 
Archiver creates and wires them with lambda functions so that FileIndexer can store in the index 
the file offset and last block# in the compressed data file, the compressor threads to pass jobs 
to the writing queue and ProgressIndicator to show current progress.

The restoring process is single threaded and very quick.

![Components](./docs/component-diagram.png)
![Data flow](./docs/dataflow-diagram.png)
![Index file](./docs/index-file-structure.png)
![Data file](./docs/data-file-structure.png)


## Useful information / code examples

* VShadow Volume Shadow Copy Service examples, https://github.com/Microsoft/Windows-classic-samples/tree/master/Samples/VShadowVolumeShadowCopy
* https://wj32.org/wp/2012/12/13/how-to-backup-files-in-c-using-the-volume-shadow-copy-service-vss/
* Progress bar, https://stackoverflow.com/questions/14539867/how-to-display-a-progress-indicator-in-pure-c-c-cout-printf
* ZLIB, https://www.zlib.net/zlib_how.html and PIGZ, https://github.com/madler/pigz
* ZLIB multi-thread, https://stackoverflow.com/questions/30294766/how-to-use-multiple-threads-for-zlib-compression

