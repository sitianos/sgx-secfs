`fio -filename=/tmp/mnt/file1 -direct=0 -rw=write -bs=4k -size=256M -numjobs=1 -runtime=300 -group_reporting -name=file1`

Intel Core i7-9700K CPU @ 3.60GHz
Core = 8

block size = 4KiB

- SecFS (chunk size = 4KiB) no writeback, thread = 1

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 5.17 | 20.2 |
| sequential write | 1.54 | 6.01 |

- SecFS (chunk size = 4KiB) no writeback, thread = 10

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 8.93 | 34.9 |
| sequential write | 1.31 | 5.14 |

- SecFS (chunk size = 4KiB) writeback, thread = 1

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 5.22 | 20.4 |
| sequential write | 2.11 | 8.25 |

- SecFS (chunk size = 4KiB) writeback, thread = 10

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 10.1 | 39.3 |
| sequential write | 10.6 | 41.4 |

- SecFS (chunk size = 1MiB)

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 4.04 | 15.8 |
| sequential write | 1.63 | 6.52 |

- FUSE passthrough filesystem

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 474 | 2917 |
| sequential write | 92.4 | 361 |

- direct filesystem

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 247 | 963 |
| sequential write | 701 | 2738 |

block size = 1 MiB

- SecFS (chunk size = 4KiB)

| rw type | IOPS | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 20 | 20.1 |
| sequential write | 6 | 6.64 |

- SecFS (chunk size = 1MiB)

| rw type | IOPS | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 20 | 20.6 |
| sequential write | 6 | 7.71 |

- FUSE passthrough filesystem

| rw type | IOPS | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 536 | 536 |
| sequential write | -- | -- |

- direct filesystem

| rw type | IOPS | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 3246 | 3247 |
| sequential write | 1059 | 1060 |
