`fio -filename=/tmp/mnt/file1 -direct=1 -rw=write,read -bs=4k -size=1G -numjobs=1 -runtime=300 -group_reporting -name=file1`
Intel Core i7-9700K CPU @ 3.60GHz

block size = 4KiB

- SecFS (chunk size = 4KiB)

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 1.06 | 4.24 |
| sequential write | 1.383 | 5.38 |

- SecFS (chunk size = 1MiB)

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 4.04 | 15.8 |
| sequential write | 1.63 | 6.52 |

- FUSE passthrough filesystem

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 15.3 | 59.8 |
| sequential write | -- | -- |

- direct filesystem

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 17.2 | 67.3 |
| sequential write | 95.7 | 374 |


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
