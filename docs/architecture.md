# Architecture of filesystem

## Filesystem Layers

1. fuse layer
   - define functions corresponding to filesystem system calls by using fuse lowlevel API

1. enclave layer
    - define functions called from fuse layer
    - user authentication, access permission checks, file encryption/decryption and integrity checks are performed within this layer

1. remote storage layer
    - process requests from enclave layer
    - allow users to handle several storage APIs (local filesystem, FTP, Amazon S3, etc.) by providing abstract class

## Filesystem Components
- each file stored in remote storage has assigned UUID as file name
- remote storage files are devided into metadata and encrypted chunks

### metadata
   - metadata files are encrypted with volume key
   - each of them has one of following types
   1. superinfo
      - only one metadata which stores infomation of the entire volume
      - every user knows UUID of this file
      1. UUID and hash of dirnode of root directory
      1. UUID and hash of usertable
      
   1. usertable
      - stores user information joined in volume
      1. user name (or user ID) and their public key
      1. whether user has right to invite another user to the volume

   1. dirnode
      - stores information of each directory
      1. directory name
      1. ACL
      1. dirent including UUID and hashes

   1. filenode
      - stores information of each file
      1. file name
      1. ACL
      1. file stat
      1. chunk list (inode or adjacency list)
  
   1. (chunk entry)
      - this metadata is required if huge files might be stored
      - it is because without this metadata, huge file will make the number of chunk entries of filenode too large
      1. UUID of chunk
      1. chunk size
      1. chunk encryption key
      1. chunk hash

### chunks
  - each chunk is encrypted with a key stored in corresponding chunk entry

