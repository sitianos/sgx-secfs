FROM ubuntu:22.04 as sgx

RUN apt-get update && apt-get install -y \
    build-essential \
    ocaml \
    ocamlbuild \
    automake \
    autoconf \
    libtool \
    wget \
    python-is-python3 \
    libssl-dev \
    git \
    cmake \
    perl \
    libssl-dev \
    libcurl4-openssl-dev \
    protobuf-compiler \
    libprotobuf-dev \
    debhelper \
    cmake \
    reprepro \
    unzip \
    pkgconf \
    libboost-dev \
    libboost-system-dev \
    libboost-thread-dev \
    lsb-release \
    libsystemd0

WORKDIR /
# COPY . .

RUN git clone https://github.com/intel/linux-sgx.git 
WORKDIR /linux-sgx
RUN make preparation

RUN make sdk_install_pkg_no_mitigation

WORKDIR /opt/intel
RUN sh -c 'echo yes | /linux-sgx/linux/installer/bin/sgx_linux_x64_sdk_*.bin'

ENV SGX_SDK=/opt/intel/sgxsdk
WORKDIR /linux-sgx
RUN make psw_install_pkg

RUN /linux-sgx/linux/installer/bin/sgx_linux_x64_psw*.bin --no-start-aesm

WORKDIR /app
CMD ["/bin/bash"]

FROM sgx as secfs

RUN apt update && apt install -y meson uuid-dev libmbedtls-dev nlohmann-json3-dev

WORKDIR /tmp
RUN git clone https://github.com/libfuse/libfuse.git

WORKDIR /app
