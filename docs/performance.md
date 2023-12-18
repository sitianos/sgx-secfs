`fio -filename=/tmp/mnt/file1 -direct=1 -rw=write -bs=4k -size=64M -numjobs=1 -runtime=60 -group_reporting -name=file1`

- SecFS

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 11.0 | 42.9 |
| sequential write | 9.04 | 35.2 |
| random read | 11.2 | 43.8 |
| random write | 19.5 | 76.1 |

- FUSE passthrough filesystem

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 8.537 | 33.4 |
| sequential write |  | |
| random read | 4.33 | 16.9 |
| random write | | |

- direct filesystem

| rw type | IOPS (k) | throughput (MiB/s) |
| ---- | ---- | ---- |
| sequential read | 74.6 | 291 |
| sequential write | 12.3 | 47.9 |
| random read | 81.4 | 318 |
| random write | 13.6 | 53.0 |
