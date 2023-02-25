CLANG ?= clang
GCC   ?= gcc

KLEE_INCLUDE = /home/klee/klee_src/include
KLEE_LIB     = /home/klee/klee_build/lib

CPPFLAGS += -I$(KLEE_INCLUDE)
CFLAGS += -g -O0 -Xclang -disable-O0-optnone

main.bc: main.c
main.c: util-linux.h busybox.h

# Binary for replying generated test inputs (for debugging etc.).
# See: https://klee.github.io/tutorials/testing-function/#replaying-a-test-case
main: main.c
	$(GCC) $(CPPFLAGS) -g3 -L$(KLEE_LIB) $< -o $@ -lkleeRuntest

%.bc: %.c
	$(CLANG) -emit-llvm -c -o $@ $< $(CPPFLAGS) $(CFLAGS)

sim: main.bc
	klee --solver-backend=z3 \
		--exit-on-error \
		--libc=uclibc \
		--posix-runtime \
		$<

clean:
	rm -f main main.bc

.PHONY: sim clean
