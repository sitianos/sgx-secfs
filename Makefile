include build.mk

.PHONY: all install clean uninstall frontend

all: frontend # enclave backend

frontend: 
	make -C $@

enclave:
	make -C enclave

backend: 
	make -C backend

clean:
	make -C frontend clean
