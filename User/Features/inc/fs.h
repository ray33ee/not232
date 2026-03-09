#ifndef _FS_FLASH_TINYFS
#define _FS_FLASH_TINYFS


#include "stdint.h"
#include "ch32v20x.h"
#include "FAT/inc/lfs.h"

typedef struct {
    lfs_file_t file;
    uint32_t taken;
} FLASHFS_FH;

typedef struct {
    lfs_dir_t file;
    uint32_t taken;
} FLASHFS_DH;


void flashfs_init();

uint32_t flashfs_remove(uint8_t* path);

uint32_t flashfs_move(uint8_t* from, uint8_t* to);

uint32_t flashfs_file_info(uint8_t* path);

uint32_t flashfs_file_open(uint8_t* path, int flags);

uint32_t flashfs_file_close(uint32_t fh);

uint32_t flashfs_file_read(uint32_t fh, uint8_t* buffer, uint32_t size);

uint32_t flashfs_file_write(uint32_t fh, uint8_t* buffer, uint32_t size) ;

uint32_t flashfs_file_seek(uint32_t fh, uint32_t offset, uint32_t whence) ;

uint32_t flashfs_file_truncate(uint32_t fh, uint32_t size);

uint32_t flashfs_mkdir(uint8_t* path);

uint32_t flashfs_dir_open(uint8_t* path);

uint32_t flashfs_dir_close(uint32_t dh);

uint32_t flashfs_dir_read(uint32_t dh);

#endif