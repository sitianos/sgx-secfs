BUILD_DIR ?= ../build
include ../build.mk

########## SGX APP Configuration ##########
ifeq ($(SGX_MODE), SIM)
	Urts_LIBS := -lsgx_urts_sim -lsgx_uae_service_sim
else ifeq ($(SGX_MODE), HW)
	Urts_LIBS := -lsgx_urts -lsgx_uae_service
else
$(error SGX_MODE should be set to SIM or HW)
endif

SGX_APP_CFLAGS := -fPIC -Wno-attributes -I$(SGX_SDK)/include

ifeq ($(SGX_DEBUG), 1)
	SGX_APP_CFLAGS += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
	SGX_APP_CFLAGS += -DNDEBUG -DEDEBUG -UDEBUG
else
	SGX_APP_CFLAGS += -DNDEBUG -UEDEBUG -UDEBUG
endif

INCLUDES := $(addprefix -I, . ../include) $(shell pkg-config fuse3 --cflags)
LIBS := $(shell pkg-config fuse3 --libs) -lmbedtls -lmbedcrypto

CFLAGS := $(SGX_COMMON_CFLAGS) $(SGX_APP_CFLAGS) $(INCLUDES)
CXXFLAGS := $(SGX_COMMON_CXXFLAGS) $(SGX_APP_CFLAGS) $(INCLUDES)

LDFLAGS := -L$(SGX_LIBRARY_PATH) -Wl,--rpath $(SGX_LIBRARY_PATH) $(Urts_LIBS) $(LIBS)

OBJ_DIR := $(BUILD_DIR)/app
objs := main.o cli.o fuse_operations.o volume.o enclave.o ocall.o backend/local.o
build_objs := $(addprefix $(OBJ_DIR)/, $(objs))
main_objs := main.o fuse_operations.o volume.o enclave.o ocall.o backend/local.o
cli_objs := cli.o volume.o enclave.o ocall.o backend/local.o
exec := $(BIN_DIR)/secfs $(BIN_DIR)/secfs_cli

.PHONY: all clean

all: $(OBJ_DIR) $(exec)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/backend
	mkdir -p $(BIN_DIR)

# $(exec): $(build_objs) $(OBJ_DIR)/enclave_u.o
# 	@echo ---------- link $@ ----------
# 	$(CXX) $^ $(LDFLAGS) -o $@
# 	@echo

$(BIN_DIR)/secfs: $(addprefix $(OBJ_DIR)/, $(main_objs)) $(OBJ_DIR)/enclave_u.o
	@echo ---------- link $@ ----------
	$(CXX) $^ $(LDFLAGS) -o $@
	@echo

$(BIN_DIR)/secfs_cli: $(addprefix $(OBJ_DIR)/, $(cli_objs)) $(OBJ_DIR)/enclave_u.o
	@echo ---------- link $@ ----------
	$(CXX) $^ $(LDFLAGS) -o $@
	@echo

$(build_objs): $(OBJ_DIR)/%.o: %.cpp
	@echo ---------- build $@ ----------
	$(CXX) $< -c $(CXXFLAGS) -o $@
	@echo

$(OBJ_DIR)/enclave_u.o: enclave_u.c
	@echo ---------- build $@ ----------
	$(CC) $< -c $(CFLAGS) -o $@
	@echo

enclave_u.c: enclave_u.h

enclave_u.h: $(SGX_EDGER8R) ../enclave/enclave.edl
	@echo ---------- generate $@ ----------
	$(SGX_EDGER8R) --untrusted ../enclave/enclave.edl --search-path . --search-path $(SGX_SDK)/include
	@echo

clean:
	rm -f $(build_objs) $(OBJ_DIR)/enclave_u.o $(exec)
