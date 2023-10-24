#include "sgx_error.h"

#include <map>

const char* enclave_err_msg(sgx_status_t ret) {
    static std::map<sgx_status_t, const char*> sgx_errlist = {
        {SGX_ERROR_UNEXPECTED, "Unexpected error occurred."},
        {SGX_ERROR_INVALID_PARAMETER, "Invalid parameter."},
        {SGX_ERROR_OUT_OF_MEMORY, "Out of memory."},
        {SGX_ERROR_ENCLAVE_LOST, "Power transition occurred."},
        {SGX_ERROR_INVALID_ENCLAVE, "Invalid enclave image."},
        {SGX_ERROR_INVALID_ENCLAVE_ID, "Invalid enclave identification."},
        {SGX_ERROR_INVALID_SIGNATURE, "Invalid enclave signature."},
        {SGX_ERROR_OUT_OF_EPC, "Out of EPC memory."},
        {SGX_ERROR_NO_DEVICE, "Invalid SGX device."},
        {SGX_ERROR_MEMORY_MAP_CONFLICT, "Memory map conflicted."},
        {SGX_ERROR_INVALID_METADATA, "Invalid enclave metadata."},
        {SGX_ERROR_DEVICE_BUSY, "SGX device was busy."},
        {SGX_ERROR_INVALID_VERSION, "Enclave version was invalid."},
        {SGX_ERROR_INVALID_ATTRIBUTE, "Enclave was not authorized."},
        {SGX_ERROR_ENCLAVE_FILE_ACCESS, "Can't open enclave file."},
        {SGX_ERROR_NDEBUG_ENCLAVE,
         "The enclave is signed as product enclave, and can not be created as debuggable enclave."},
        {SGX_ERROR_MEMORY_MAP_FAILURE, "Failed to reserve memory for the enclave."},
    };
    if (sgx_errlist.count(ret) != 0) {
        return sgx_errlist[ret];
    } else {
        return "Unknown error occurred";
    }
}
