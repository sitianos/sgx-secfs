# Fuse Operations of Filesystem

### lookup
- looks up child inode with parent inode number and child name
-  within enclave
    - check if read operation to parent directory is permitted
    - fetch corresponding inode file from storage if
    - cache this inode
    - increment reference count
- returns inode number and attributes

### forget
- decreases refernce count of given inode
- within enclave
    - if refernce count becomes 0, corresponding cache is removed from enclave

### getattr
- get stat of given inode
- operates almost the same as lookup

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

### read

### write

### flush

### release

### setxattr

### getxattr

### listxattr

### removexattr

### access

### create

### opendir
- opens directory with its inode number
- within enclave
    - check if read operation to directory is permitted
    - get dirent from its cache

### readdir
- gets directory entries of opened directory
- returns entries fetched on opendir
- if needed, fetches additional entries from enclave

### releasedir
- closes directory opened with opendir
