CLANG ?= clang
GCC   ?= gcc

KLEE_INCLUDE = /home/klee/klee_src/include
KLEE_LIB     = /home/klee/klee_build/lib

CPPFLAGS += -I$(KLEE_INCLUDE)
CFLAGS += -g -O0 -Xclang -disable-O0-optnone

INPUT_SIZE ?= 8
CFLAGS_SIZE = -DINPUT_SIZE=$(INPUT_SIZE)

main.bc: main.c busybox.h util-linux.h
# Binary for replying generated test inputs (for debugging etc.).
# See: https://klee.github.io/tutorials/testing-function/#replaying-a-test-case
main: main.c busybox.h util-linux.h
	$(GCC) $(CPPFLAGS) -g3 -L$(KLEE_LIB) $< -o $@ $(CFLAGS_SIZE)

%.bc: %.c
	$(CLANG) -emit-llvm -c -o $@ $< $(CPPFLAGS) $(CFLAGS) $(CFLAGS_SIZE)

sim: main.bc
	klee --solver-backend=z3 \
		--max-memory=$(shell expr 1024 \* 1024 \* 1024) \
		--exit-on-error \
		--libc=uclibc \
		--posix-runtime \
		--check-div-zero=false \
		--check-overshift=false \
		$<

clean:
	rm -f main main.bc

.PHONY: sim clean
