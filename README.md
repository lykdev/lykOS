# LykOS
This is the main repository of LykOS, an open source hobbyist operating system targeting 64-bit architectures.

### Implemented and Planned Features
A feature is considered implemented if it exists in any working form, even if rudimentary or incomplete.
#### General
- [x] Virtual memory
- [x] Slab allocator
- [x] Kernel modules
- [ ] Userspace
  - [x] Ring 3
  - [x] ELF loading
  - [x] Basic system calls
  - [ ] ...
- [ ] ACPI support
- [ ] Basic graphical environment
#### Tasking
- [x] SMP (multiprocessing)
- [x] Multitasking
  - [x] Non-preemtive
  - [x] Preemtive
- [ ] IPC
  - [ ] Pipes
  - [ ] Sockets
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
- [x] /dev
- [ ] /mod
- [ ] /proc
