SGX_SDK ?= /opt/intel/sgxsdk
SGX_MODE ?= SIM
SGX_ARCH ?= x64
SGX_DEBUG ?= 1

BASE_DIR ?= $(PWD)
BUILD_DIR ?= $(BASE_DIR)/build
BIN_DIR ?= $(BUILD_DIR)/bin
LIB_DIR ?= $(BUILD_DIR)/lib
INCLUDE_DIR ?= $(BUILD_DIR)/include
OBJ_DIR ?= $(BUILD_DIR)/obj

C ?= gcc
CXX ?= g++
