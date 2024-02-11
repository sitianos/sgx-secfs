`fio -filename=mnt/file1 -direct=0 -rw=write -bs=4k -size=256M -numjobs=1 -runtime=300 -group_reporting -name=file1`

Intel Core i7-9700K CPU @ 3.60GHz 8core
64MB enclave memory
Storage: NFS

block size = 4KiB

- SecFS (chunk size = 4KiB) cache disabled, thread = 1

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 5.17 | 20.2 |
| sequential write | 1.54 | 6.01 |

- SecFS (chunk size = 4KiB) cache disabled, thread = 10

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 8.93 | 34.9 |
| sequential write | 1.31 | 5.14 |

- SecFS (chunk size = 4KiB) cache enable, thread = 1

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 5.22 | 20.4 |
| sequential write | 2.11 | 8.25 |

- SecFS (chunk size = 4KiB) cache enable, thread = 10

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 10.1 | 39.3 |
| sequential write | 10.6 | 41.4 |

- SecFS (chunk size = 1MiB) cache enable, thread = 1

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 36.6 | 143 |
| sequential write | 53.9 | 211 |

- SecFS (chunk size = 1MiB) writeback, thread = 10

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 4.19 | 16.4 |
| sequential write | 14.4 | 56.3 |

- NEXUS

| rw type | IOPS | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 96.8 | 378 |
| sequential write | 93.2 | 382 |

- FUSE passthrough filesystem threads = 1

| rw type | IOPS | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 119 | 465 |
| sequential write | 103 | 423 |

- FUSE passthrough filesystem threads = 10

| rw type | IOPS | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 133 | 518 |
| sequential write | 156 | 608 |

- NFS

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read |  720 | 2950 |
| sequential write | 308 | 1260 |
