#Grub2 Vloopback module
The grub2 driver allow grub2 to read from virtual disk files ( vmdk, vdi, img, qemu), however this will be usable only for "vdisk" aware kernels to boot from.
The reason is that vdisk is not directly accesible from bios and because of that,  the kernel must also know to access the virtual disk.
It works in a similar way to loopback module, however there are some lowlevel differences regarding the way the grub2 will be informed about
the size of the disk, partitions, etc.
At this moment the goals are:
-build vloopback module
-allow grub2 to load the module
-set the file as new device
Future goals (passing the right vpartition to kernel ) are in planning state
