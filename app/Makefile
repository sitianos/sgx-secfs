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

INCLUDES := $(addprefix -I, ../include) $(shell pkg-config fuse3 --cflags)
LIBS := $(shell pkg-config fuse3 --libs)

CFLAGS := $(SGX_COMMON_CFLAGS) $(SGX_APP_CFLAGS) $(INCLUDES)
CXXFLAGS := $(SGX_COMMON_CXXFLAGS) $(SGX_APP_CFLAGS) $(INCLUDES)

LDFLAGS := -L$(SGX_LIBRARY_PATH) $(Urts_LIBS) $(LIBS) -Wl,--rpath $(SGX_LIBRARY_PATH)

OBJ_DIR := $(BUILD_DIR)/app
objs := main.o fuse_operations.o volume.o enclave.o backend/local.o
build_objs := $(addprefix $(OBJ_DIR)/, $(objs))
exec := $(BIN_DIR)/secfs

.PHONY: all clean

all: $(OBJ_DIR) $(build_objs) $(exec)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)
	mkdir -p $(OBJ_DIR)/backend
	mkdir -p $(BIN_DIR)

$(build_objs): $(OBJ_DIR)/%.o: %.cpp
	@echo ---------- build $@ ----------
	$(CXX) $< -c $(CXXFLAGS) -o $@
	@echo

$(exec): $(build_objs)
	@echo ---------- link $@ ----------
	$(CXX) $^ $(LDFLAGS) -o $@
	@echo

clean:
	rm -f $(build_objs) $(exec)
