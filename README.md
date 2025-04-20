# LykOS
This is the main repository of LykOS, an open source hobbyist operating system targeting 64-bit architectures.

### Implemented and Planned Features
#### General
- [x] Virtual Memory
- [x] Slab allocator
- [x] Kernel modules
- [ ] Userspace
  - [x] Ring 3
  - [x] ELF loading
  - [ ] Basic system calls
  - [ ] ...
- [ ] ACPI support
- [ ] Basic graphical environment
#### Tasking
- [x] SMP (multiprocessing)
- [ ] Multitasking
  - [x] Non-preemtive
  - [ ] Preemtive
- [ ] IPC
#### Drivers
- [ ] AHCI
- [ ] NVMe
- [ ] RTL8139/8169
- [ ] PS2 keyboard
- [ ] PS2 mouse
- [ ] USB
  - [ ] Keyboard
  - [ ] Mouse
  - [ ] Mass storage
  - [ ] ...
#### Filesystems
- [x] VFS
- [x] Initrd
- [ ] Ext2
- [ ] FAT32
- [ ] /dev
- [ ] /mod
- [ ] /proc
