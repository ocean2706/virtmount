# virtmount
r/w userspace/kernel module for mounting .vdi, .vmdk, .vhd, .vhdx, like on qemu-nbd 

## Goals
User must be able to setup a virtual disk image as local device tree
Also, the linux kernel must be able to rootmount a virtual disk partition
Some existing userspace tools can be used but also more lowlevel kernel routines must be available.

Assuming vdiskload is a tool that can be used to inform linux kernel about disk format and partitions, the following commands will be available:

vdiskload file.vmdk | file.vdi | file.qcow | file.qcow2 | file.vhd | file.vhdx |file.img | other future file formats
or, you can ansamble a multiple partition images (sparse .img or compressed archive(@planning)) as a single disk, using a complex file definition (see .json sample)
vdiskload partitiontable_or_disk_definition.json

after loading, the disk will be available as /dev/vdi0, /dev/vmdk0 and partitions as  

hdparam -i /dev/vdi0 or  equivalent  must report the disk by default in a similar way that losetup report disks. In the future, maybe lowlevel device emulation can be added in order to emulate directly in the kernel of some "standard" (sata,scsi, usb) controlers in order to control better the behaviour of your disk ( without running full emulation ). This require additional research.
The main goal is to behave as loop device, but with the following advantages:
-allow to use-it as root partition (kernel parameters)
-boot grub from it ?
-inform the kernel as it was normal (removable) disk - this, alongside the first goal is the main difference against qemu-nbd. Also, the tools must make minimal custom syscalls, load minimal drivers, etc, in order to make the kernel to do the most of the work as much as possible using standard syscalls.
-minimal requirements for building it.

## Competitors
Qemu is used in libguestfs. What that means ? In order to guestmount a disk using libguestfs, behind the scene you will have a small vm under qemu that will run. This is introducing additional overhead in terms of time and resourcess, just only for simple reading from a vdisk 
Microsoft Windows 10 allow boot from special prepared vmdx at this time. 

## OS Support
Linux first.
FreeBSD
hope Windows, in order to allow booting from other disk types.

