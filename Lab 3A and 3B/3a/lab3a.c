#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
#include <stdint.h>

#include "ext2_fs.h"

#define NORMAL_EXIT 0
#define SYS_CALL_ERROR 1
#define ARGS_ERROR 1
#define OTHER_ERROR 2

#define DEBUG 0 //toggle switch for debugging mode

#define SUPERBLOCK_START 1024

#define EXT2_S_IFDIR 0x4000
#define EXT2_S_IFREG 0x8000
#define EXT2_S_IFLNK 0xA000

__u32 blockSize;
int numBlockNumsPerBlock;
int fd;

typedef struct ext2_super_block ext2_super_block_t;
typedef struct ext2_dir_entry ext2_dir_entry_t;

int processDirInBlock(int blockNum, int logicalByteOffset, int parentInodeNum);
int singleBlockDir(int blockNumber, int logicalByteOffset, int parentInodeNum);
int doubleBlockDir(int blockNumber, int logicalByteOffset, int parentInodeNum);
int tripleBlockDir(int blockNumber, int logicalByteOffset, int parentInodeNum);

void singleBlock(int inodeNumber, __u32 blockNumber, int logicalBlockOffset){
    // inode number, blockNumber (if 1, block #12; if 2, block 13, etc.),
    // logical block offset: self explanatory
    __u32 blockNumbers[numBlockNumsPerBlock];
    int newBase = blockSize*blockNumber;
    pread(fd, &blockNumbers, blockSize, newBase);
    for(int j = 0; j<numBlockNumsPerBlock; j++){
        if(blockNumbers[j] != 0){
            printf("INDIRECT,%d,1,%d,%d,%d\n", inodeNumber, logicalBlockOffset+j, blockNumber, blockNumbers[j]);
        }
    }
}

void doubleBlock(int inodeNumber, __u32 blockNumber, int logicalBlockOffset){
    __u32 doubBlockNumbers[numBlockNumsPerBlock];
    int newBase = blockSize*blockNumber;
    pread(fd, &doubBlockNumbers, blockSize, newBase);
    for(int j = 0; j<numBlockNumsPerBlock; j++){
        if(doubBlockNumbers[j] != 0){
            printf("INDIRECT,%d,2,%d,%d,%d\n", inodeNumber, logicalBlockOffset+j, blockNumber, doubBlockNumbers[j]);
            singleBlock(inodeNumber, doubBlockNumbers[j], logicalBlockOffset+j);
        }
    }
}

void tripleBlock(int inodeNumber, __u32 blockNumber, int logicalBlockOffset){
    __u32 tripBlockNumbers[numBlockNumsPerBlock];
    int newBase = blockSize*blockNumber;
    pread(fd, &tripBlockNumbers, blockSize, newBase);
    for(int j = 0; j<numBlockNumsPerBlock; j++){
        if(tripBlockNumbers[j] != 0){
            printf("INDIRECT,%d,3,%d,%d,%d\n", inodeNumber, logicalBlockOffset+j, blockNumber, tripBlockNumbers[j]);
            doubleBlock(inodeNumber, tripBlockNumbers[j], logicalBlockOffset+j);
        }
    }
}

int main(int argc, char** argv){
    if(argc != 2){
        fprintf(stderr, "lab 3A: Missing file system name to analyze\n");
        exit(ARGS_ERROR);
    }
    fd = open(argv[1], O_RDONLY);
    if(fd<0){
        fprintf(stderr, "lab 3A: Failed to open disk image: %s\n", strerror(errno));
        exit(SYS_CALL_ERROR);
    }


    ext2_super_block_t superBlock;


    if(pread(fd, &superBlock, sizeof(ext2_super_block_t), SUPERBLOCK_START) < 0){
        fprintf(stderr, "lab 3A: Failed to read from disk image: %s\n", strerror(errno));
        exit(SYS_CALL_ERROR);
    }

    if(superBlock.s_magic != EXT2_SUPER_MAGIC){
        fprintf(stderr, "lab 3A: file is not valid EXT2 image\n");
        exit(OTHER_ERROR);
    }

    //print superblock stats
    blockSize = 1024 << superBlock.s_log_block_size;
    const __u32 inodeSize = superBlock.s_inode_size;
    printf("SUPERBLOCK,");
    printf("%d,%d,%d,%d,%d,%d,%d\n", superBlock.s_blocks_count,
            superBlock.s_inodes_count, blockSize,
            superBlock.s_inode_size, superBlock.s_blocks_per_group,
            superBlock.s_inodes_per_group, superBlock.s_first_ino);

    //group summary
    if(lseek(fd, SUPERBLOCK_START+blockSize, SEEK_SET)<0){
        fprintf(stderr, "lab 3A: Failed to adjust file offset: %s\n", strerror(errno));
        exit(SYS_CALL_ERROR);
    }
    
    typedef struct ext2_group_desc ext2_group_desc_t;
    ext2_group_desc_t groupDescriptor;
    if(read(fd, &groupDescriptor, sizeof(ext2_group_desc_t))<0){
        fprintf(stderr, "lab 3A: Failed to read from disk image: %s\n", strerror(errno));
        exit(SYS_CALL_ERROR);
    }
   
    printf("GROUP,0,");
    // *************************************************
    // is total blocks always equal to total blocks from superblock? Check!
    // *************************************************
    printf("%d,%d,%d,%d,%d,%d,%d\n", superBlock.s_blocks_count,
        superBlock.s_inodes_count, groupDescriptor.bg_free_blocks_count,
        groupDescriptor.bg_free_inodes_count, groupDescriptor.bg_block_bitmap,
        groupDescriptor.bg_inode_bitmap, groupDescriptor.bg_inode_table);
    
    // __u32 blockBitmapStart = groupDescriptor.bg_block_bitmap;
    
    //free blocks
    char* blockBitmap = (char*)malloc(blockSize*sizeof(char));
    if(blockBitmap == NULL){
        fprintf(stderr, "lab 3A: Failed to allocate memory\n");
        exit(OTHER_ERROR);
    }
    if(lseek(fd, blockSize*groupDescriptor.bg_block_bitmap, SEEK_SET)<0){
        fprintf(stderr, "lab 3A: Failed to adjust file offset: %s\n", strerror(errno));
        exit(SYS_CALL_ERROR);
    }
    if(read(fd, blockBitmap, blockSize)<0){
        fprintf(stderr, "lab 3A: Failed to read from disk image: %s\n", strerror(errno));
        exit(SYS_CALL_ERROR);
    }


    __u32 currBlockNum = 0;
    int shouldBreak = 0;
    for(__u32 i = 0; i<blockSize && !shouldBreak; i++){
        int byteChunk = blockBitmap[i];
        for(__u32 j = 8; j>=1; j--){
            if(currBlockNum >= superBlock.s_blocks_count){
                shouldBreak = 1;
                break;
            }
            if(DEBUG){
                fprintf(stderr, "Analyzing block %d\n", currBlockNum);
            }
            int status = byteChunk&0x80;
            if(status == 0){
                printf("BFREE,%d\n", 8*i+j);
            }
            byteChunk <<= 1;
            currBlockNum++;
        }
    }

    free(blockBitmap);

    //free inodes
    // __u32 inodeBitMapStart = groupDescriptor.bg_inode_bitmap;
    char* inodeBitmap = (char*)malloc(blockSize*sizeof(char));
    if(inodeBitmap == NULL){
        fprintf(stderr, "lab 3A: Failed to allocate memory\n");
        exit(OTHER_ERROR);
    }
    if(lseek(fd, blockSize*groupDescriptor.bg_inode_bitmap, SEEK_SET)<0){
        fprintf(stderr, "lab 3A: Failed to adjust file offset: %s\n", strerror(errno));
        exit(SYS_CALL_ERROR);
    }
    if(read(fd, inodeBitmap, blockSize)<0){
        fprintf(stderr, "lab 3A: Failed to read from disk image: %s\n", strerror(errno));
        exit(SYS_CALL_ERROR);
    }

    __u32 currInodeNum = 0;
    int shouldBreakInode = 0;

    for(__u32 i = 0; i<blockSize && !shouldBreakInode; i++){
        int byteChunk = inodeBitmap[i];
        for(__u32 j = 8; j>=1; j--){
            if(currInodeNum >= superBlock.s_inodes_count){
                shouldBreakInode = 1;
                break;
            }
            int status = byteChunk&0x80;
            if(status == 0){
                printf("IFREE,%d\n", 8*i+j);
            }
            byteChunk <<= 1;
            currInodeNum++;
        }
    }

    free(inodeBitmap);

    // inodes
    int inodeTableByteAddr = blockSize*groupDescriptor.bg_inode_table;
    typedef struct ext2_inode ext2_inode_t;
    ext2_inode_t currInode;
    if(lseek(fd, inodeTableByteAddr, SEEK_SET)<0){
        fprintf(stderr, "lab 3A: Failed to adjust file offset: %s\n", strerror(errno));
        exit(SYS_CALL_ERROR);
    }
    for(__u32 i = 0; i<superBlock.s_inodes_count; i++){
        if(read(fd, &currInode, inodeSize)<0){
            fprintf(stderr, "lab 3A: Failed to read from disk image: %s\n", strerror(errno));
            exit(SYS_CALL_ERROR);
        }

        if(currInode.i_mode == 0 || currInode.i_links_count == 0){
            continue;
        }
        printf("INODE,%d,", i+1); //+1 cause inodes start counting at 1
        __u32 inodeFileType = currInode.i_mode & 0xF000;
        if(inodeFileType == EXT2_S_IFREG){
            printf("f,");
        }
        else if(inodeFileType == EXT2_S_IFDIR){
            printf("d,");
        }
        else if(inodeFileType == EXT2_S_IFLNK){
            printf("s,");
        }
        else{
            printf("?,");
        }
        printf("%o,", currInode.i_mode&0xFFF);
        printf("%d,%d,%d,", currInode.i_uid, currInode.i_gid, currInode.i_links_count);

        time_t ctime = currInode.i_ctime;
        time_t atime = currInode.i_atime;
        time_t mtime = currInode.i_mtime;
        struct tm* formattedTime;
        char buffer[80];
        formattedTime = gmtime(&ctime);
        strftime(buffer, sizeof(buffer),"%m/%d/%y %H:%M:%S", formattedTime );
        printf("%s,", buffer);
        formattedTime = gmtime(&mtime);
        strftime(buffer, sizeof(buffer),"%m/%d/%y %H:%M:%S", formattedTime );
        printf("%s,", buffer);
        formattedTime = gmtime(&atime);
        strftime(buffer, sizeof(buffer),"%m/%d/%y %H:%M:%S", formattedTime );
        printf("%s,", buffer);

        printf("%d,%d", currInode.i_size, currInode.i_blocks);

        //print out the block #'s, whether it's file or directory or symlink >=60
        if( !(  inodeFileType == EXT2_S_IFLNK && currInode.i_size<60) ){
            printf(",");
            for(int i_block_index = 0; i_block_index<EXT2_N_BLOCKS-1; i_block_index++){
                // N_BLOCKS minus 1 b/c we want the last one printed without a comma
                printf("%d,", currInode.i_block[i_block_index]);
            }
            printf("%d\n", currInode.i_block[14]);
        }
        else{
            //symlink and length < 60: we're DONE
            printf("\n");
            continue;
        }

        numBlockNumsPerBlock = blockSize/sizeof(__u32);

        //print DIRENT with indirect blocks]

        // char nameBuf[256];
        if(inodeFileType == EXT2_S_IFDIR){
            if(DEBUG){
                fprintf(stderr, "Inode %d is a directory\n", i+1);
            }

            // ext2_dir_entry_t dirEntry;
            int logicalByteOffset = 0;
            for(__u32 j = 0; j<EXT2_NDIR_BLOCKS; j++){
                logicalByteOffset = processDirInBlock(currInode.i_block[j], logicalByteOffset, i+1);
                // if(currInode.i_block[j] == 0){
                //     continue;
                // }
                
                // if(DEBUG){
                //     fprintf(stderr, "Analyzing block # %d\n", j);
                // }

                // int blockBase = blockSize*currInode.i_block[j];
                // for(__u32 k = 0; k<blockSize; k += dirEntry.rec_len){
                //     pread(fd, &dirEntry, sizeof(ext2_dir_entry_t), blockBase+k);
                //     if(dirEntry.file_type == 0){
                //         break;
                //     }
                //     if(dirEntry.inode != 0 && dirEntry.name_len != 0){
                //         memcpy(&nameBuf, dirEntry.name, dirEntry.name_len);
                //         nameBuf[dirEntry.name_len] = '\0';
                //         // printf("BLOCK4: %d\n", j);
                //             printf("DIRENT,%d,%d,%d,%d,%d,'%s'\n", i+1, logicalByteOffset, 
                //                 dirEntry.inode, dirEntry.rec_len, 
                //                 dirEntry.name_len, nameBuf);
                //     }
                //     logicalByteOffset += dirEntry.rec_len;
                // }
            }
            if(currInode.i_block[12] != 0){
                logicalByteOffset = singleBlockDir(currInode.i_block[12], logicalByteOffset, i+1);
            }
            if(currInode.i_block[13] != 0){
                logicalByteOffset = doubleBlockDir(currInode.i_block[13], logicalByteOffset, i+1);
            }
            if(currInode.i_block[14] != 0){
                logicalByteOffset = tripleBlockDir(currInode.i_block[14], logicalByteOffset, i+1);
            }
        }

        if(DEBUG){
            fprintf(stderr, "%d:%d,%d,%d\n", i+1,currInode.i_block[12], currInode.i_block[13], currInode.i_block[14]);
        }

        //process indirect blocks
        //single block case
        if(currInode.i_block[12] != 0){
            if(DEBUG){
                fprintf(stderr, "single indirect for block %d\n", currInode.i_block[12]);
            }
            singleBlock(i+1, currInode.i_block[12], 12);
        }
        if(currInode.i_block[13] != 0){
            doubleBlock(i+1, currInode.i_block[13], 268);
        }
        if(currInode.i_block[14] != 0){
            tripleBlock(i+1, currInode.i_block[14], 65804);
        }
    }
    if(close(fd)<0){
            fprintf(stderr, "lab 3A: error in closing file descriptor: %s\n", strerror(errno)); 
            exit(SYS_CALL_ERROR);
    }

    exit(NORMAL_EXIT);
}

int processDirInBlock(int blockNum, int logicalByteOffset, int parentInodeNum){
    int blockBase = blockSize*blockNum;
    ext2_dir_entry_t dirEntry;
    char nameBuf[256];
    for(__u32 k = 0; k<blockSize; k += dirEntry.rec_len){
        pread(fd, &dirEntry, sizeof(ext2_dir_entry_t), blockBase+k);
        if(dirEntry.file_type == 0){
            break;
        }
        if(dirEntry.inode != 0 && dirEntry.name_len != 0){
            memcpy(&nameBuf, dirEntry.name, dirEntry.name_len);
            nameBuf[dirEntry.name_len] = '\0';
            // printf("BLOCK4: %d\n", j);
                printf("DIRENT,%d,%d,%d,%d,%d,'%s'\n", parentInodeNum, logicalByteOffset, 
                    dirEntry.inode, dirEntry.rec_len, 
                    dirEntry.name_len, nameBuf);
        }
        logicalByteOffset += dirEntry.rec_len;
    }
    return logicalByteOffset;
}

int singleBlockDir(int blockNumber, int logicalByteOffset, int parentInodeNum){
    // inode number, blockNumber (if 1, block #12; if 2, block 13, etc.),
    // logical block offset: self explanatory
    __u32 blockNumbers[numBlockNumsPerBlock];
    int newBase = blockSize*blockNumber;
    pread(fd, &blockNumbers, blockSize, newBase);
    for(int j = 0; j<numBlockNumsPerBlock; j++){
        if(blockNumbers[j] != 0){
            logicalByteOffset = processDirInBlock(blockNumbers[j], logicalByteOffset, parentInodeNum);
        }
    }
    return logicalByteOffset;
}

int doubleBlockDir(int blockNumber, int logicalByteOffset, int parentInodeNum){
        // inode number, blockNumber (if 1, block #12; if 2, block 13, etc.),
    // logical block offset: self explanatory
    __u32 blockNumbers[numBlockNumsPerBlock];
    int newBase = blockSize*blockNumber;
    pread(fd, &blockNumbers, blockSize, newBase);
    for(int j = 0; j<numBlockNumsPerBlock; j++){
        if(blockNumbers[j] != 0){
            logicalByteOffset = singleBlockDir(blockNumbers[j], logicalByteOffset, parentInodeNum);
        }
    }
    return logicalByteOffset;
}

int tripleBlockDir(int blockNumber, int logicalByteOffset, int parentInodeNum){
    __u32 blockNumbers[numBlockNumsPerBlock];
    int newBase = blockSize*blockNumber;
    pread(fd, &blockNumbers, blockSize, newBase);
    for(int j = 0; j<numBlockNumsPerBlock; j++){
        if(blockNumbers[j] != 0){
            logicalByteOffset = doubleBlockDir(blockNumbers[j], logicalByteOffset, parentInodeNum);
        }
    }
    return logicalByteOffset;
}
