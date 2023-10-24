.PHONY: all install clean uninstall app enclave tests

all: app enclave

app:
	make -C $@

enclave:
	make -C $@

tests:
	make -C $@

clean:
	make -C app clean
	make -C enclave clean
	make -C tests clean
