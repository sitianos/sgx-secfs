# Security of Filesystem

## Security of enclave
### enclave environment
- Enclave is an isolated region within a userspace application
- OS and VM cannnot access memory inside the enclave 

### Sealing
- Sealing is a way of persisting sensitive data across execution
- Code within enclave can generate sealing key which is accessible only inside the enclave
- When enclave code takes sensitive data outside the enclave, it can use sealing key

### Remote Attestation
- Users can validate the integrity of remove enclave

## Replay Attack
### Substitution Attack
- The owner of storage server may try to replace data on storage with old data
- 

## Oblivious Access
### Access Pattern Attack
- Even if we uses enclave for secure computation, clients can infer correspondence between file/directory and UUID of metadata by obsearving ECALL and OCALL
- When malicious clients collude with malicious server, server can detect when and who accesses specified file/directory

### ORAM
- To hide access pattern from malicious clients or server, it utilizes oblibious RAM (ORAM)
- By randomizing access order or adding dummy data to request, it disables adversaries to infer access pattern
- We uses Path ORAM algorithm

## Out of Scope
### Availability
- Attackers who have access permission to storage can corrupt the filesystem, though they cannnot compromise confidentiality and integrity.
- Attackers also can perform a DoS attack on the storage server
- Provider of this filesystem should protect against these attacks by forcing access control over the storage service
