DIRS=lib core

all:
	for i in $(DIRS);do                          \
		(cd $$i && make -f Makefile ) || exit 1; \
	done
clean:
	for i in $(DIRS);do  \
	    (cd $$i && make -f Makefile clean) || exit 1; \
	done
