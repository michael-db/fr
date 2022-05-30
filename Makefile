.PHONY: test
test: fr
	@export FRDIGITS=	# clean default test environment
	@ls test/*.test|xargs -n 1 test/run-test

fr: fr.c
	@VERSION="$$(./get-version.sh)"; \
	echo "Making $@, version=\"$${VERSION}\""; \
	sed 's|// VERSION|"\\nVersion: '"$${VERSION}"'\\n"|' $^ | \
	cc -o $@ -x c - -Wall -Wextra -std=c99 -pedantic \
	-Wmissing-prototypes -Wstrict-prototypes -O2 -Wconversion

install: test
	sudo cp fr /usr/bin
