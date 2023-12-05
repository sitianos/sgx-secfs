# Fuse Operations of Filesystem

functions below of [fuse_lowlevel_ops](https://libfuse.github.io/doxygen/structfuse__lowlevel__ops.html) structure are defined

### lookup
- looks up child inode with parent inode number and child name
-  within enclave
    - check if read operation to parent directory is permitted
    - fetch corresponding inode file from storage if
    - cache this inode
    - increment reference count
- returns child inode number and attributes

### forget
- decreases reference count of given inode
- within enclave
    - if reference count becomes 0, corresponding cache will be removed from enclave

### getattr
- get stat of given inode
- operation is almost the same as lookup

### setattr

### (readlink)

### mkdir
- creates directory with parent inode and directory name
- within enclave
    - check write permission to parent dirnode
    - create dirnode
    - add its entry to parent dirnode
    - save these dirnode to storage
- returns created inode number and attributes

### unlink
- remove file with parent inode and file name
- within enclave
    - check write permission to parent dirnode
    - remove removed file from storage
    - remove chunks from storage
    - save parent dirnode to storage

### rmdir
- removes directory with parent inode number and directory name
- within enclave
    - check write permission to parent dirnode
    - check if directory is empty
    - remove removed dirnode from storage
    - remove its entry from parent dirnode
    - save parent dirnode to storage

### (symlink)

### rename

### (link)

### open
- opens file with flags for writing, reading or both
- within enclave
    - checks by given flags if user has access permission to the file

### read
- reads data of opened file to given buffer with offset and size
- within enclave
    - if required chunks are not stored on local, fetch them from storage
    - check integrity of them
    - unset modified bit of each fetched chunk
- returns requested data and its size

### write
- writes given data to opened file with offset and size
- within enclave
    - write data to local chunk
    - if only part of the chunk is written, fetch the chunk from storage in advance
    - set modified bit of each modified chunk
- returns size of written data

### flush
- closes opened file
- within enclave
    - save chunks whose modified bit is set to storage
    - unset modified bit

### release

### opendir
- opens directory with its inode number
- within enclave
    - check read permission to directory
    - get directory entries from cache

### readdir
- gets directory entries of opened directory
- returns entries fetched on opendir
- if needed, fetches additional entries from enclave

### releasedir
- closes directory opened with opendir

### setxattr

### getxattr

### listxattr

### removexattr

### access
- check if access to given file or directory is permitted

### create
- create file with parent directory inode and file name
- almost the same as mkdir
