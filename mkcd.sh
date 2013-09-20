#!/bin/bash
cp kernel.bin isofiles/boot/kernel.bin
mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o bootable.iso isofiles
wodim -v dev=/dev/sr0 -blank=fast
wodim -v dev=/dev/sr0 speed=0 bootable.iso
