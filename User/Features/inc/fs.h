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

void flashfs_remove(const char* path);

void flashfs_move(const char* from, const char* to);

void flashfs_file(const char* path);

uint32_t flashfs_file_open(const char* path, int flags);

void flashfs_file_close(uint32_t fh);

void flashfs_file_read(uint32_t fh, uint8_t* buffer, uint32_t size);

void flashfs_file_write(uint32_t fh, uint8_t* buffer, uint32_t size) ;

int flashfs_file_seek(uint32_t fh, uint32_t offset, uint32_t whence) ;

void flashfs_file_truncate(uint32_t fh, uint32_t size);

int flashfs_file_size(uint32_t fh);

void flashfs_mkdir(const char* path);

#endif