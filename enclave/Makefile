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

Enclave_Include_Paths := -I$(BUILD_DIR)/enclave -I$(SGX_SDK)/include -I$(SGX_SDK)/include/libcxx -I$(SGX_SDK)/include/tlibc 

CFLAGS := -nostdinc -fvisibility=hidden -fpie -fstack-protector -fno-builtin-printf $(Enclave_Include_Paths)
CXXFLAGS := $(CFLAGS) -nostdinc++

Enclave_Security_Link_Flags := -Wl,-z,relro,-z,now,-z,noexecstack

Enclave_Link_Flags := $(Enclave_Security_Link_Flags) \
    -Wl,--no-undefined -nostdlib -nodefaultlibs -nostartfiles -L$(SGX_LIBRARY_PATH) \
	-Wl,--whole-archive -l$(Trts_Library_Name) -Wl,--no-whole-archive \
	-Wl,--start-group -lsgx_tstdc -lsgx_tcxx -lsgx_tcrypto -l$(Service_Library_Name) -Wl,--end-group \
	-Wl,-Bstatic -Wl,-Bsymbolic -Wl,--no-undefined \
	-Wl,-pie,-eenclave_entry -Wl,--export-dynamic  \
	-Wl,--defsym,__ImageBase=0 \
	-Wl,--version-script=$(Enclave_Version_Script)

OBJ_DIR := $(BUILD_DIR)/enclave
objs := uuid.o superinfo.o dirnode.o ecall.o
build_objs := $(addprefix $(OBJ_DIR)/, $(objs))
signed_enclave_img := $(LIB_DIR)/secfs_enclave.signed.so
enclave_img := $(LIB_DIR)/secfs_enclave.so

.PHONY: all clean

all: $(enclave_img)

$(enclave_img): $(OBJ_DIR)/enclave_t.o $(build_objs)
	@echo ---------- link $@ ----------
	$(CXX) $^ $(Enclave_Link_Flags) -o $@
	@echo

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(build_objs): $(OBJ_DIR)/%.o: %.cpp $(OBJ_DIR)/enclave_t.h
	@echo ---------- build $@ ----------
	$(CXX) $< -c $(CXXFLAGS) -o $@
	@echo

$(OBJ_DIR)/enclave_t.o: $(OBJ_DIR)/enclave_t.c
	@echo ---------- build $@ ----------
	$(CC) $< -c $(CFLAGS) -o $@
	@echo

$(OBJ_DIR)/enclave_t.c: $(OBJ_DIR)/enclave_t.h

$(OBJ_DIR)/enclave_t.h: $(SGX_EDGER8R) enclave.edl
	$(SGX_EDGER8R) --trusted --trusted-dir $(OBJ_DIR) enclave.edl --search-path . --search-path $(SGX_SDK)/include

clean:
	rm -f $(OBJ_DIR)/enclave_t.* $(build_objs) $(enclave_img) $(signed_enclave_img)