# Installation Guide
At this time SecFS can be run in Docker container

## prerequisites:
- Docker CLI and Docker Compose Plugin
- Access permission to /var/run/dockerd (by joining docker group) or Docker Rootlesskit
- Kernel support for FUSE module and access permission to /dev/fuse

## build
- First, build Docker image and create container with following commands
```
$ cd docker
$ docker compose build
$ docker compose up -d
```

- Then, build SecFS within the container
```
$ docker attach docker_secfs
# mkdir /build
# make BUILD_DIR=/build
```

- If build is succeeded, you can find `secfs` and `secfs_cli` commands in `/build/bin` and enclave named `secfs_enclave.signed.so` in `/build/lib`

## how to run filesystem
- Before mounting filesystem, you have to create volume with configuration file
- At first, create key pair for user identification with following command
```
# mkdir /working && mkdir /working/vol && cd /working 
# openssl ecparam -genkey -name secp384r1 -out privkey.pem
# openssl ec -in privkey.pem -pubout -out pubkey.pem
```

- Then create configuration file such like
```
{
    "public_key_path": "/working/pubkey.pem",
    "volumekey": "",
    "enclave_path": "/build/lib/secfs_enclave.signed.so",
    "storage": {
        "type": "local",
        "config": {
            "base_dir": "/working/vol"
        }
    }
}
```
in which
- `public_key_path`: the location of public key
- `enclave_path`: the location of enclave file
- `storage.config.base_dir`: the directory in which the encrypted files are stored

- Create volume with configuration file
```
# /build/bin/secfs_cli create --config volume.json
```
- if secuceeded you can find some files in storage base directory
- then you can mount the volume with configuration file and mount point
```
# mkdir /tmp/mnt
# /build/bin/secfs /tmp/mnt -f -d --config volume.json
```

- To test the filesystem, create another terminal and run shell in Docker container
```
$ cd docker
$ docker compose exec secfs bash
```
- Then you can list, create, edit or remove files and directories in mounpoint
- Filnaly unmount it
```
# umount /tmp/mnt
```
- `secfs` program will be automatically finished
