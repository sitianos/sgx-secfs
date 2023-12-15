# SecFS: Shared Filesystem Using Intel SGX
## Overview
SecFS is a filesystem for sharing files among multiple users. It uses Intel SGX to provide following features

- ensuring confidenciality and integrity of files and directories in the volume
- allowing users to manipulate access controls over each file or directory and modify them with little overhead

## Structure of Filesystem
Read [docs](docs/)

## How to build
At this moment, this filesystem can be built only on Docker container. Read [this](INSTALL.md) for installation
