#ifndef __SFS_H__
#define __SFS_H__

#define BITS_IN_BYTE 8
#define SFS_MAGIC 0x20160105                  // magic num
#define SFS_DEFAULT_BLOCKSIZE 4096            // block size
#define SFS_DEFAULT_INODE_TABLE_SIZE 1024     // inode table size
#define SFS_DEFAULT_DATA_BLOCK_TABLE_SIZE 1024// data block table size
#define SFS_FILENAME_MAXLEN 255               // max len of filename

extern struct mutex sfs_sb_lock;

struct sfs_dir_record {
    char filename[SFS_FILENAME_MAXLEN];
    uint64_t inode_no;
};

struct sfs_inode {
    mode_t mode;
    uint64_t inode_no;
    uint64_t data_block_no;

    union {
        uint64_t file_size;
        uint64_t dir_children_count;
    };
};

struct sfs_superblock {
    uint64_t version;
    uint64_t magic;
    uint64_t blocksize;

    uint64_t inode_table_size;
    uint64_t inode_count;

    uint64_t data_block_table_size;
    uint64_t data_block_count;
};

static const uint64_t SFS_SUPERBLOCK_BLOCK_NO = 0;
static const uint64_t SFS_INODE_BITMAP_BLOCK_NO = 1;
static const uint64_t SFS_DATA_BLOCK_BITMAP_BLOCK_NO = 2;
static const uint64_t SFS_INODE_TABLE_START_BLOCK_NO = 3;

static const uint64_t SFS_ROOTDIR_INODE_NO = 0;
// data block no is the absolute block number from start of device
// data block no offset is the relative block offset from start of data block table

static const uint64_t HELLOFS_ROOTDIR_DATA_BLOCK_NO_OFFSET = 0;

// hepler functions
static inline uint64_t SFS_INODES_PER_BLOCK_HSB(struct sfs_superblock* sfs_sb) {
    return sfs_sb->blocksize / sizeof(struct sfs_inode);
}


static inline uint64_t SFS_DATA_BLOCK_TABLE_START_BLOCK_NO_HSB(struct sfs_superblock* sfs_sb) {
    return SFS_INODE_TABLE_START_BLOCK_NO + sfs_sb->inode_table_size / SFS_INODES_PER_BLOCK_HSB(sfs_sb) + 1;
}



#endif // __SFS_H__
