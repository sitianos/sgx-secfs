include ../build.mk

ifeq ($(SGX_MODE), SIM)
	Urts_LIBS := -lsgx_urts_sim -lsgx_uae_service_sim
else ifeq ($(SGX_MODE), HW)
	Urts_LIBS := -lsgx_urts -lsgx_uae_service
# else
# 	$(error SGX_MODE should be set to SIM or HW)
endif

SGX_APP_CFlAGS := -fPIC -Wno-attributes -I$(SGX_SDK)/include

ifeq ($(SGX_DEBUG), 1)
	SGX_APP_CFlAGS += -DDEBUG -UNDEBUG -UEDEBUG
else ifeq ($(SGX_PRERELEASE), 1)
	SGX_APP_CFlAGS += -DNDEBUG -DEDEBUG -UDEBUG
else
	SGX_APP_CFlAGS += -DNDEBUG -UEDEBUG -UDEBUG
endif

INCLUDES := $(addprefix -I, . ../include) $(shell pkg-config fuse3 --cflags)
LIBS := $(shell pkg-config fuse3 --libs)

CFLAGS := $(SGX_COMMON_CFLAGS) $(SGX_APP_CFlAGS) $(INCLUDES)
CXXFLAGS := $(SGX_COMMON_CXXFLAGS) $(SGX_APP_CFlAGS) $(INCLUDES)

LDFLAGS := -L$(SGX_LIBRARY_PATH) $(Urts_LIBS) $(LIBS) -Wl,--rpath $(SGX_LIBRARY_PATH)

objs := main.o fuse_interface.o
build_objs := $(addprefix $(OBJ_DIR)/, $(objs))
exec := $(BIN_DIR)/secfs

$(warning $(build_objs))

.PHONY: all clean

all: $(exec)

$(build_objs): $(OBJ_DIR)/%.o: %.cpp
	@echo build $@
	$(CXX) $^ -c $(CXXFLAGS) -o $@

$(exec): $(build_objs)
	$(CXX) $^ $(LDFLAGS) -o $@

clean:
	rm -f $(build_objs) $(exec)
