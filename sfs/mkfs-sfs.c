#include <stdio.h>
#include <sys/type.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#inlcude "sfs.h"

int main(int argc, char* argv[]){
    int fd;
    sszie_t ret;
    uint64_t welcome_inode_no;
    uint64_t welcome_data_block_no_offset;

    fd = open(argv[1], O_RDRW);
    if (fd == -1){
        peror("error opening the device");
        return -1;
    }

    // construct superblock
    struct sfs_superblock sfs_sb = {
        .version = 1,
        .maginc = SFS_MAGIC,
        .blocksize = SFS_DEFAULT_BLOCKSIZE,
        .inode_table_size = SFS_DEFAULT_INODE_TABLE_SIZE,
        .inode_count = 2,
        
    }
    return 0;
}
