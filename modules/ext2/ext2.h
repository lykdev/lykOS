#pragma once

#include <lib/def.h>

typedef struct
{
    u32  inodes_count;
    u32  blocks_count;
    u32  r_blocks_count;
    u32  free_blocks_count;
    u32  free_inodes_count;
    u32  first_data_block;
    u32  log_block_size;
    u32  log_frag_size;
    u32  blocks_per_group;
    u32  frags_per_group;
    u32  inodes_per_group;
    u32  mtime;
    u32  wtime;
    u16  mnt_count;
    u16  max_mnt_count;
    u16  magic;
    u16  state;
    u16  errors;
    u16  minor_rev_level;
    u32  lastcheck;
    u32  checkinterval;
    u32  creator_os;
    u32  rev_level;
    u16  def_resuid;
    u16  def_resgid;
}
__attribute__((packed))
ext2_super_block_t;
