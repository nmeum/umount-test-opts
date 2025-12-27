(list (channel
        (name 'nmeum)
        (url "https://github.com/nmeum/guix-channel.git")
        (branch "master")
        (commit
          "b821950bb6155c6f3d322280645071edc2bff1f2")
        (introduction
          (make-channel-introduction
            "808a00792c114c5c1662e8b1a51b90a2d23f313a"
            (openpgp-fingerprint
              "514E 833A 8861 1207 4F98  F68A E447 3B6A 9C05 755D"))))
      (channel
        (name 'guix)
        (url "https://git.guix.gnu.org/guix.git")
        (branch "master")
        (commit
          "aff9ce37616997647e7124edb0b3636ef98be68b")
        (introduction
          (make-channel-introduction
            "9edb3f66fd807b096b48283debdcddccfea34bad"
            (openpgp-fingerprint
              "BBB0 2DDF 2CEA F6A8 0D1D  E643 A2A0 6DF2 A33A 54FA")))))
