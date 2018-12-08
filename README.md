# vbackup

### Requirements
* Command line application
* Use STL
* Create archive of volume, e.g. C:
* Include all files
* Provide archive and restore commands
* Multi-thread
* Show progress bar
* Use ZLIB library for compression

### Not needed or to do later / in the future
* Archive and restore file attributes and permissions (access control lists)

### Design
![Component flow](./docs/component-diagram.png)
![Data flow](./docs/data-flow-diagram.png)
![Index file](./docs/index-file-structure.png)
![Data file](./docs/data-file-structure.png)

### Useful information / code examples

* VShadow Volume Shadow Copy Service examples
    * https://github.com/Microsoft/Windows-classic-samples/tree/master/Samples/VShadowVolumeShadowCopy
    * https://wj32.org/wp/2012/12/13/how-to-backup-files-in-c-using-the-volume-shadow-copy-service-vss/
* Progress bar, https://stackoverflow.com/questions/14539867/how-to-display-a-progress-indicator-in-pure-c-c-cout-printf
* ZLIB, https://www.zlib.net/zlib_how.html and PIGZ, https://github.com/madler/pigz
* ZLIB multi-thread, https://stackoverflow.com/questions/30294766/how-to-use-multiple-threads-for-zlib-compression

