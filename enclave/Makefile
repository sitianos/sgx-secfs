BUILD_DIR ?= ../build
include ../build.mk

Enclave_Version_Script := enclave_debug.lds
ifeq ($(SGX_MODE), HW)
ifneq ($(SGX_DEBUG), 1)
ifneq ($(SGX_PRERELEASE), 1)
	Enclave_Version_Script = enclave.lds 
endif
endif
endif

ifneq ($(SGX_MODE), HW)
	Trts_Library_Name := sgx_trts_sim
	Service_Library_Name := sgx_tservice_sim
else
	Trts_Library_Name := sgx_trts
	Service_Library_Name := sgx_tservice
endif
Crypto_Library_Name := sgx_tcrypto

Enclave_Include_Paths := $(addprefix -I, . ../include $(SGX_SDK)/include $(SGX_SDK)/include/libcxx $(SGX_SDK)/include/tlibc) $(shell pkg-config fuse3 --cflags)

CFLAGS := -nostdinc -fvisibility=hidden -fpie -fstack-protector -fno-builtin-printf $(Enclave_Include_Paths)
CXXFLAGS := $(CFLAGS) -nostdinc++

Enclave_Security_Link_Flags := -Wl,-z,relro,-z,now,-z,noexecstack

Enclave_Link_Flags := $(Enclave_Security_Link_Flags) \
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_LIBRARY_PATH) \
	-Wl,--whole-archive -l$(Trts_Library_Name) -Wl,--no-whole-archive \
	-Wl,--start-group -lsgx_tstdc -lsgx_tcxx -lsgx_mbedcrypto -lsgx_tcrypto -l$(Service_Library_Name) -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
	-Wl,--defsym,__ImageBase=0 \
	-Wl,--version-script=$(Enclave_Version_Script)

OBJ_DIR := $(BUILD_DIR)/enclave
objs := uuid.o metadata.o superinfo.o usertable.o dirnode.o filenode.o volume.o ecall_fs.o ecall_volume.o enclave.o storage.o
build_objs := $(addprefix $(OBJ_DIR)/, $(objs))
signed_enclave_img := $(LIB_DIR)/secfs_enclave.signed.so
enclave_img := $(LIB_DIR)/secfs_enclave.so
signing_config := enclave.config.xml
signing_key ?= $(OBJ_DIR)/sign.key

.PHONY: all clean

all: $(OBJ_DIR) $(enclave_img) $(signed_enclave_img)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
	mkdir -p $(LIB_DIR)

$(enclave_img): $(build_objs) $(OBJ_DIR)/enclave_t.o
	@echo ---------- link $@ ----------
	$(CXX) $^ $(Enclave_Link_Flags) -o $@
	@echo

$(build_objs): $(OBJ_DIR)/%.o: %.cpp enclave_t.h
	@echo ---------- build $@ ----------
	$(CXX) $< -c $(CXXFLAGS) -o $@
	@echo

$(OBJ_DIR)/enclave_t.o: enclave_t.c
	@echo ---------- build $@ ----------
	$(CC) $< -c $(CFLAGS) -o $@
	@echo

enclave_t.c: enclave_t.h

enclave_t.h: $(SGX_EDGER8R) enclave.edl
	$(SGX_EDGER8R) --trusted enclave.edl --search-path . --search-path $(SGX_SDK)/include

$(signing_key):
	openssl genrsa -out $@ -3 3072

$(signed_enclave_img): $(enclave_img) $(signing_key)
	$(SGX_ENCLAVE_SIGNER) sign -key $(signing_key) -enclave $(enclave_img) -out $@ -config $(signing_config)

clean:
	rm -f $(build_objs) $(OBJ_DIR)/enclave_t.o $(enclave_img) $(signed_enclave_img)
