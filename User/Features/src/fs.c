#include "Features/inc/fs.h"

lfs_t lfs;

#define MAX_FILE_HANDLES    (10)
#define MAX_DIR_HANDLES    (10)

FLASHFS_FH file_handles[MAX_FILE_HANDLES];
FLASHFS_DH dir_handles[MAX_DIR_HANDLES];


/* ------------------------------------------------------------------------- */
/* littlefs region in CH32 internal flash                                    */
/* First 64 KiB kept for program, next 160 KiB used for littlefs             */
/* ------------------------------------------------------------------------- */

#define LFS_FLASH_BASE_ADDR   ((uint32_t)0x08010000UL)
#define LFS_FLASH_SIZE        ((uint32_t)(160UL * 1024UL))

/*
    Use standard erase granularity: 4 KiB
    Use standard programming granularity: 2 bytes
*/
#define LFS_FLASH_BLOCK_SIZE  ((uint32_t)4096UL)
#define LFS_FLASH_BLOCK_COUNT ((lfs_size_t)(LFS_FLASH_SIZE / LFS_FLASH_BLOCK_SIZE))

/* ------------------------------------------------------------------------- */
/* Helpers                                                                   */
/* ------------------------------------------------------------------------- */

static int ch32_lfs_addr_valid(lfs_block_t block, lfs_off_t off, lfs_size_t size)
{
    if (block >= LFS_FLASH_BLOCK_COUNT) {
        return 0;
    }

    if ((off + size) > LFS_FLASH_BLOCK_SIZE) {
        return 0;
    }

    return 1;
}

static uint32_t ch32_lfs_phys_addr(lfs_block_t block, lfs_off_t off)
{
    return LFS_FLASH_BASE_ADDR
         + ((uint32_t)block * LFS_FLASH_BLOCK_SIZE)
         + (uint32_t)off;
}


int flash_read(
    const struct lfs_config *c,
    lfs_block_t block,
    lfs_off_t off,
    void *buffer,
    lfs_size_t size
) {
    uint32_t addr;

    (void)c;

    if (!ch32_lfs_addr_valid(block, off, size)) {
        return LFS_ERR_INVAL;
    }

    addr = ch32_lfs_phys_addr(block, off);

    memcpy(buffer, (const void *)addr, (size_t)size);
    return 0;
}

int flash_prog(
    const struct lfs_config *c,
    lfs_block_t block,
    lfs_off_t off,
    const void *buffer,
    lfs_size_t size
) {
    uint32_t addr;
    const uint8_t *src;
    lfs_size_t i;
    FLASH_Status status;

    (void)c;

    if (!ch32_lfs_addr_valid(block, off, size)) {
        return LFS_ERR_INVAL;
    }

    /*
        CH32 standard programming is halfword-based.
        Keep prog_size = 2 and cache_size/read_size aligned accordingly.
    */
    if (((off & 1U) != 0U) || ((size & 1U) != 0U)) {
        return LFS_ERR_INVAL;
    }

    addr = ch32_lfs_phys_addr(block, off);
    src = (const uint8_t *)buffer;

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_WRPRTERR);

    for (i = 0; i < size; i += 2U) {
        uint16_t half =
            (uint16_t)src[i]
            | ((uint16_t)src[i + 1U] << 8);

        status = FLASH_ProgramHalfWord(addr + (uint32_t)i, half);
        if (status != FLASH_COMPLETE) {
            FLASH_Lock();
            return LFS_ERR_IO;
        }
    }

    FLASH_Lock();
    return 0;
}

int flash_erase(
    const struct lfs_config *c,
    lfs_block_t block
) {
    uint32_t addr;
    FLASH_Status status;

    (void)c;

    if (block >= LFS_FLASH_BLOCK_COUNT) {
        return LFS_ERR_INVAL;
    }

    addr = ch32_lfs_phys_addr(block, 0);

    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_WRPRTERR);

    status = FLASH_ErasePage(addr);

    FLASH_Lock();

    if (status != FLASH_COMPLETE) {
        return LFS_ERR_IO;
    }

    return 0;
}

int flash_sync(const struct lfs_config *c)
{
    (void)c;
    return 0;
}

/* ------------------------------------------------------------------------- */
/* littlefs config                                                            */
/* ------------------------------------------------------------------------- */

const struct lfs_config cfg = {
    .read  = flash_read,
    .prog  = flash_prog,
    .erase = flash_erase,
    .sync  = flash_sync,

    .read_size      = 2,
    .prog_size      = 2,
    .block_size     = LFS_FLASH_BLOCK_SIZE,
    .block_count    = LFS_FLASH_BLOCK_COUNT,
    .cache_size     = 64,
    .lookahead_size = 16,
    .block_cycles   = 100
};

uint32_t allocate_fh() {
    for (int i = 0; i < MAX_FILE_HANDLES; i++) {
        if (file_handles[i].taken == 0) {
            file_handles[i].taken = 1;
            return i;
        }
    }
    return -1;
}

void free_fh(uint32_t fh) {
    file_handles[fh].taken = 0;
}

uint32_t allocate_dh() {
    for (int i = 0; i < MAX_DIR_HANDLES; i++) {
        if (dir_handles[i].taken == 0) {
            dir_handles[i].taken = 1;
            return i;
        }
    }
    return -1;
}

void free_dh(uint32_t fh) {
    dir_handles[fh].taken = 0;
}

void flashfs_init() {
    int err = lfs_mount(&lfs, &cfg);

    // reformat if we can't mount the filesystem
    // this should only happen on the first boot
    if (err) {
        lfs_format(&lfs, &cfg);
        lfs_mount(&lfs, &cfg);
    }
}

void flashfs_remove(const char* path) {
    lfs_remove(&lfs, path);
}

void flashfs_move(const char* from, const char* to) {
    lfs_rename(&lfs, from, to);
}

void flashfs_file(const char* path) {
    struct lfs_info thing;

    lfs_stat(&lfs, path, &thing);
}

uint32_t flashfs_file_open(const char* path, int flags) {
    uint32_t fh = allocate_fh();
    if (fh == -1) {
        return - 1;
    }
    lfs_file_open(&lfs, &(file_handles[fh].file), path, flags);
    return fh;
}

void flashfs_file_close(uint32_t fh) {
    lfs_file_close(&lfs, &(file_handles[fh].file));
    free_fh(fh);
}

void flashfs_file_read(uint32_t fh, uint8_t* buffer, uint32_t size) {
    lfs_file_read(&lfs, &(file_handles[fh].file), buffer, size);
}

void flashfs_file_write(uint32_t fh, uint8_t* buffer, uint32_t size) {
    lfs_file_write(&lfs, &(file_handles[fh].file), buffer, size);
}

int flashfs_file_seek(uint32_t fh, uint32_t offset, uint32_t whence) {
    return lfs_file_seek(&lfs, &(file_handles[fh].file), offset, whence);
}

void flashfs_file_truncate(uint32_t fh, uint32_t size) {
    lfs_file_truncate(&lfs, &(file_handles[fh].file), size);
}

int flashfs_file_size(uint32_t fh) {
    lfs_file_size(&lfs, &(file_handles[fh].file));
}

void flashfs_mkdir(const char* path) {
    lfs_mkdir(&lfs, path);
}