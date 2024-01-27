# Installation Guide
At this time SecFS can be run in Docker container

## prerequisites:
- Docker CLI and Docker Compose Plugin
- Access permission to /var/run/dockerd (by joining docker group) or Docker Rootlesskit
- Kernel support for FUSE module and access permission to /dev/fuse
- Kernel support for SGX and access permission to /dev/sgx_*

## build
- First, build Docker image and create container with following commands
```
$ cd docker
$ docker compose -f compose_prod.yml build
```
- If you want to change build option, modify optional arguments to `make` in Dockerfile_HW

- Then, create a container from the image and run SecFS within the container
```
$ docker compose -f compose_prod.yml up -d
$ docker compose -f compose_prod.yml exec secfs_hw bash
```

- You can find `secfs` and `secfs_cli` commands in `/app/build/bin` and enclave named `secfs_enclave.signed.so` in `/app/build/lib`

## how to run filesystem
- Before mounting filesystem, you have to create volume with configuration file
- At first, create key pair for user identification with following command
```
# mkdir -p /working/vol && cd /working 
# openssl ecparam -genkey -name secp384r1 -out privkey.pem
# openssl ec -in privkey.pem -pubout -out pubkey.pem
```

- Then create base configuration file such like
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

- Create volume with base configuration file
```
# secfs_cli create -n base.json -c volume.json
```
- if secuceeded you can find `volume.json` and some files in storage base directory
- then you can mount the volume with `volume.json` and mount point
```
# mkdir /tmp/mnt
# secfs /tmp/mnt -f -d -o config=volume.json
```

- To test the filesystem, create another terminal and run shell in Docker container
```
$ cd docker
$ docker compose -f compose_prod.yml exec secfs bash
```
- Then you can list, create, edit or remove files and directories in mounpoint
- Filnaly unmount it
```
# umount /tmp/mnt
```
- `secfs` program will be automatically finished
