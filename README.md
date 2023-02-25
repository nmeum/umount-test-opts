# umount-test-opts

This repository contains files to perform equivalence testing of the `-O` option implementation for `umount(8)` shipped with [BusyBox](https://busybox.net/) and [util-linux](https://git.kernel.org/cgit/utils/util-linux/util-linux.git).

## Background

The `-O` option provided by `umount(8)` unmounts file systems by mount options.
This is implemented by checking the mount options of a file system against a provided pattern.
The function which performs this comparison is called [`mnt_match_options()`](https://cdn.kernel.org/pub/linux/utils/util-linux/v2.37/libmount-docs/libmount-Options-string.html#mnt-match-options) in util-linux.
Vanilla BusyBox does not provide this feature but I have written a patch which [adds this feature to Busybox](http://lists.busybox.net/pipermail/busybox/2022-June/089769.html).
The corresponding function is called `fsopts_matches` in my patch.
This is an attempt to check if my BusyBox implementation of this feature is "correct".

## Setup

The setup provided here uses [symbolic execution](https://en.wikipedia.org/wiki/Symbolic_execution) with [KLEE](https://klee.github.io) to ensure that both implementations always return the same value for the same input strings via `assert(3)`:

    assert(mnt_match_options(options, pattern) == fsopts_matches(options, pattern));

This is a well-known idea which is already mentioned in the [original KLEE paper](https://www.usenix.org/legacy/events/osdi08/tech/full_papers/cadar/cadar.pdf) (Section 5.5).

## Usage

This repository uses the KLEE [Docker](https://www.docker.io/) image, to use this image run:

    $ docker build -t klee-umount
    $ docker run -it klee-umount

Within the container run the following commands:

    $ cd umount
    $ make sim

This will execute the code with KLEE.

## Limitations

* The equivalence checked is only performed for fixed-size strings of length `N`.
* The input is heavily constrained since util-linux returns wild values otherwise.
  For example, util-linux is of the opinion that empty mount options match the pattern `"`.
