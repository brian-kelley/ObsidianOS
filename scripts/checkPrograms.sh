#!/bin/bash
for util in "grub-install" "fdisk" "losetup" "mkdosfs";
  do
    if which $util >/dev/null;
      then
        echo "Have $util."
    else
        echo "Do not have $util. Install it."
    fi
done
