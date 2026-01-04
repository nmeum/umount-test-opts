# umount-test-opts

This repository contains files to perform equivalence testing of the `-O` option implementation for `umount(8)` shipped with [BusyBox](https://busybox.net/) and [util-linux](https://git.kernel.org/cgit/utils/util-linux/util-linux.git).

## Background

The `-O` option provided by `umount(8)` unmounts file systems by mount options.
This is implemented by checking the mount options of a file system against a provided pattern.
The function that performs this comparison is called [`mnt_match_options()`](https://cdn.kernel.org/pub/linux/utils/util-linux/v2.37/libmount-docs/libmount-Options-string.html#mnt-match-options) in util-linux.
Vanilla BusyBox does not provide this feature, but I have written a patch that [adds this feature to BusyBox](http://lists.busybox.net/pipermail/busybox/2022-June/089769.html).
The corresponding function is called `fsopts_matches` in my patch.
This is an attempt to check if my BusyBox implementation of this feature is "correct".
This setup has found one [undefined behavior bug in util-linux](https://git.kernel.org/pub/scm/utils/util-linux/util-linux.git/commit/?id=06ee5267516761721ebfbdfa313980cef8e54c66) and two functional errors in [my BusyBox patch](http://lists.busybox.net/pipermail/busybox/2023-March/090193.html).

## Setup

The setup provided here uses [symbolic execution](https://en.wikipedia.org/wiki/Symbolic_execution) with [KLEE](https://klee.github.io) to ensure that both implementations always return the same value for the same input strings via an assertion in the test harness:

    assert(mnt_match_options(options, pattern) == fsopts_matches(options, pattern));

This is a well-known idea that is already mentioned in the [original KLEE paper](https://www.usenix.org/legacy/events/osdi08/tech/full_papers/cadar/cadar.pdf) (Section 5.5).
For two 8 byte inputs, KLEE finds 46588712 execution paths for this assertion under the constraints mentioned below.
On all of these execution paths, the assertion holds.
On a AMD Ryzen 5700, this takes about 4 to 5 minutes.

## Usage

Ideally, use this repository with [Guix](https://guix.gnu.org):

    $ guix time-machine -C channels.scm -- shell

Within the Guix shell run the following commands:

    $ make sim

Alternatively, you can also try to use KLEE's [Docker](https://www.docker.io/) image:

    $ docker run --rm -it klee/klee -v $(PWD):/code

Within the container run:

    $ cd /code
    $ make sim

## Limitations

* The equivalence checked is only performed for fixed-size strings of length `N`.
* Support for `key=value` mount options is not implemented in BusyBox, hence the input is constrained to `[-,+A-Za-z0-9]`.
* The inputs always have a length of `INPUT_SIZE` since the aforementioned constraint does not include null terminators.
