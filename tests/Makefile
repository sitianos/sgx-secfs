BUILD_DIR ?= ../build
include ../build.mk

INCLUDES := $(addprefix -I, ../include $(SGX_SDK)/include) $(shell pkg-config fuse3 --cflags)
LIBS := $(shell pkg-config fuse3 --libs)

CFLAGS := $(INCLUDES)
CXXFLAGS := $(INCLUDES)

LDFLAGS := $(LIBS)

TEST_DIR := $(BUILD_DIR)/tests
APP_DIR := $(BUILD_DIR)/app
ENCLAVE_DIR := $(BUILD_DIR)/enclave

objs := test_storage.o test_volume.o test_uuid.o
test_objs := $(addprefix $(TEST_DIR)/, $(objs))
test_execs := $(patsubst %.o, $(TEST_DIR)/%, $(objs))

.PHONY: all clean

all: $(TEST_DIR) $(test_execs)

$(TEST_DIR):
	mkdir -p $(TEST_DIR)

$(APP_DIR)/%.o:
	make -C ../app $@

$(ENCLAVE_DIR)/%.o:
	make -C ../enclave $@

$(test_objs): $(TEST_DIR)/%.o: %.cpp
	@echo ---------- build $@ ----------
	$(CXX) $^ -c $(CXXFLAGS) -o $@
	@echo

$(TEST_DIR)/test_storage: $(APP_DIR)/backend/local.o $(TEST_DIR)/test_storage.o
	$(CXX) $^ $(LDFLAGS) -o $@

$(TEST_DIR)/test_volume: $(APP_DIR)/volume.o $(APP_DIR)/enclave.o $(APP_DIR)/backend/local.o $(TEST_DIR)/test_volume.o
	$(CXX) $^ $(LDFLAGS) -o $@

$(TEST_DIR)/test_uuid: $(ENCLAVE_DIR)/uuid.o $(TEST_DIR)/test_uuid.o
	$(CXX) $^ $(LDFLAGS) -o $@

$(TEST_DIR)/test_dirent: $(ENCLAVE_DIR)/uuid.o $(ENCLAVE_DIR)/dirent.o $(TEST_DIR)/test_dirent.o
	$(CXX) $^ $(LDFLAGS) -o $@

clean:
	rm -f $(test_objs) $(test_execs)
