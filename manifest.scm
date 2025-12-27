(use-modules (guix packages)
             (gnu packages base)
             (gnu packages check)
             (gnu packages linux)
             (gnu packages llvm))

(packages->manifest
  (list
    coreutils
    clang-13
    gnu-make
    klee
    linux-libre-headers
    llvm-13))
