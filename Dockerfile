FROM klee/klee

ADD --chown=klee:klee . /home/klee/umount
CMD su - klee
