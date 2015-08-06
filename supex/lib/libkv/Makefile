# top level Makefile


DIR_KV=./src


MAKE=make
MAKE_DIR=make -C




all:DEST_KV

DEST_KV:
	$(MAKE_DIR) $(DIR_KV)


release:
	$(MAKE_DIR) $(DIR_KV) release


doc:
	doxygen Doxyfile



clean:
	$(MAKE_DIR) $(DIR_KV) clean
	$(MAKE_DIR) ./jemalloc/ clean 


push:
	git push origin HEAD:refs/for/master

.PHONY: release doc clean 
