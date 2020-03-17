#!/usr/local/cs/bin/python3

import sys
import math
import string
import argparse
import csv

class RefBlock:
    """ 
        Class to keep track of inode number and block offset     
    """

    def __init__(self, inode_num, offset, block_type):
        """
            self.m_inode_num: string containing the inode number
            self.m_offset: string containing the offset
            self.m_block_type: int describing block type:
                0 for normal, 1 for singly indirect, 2 for double, 3 for triple
        """
        self.m_inode_num = inode_num
        self.m_offset = offset
        self.m_block_type = block_type

def main():
    # "global" constants
    debug = 0
    NORMAL_EXIT = 0
    ARGS_ERROR = 1
    OTHER_ERROR = 2
    INVALID_FS_ERROR = 2
    printed_something = 0

    if len(sys.argv) != 2:
        print('lab 3B: missing file to analyze, or too many parameters provided', file=sys.stderr)
        sys.exit(1)
    file_path = sys.argv[1]
    if debug:
        print('Analyzing file: ' +file_path, file=sys.stderr)

    try:
        input_file = open(file_path, 'r')
    except IOError:
        print('Lab 3B: failed to open file ' +file_path, file=sys.stderr)
        sys.exit(ARGS_ERROR)
    
    file_lines = input_file.readlines()

    bfree_lines = []
    ifree_lines = []
    dirent_lines = []
    inode_lines = []
    indirect_lines = []
    superblock_line = None
    groupdesc_line = None
    # group and superblock are individual lines

    num_blocks = -1
    num_inodes = -1
    first_allowed_inode_num = -1
    first_allowed_block_num = -1
    
    for line in file_lines:
        if line[:5] == 'IFREE':
            ifree_lines.append(line[:-1])
        elif line[:5] == 'BFREE':
            bfree_lines.append(line[:-1])
        elif line[:6] == 'DIRENT':
            dirent_lines.append(line[:-1])
        elif line[:5] == 'INODE':
            inode_lines.append(line[:-1])
        elif line[:5] == 'GROUP':
            groupdesc_line = line[:-1]
        elif line[:10] == 'SUPERBLOCK':
            superblock_line = line[:-1]
        elif line[:8] == 'INDIRECT':
            indirect_lines.append(line[:-1])
        elif line == '\n':
            continue
        else:
            print('Lab 3B: invalid line found in input file', file=sys.stderr)
            sys.exit(OTHER_ERROR)
    
    superblock_info = superblock_line.split(',')
    # print(str(superblock_info))
    groupdesc_info = groupdesc_line.split(',')
    
    num_blocks = int(superblock_info[1])
    num_inodes = int(superblock_info[2])
    first_allowed_inode_num = int(superblock_info[7])
    first_allowed_block_num = (int)(groupdesc_info[8])+math.ceil((int)(superblock_info[4])*(int)(superblock_info[2])/(int)(superblock_info[3]))
    last_allowed_block_num = num_blocks - 1
    # print('First allowed block number: ' + str(first_allowed_block_num), file=sys.stderr)

    all_blocks = set()
    referenced_blocks = set()
    free_blocks = set()
    referenced_blocks_count = {}

    inode_link_count = {}
    all_inodes = set()
    free_inodes = set()

    for i in range(first_allowed_block_num, last_allowed_block_num+1):
        all_blocks.add(i)

    for i in range(first_allowed_inode_num, num_inodes+1):
        all_inodes.add(i)

    for line in ifree_lines:
        inode_num = int(line[6:])
        free_inodes.add(inode_num)
        all_inodes.discard(inode_num)
    
    dirents_grouped = {}
    for line in dirent_lines:
        dirent_entry = line.split(',')
        parent_inode_num = int(dirent_entry[1])
        if parent_inode_num not in dirents_grouped:
            dirents_grouped[parent_inode_num] = []
        dirents_grouped[parent_inode_num].append(dirent_entry)

    found_dot = False
    found_dot_dot = False
    parent_mappings = {}
    parent_mappings[2] = 2

    for dir_files in dirents_grouped.values():
        for dirent_entry in dir_files:
            inode_num = int(dirent_entry[3])
            
            if inode_num not in inode_link_count:
                inode_link_count[inode_num] = 0
            inode_link_count[inode_num] += 1

    for line in inode_lines:
        inode_entry = line.split(',')
        inode_num = int(inode_entry[1])
        all_inodes.discard(inode_num)
        if inode_num in free_inodes:
            print('ALLOCATED INODE ' + inode_entry[1] + ' ON FREELIST')
            printed_something = 1
            free_inodes.discard(inode_num)
        
        if inode_num not in inode_link_count:
            # for if inode_num is a reserved inode, or if it never appeared
            print('INODE ' + inode_entry[1] + ' HAS 0 LINKS BUT LINKCOUNT IS ' + inode_entry[6])
            printed_something = 1
        elif int(inode_entry[6]) != inode_link_count[inode_num]:
            print('INODE ' + inode_entry[1] + ' HAS ' + str(inode_link_count[inode_num]) + ' LINKS BUT LINKCOUNT IS ' + inode_entry[6])
            printed_something = 1

        # symlink, only 12 entries and no blocks to check
        if len(inode_entry) == 12:
            continue
        
        for i in range(12):
            block_num = int(inode_entry[12+i])
            if block_num == 0:
                continue
            elif block_num < 0 or block_num>last_allowed_block_num:
                print('INVALID BLOCK ' + inode_entry[12+i] + ' IN INODE ' + inode_entry[1] + ' AT OFFSET ' + str(i))
                printed_something = 1
            elif block_num>0 and block_num < first_allowed_block_num:
                print('RESERVED BLOCK ' + inode_entry[12+i] + ' IN INODE ' + inode_entry[1] + ' AT OFFSET ' + str(i))
                printed_something = 1
            else:
                all_blocks.discard(block_num)
                referenced_blocks.add(block_num)
                if block_num not in referenced_blocks_count:
                    referenced_blocks_count[block_num] = []
                block_info = RefBlock(inode_entry[1], str(i), 0)
                referenced_blocks_count[block_num].append(block_info)
        # print(str(i))
        block_num = int(inode_entry[24]) #singly indirect block
        if block_num < 0 or block_num>last_allowed_block_num:
            print('INVALID INDIRECT BLOCK ' + inode_entry[24] + ' IN INODE ' + inode_entry[1] + ' AT OFFSET 12')
            printed_something = 1
        elif block_num>0 and block_num < first_allowed_block_num:
            print('RESERVED INDIRECT BLOCK ' + inode_entry[24] + ' IN INODE ' + inode_entry[1] + ' AT OFFSET 12')
            printed_something = 1
        elif block_num != 0:
            all_blocks.discard(block_num)
            referenced_blocks.add(block_num)
            if block_num not in referenced_blocks_count:
                referenced_blocks_count[block_num] = []
            block_info = RefBlock(inode_entry[1], '12', 1)
            referenced_blocks_count[block_num].append(block_info)

        block_num = int(inode_entry[25]) #doubly indirect block
        if block_num < 0 or block_num>last_allowed_block_num:
            print('INVALID DOUBLE INDIRECT BLOCK ' + inode_entry[25] + ' IN INODE ' + inode_entry[1] + ' AT OFFSET 268')
            printed_something = 1
        elif block_num>0 and block_num < first_allowed_block_num:
            print('RESERVED DOUBLE INDIRECT BLOCK ' + inode_entry[25] + ' IN INODE ' + inode_entry[1] + ' AT OFFSET 268')
            printed_something = 1
        elif block_num != 0:
            all_blocks.discard(block_num)
            referenced_blocks.add(block_num)
            if block_num not in referenced_blocks_count:
                referenced_blocks_count[block_num] = []
            block_info = RefBlock(inode_entry[1], '268', 2)
            referenced_blocks_count[block_num].append(block_info)

        block_num = int(inode_entry[26]) #TRIPLY indirect block
        if block_num < 0 or block_num>last_allowed_block_num:
            print('INVALID TRIPLE INDIRECT BLOCK ' + inode_entry[26] + ' IN INODE ' + inode_entry[1] + ' AT OFFSET 65804')
            printed_something = 1
        elif block_num>0 and block_num < first_allowed_block_num:
            print('RESERVED TRIPLE INDIRECT BLOCK ' + inode_entry[26] + ' IN INODE ' + inode_entry[1] + ' AT OFFSET 65804')
            printed_something = 1
        elif block_num != 0:
            all_blocks.discard(block_num)
            referenced_blocks.add(block_num)
            if block_num not in referenced_blocks_count:
                referenced_blocks_count[block_num] = []
            block_info = RefBlock(inode_entry[1], '65804', 3)
            referenced_blocks_count[block_num].append(block_info)


    # directory consistency stuff
    for dir_files in dirents_grouped.values():
        for dirent_entry in dir_files:
            inode_num = int(dirent_entry[3])
            if inode_num < 1 or inode_num > num_inodes:
                print('DIRECTORY INODE ' + dirent_entry[1] + " NAME " + dirent_entry[6] + " INVALID INODE " +dirent_entry[3])
                printed_something = 1
            elif inode_num in free_inodes:
                print('DIRECTORY INODE ' + dirent_entry[1] + " NAME " + dirent_entry[6] + " UNALLOCATED INODE " +dirent_entry[3])
                printed_something = 1
            
            if dirent_entry[6] != "'.'" and dirent_entry[6] != "'..'":
                parent_mappings[int(dirent_entry[3])] = int(dirent_entry[1])

    # . and .. cases
    for dir_files in dirents_grouped.values():
        for dirent_entry in dir_files:
            if dirent_entry[6] == "'.'" and dirent_entry[3] != dirent_entry[1]:
                print('DIRECTORY INODE ' + dirent_entry[1] + " NAME '.' LINK TO INODE " + dirent_entry[3] + ' SHOULD BE ' + dirent_entry[1])
                printed_something = 1
            elif dirent_entry[6] == "'..'" and int(dirent_entry[3]) != parent_mappings[int(dirent_entry[1])]:
                print('DIRECTORY INODE ' + dirent_entry[1] + " NAME '..' LINK TO INODE " + dirent_entry[3] + ' SHOULD BE ' 
                    + str( parent_mappings[int( dirent_entry[1] )] ) )
                printed_something = 1
    
    for line in indirect_lines:
        indirect_entry = line.split(',')
        block_num = int(indirect_entry[5])

        # can you have an block number of 0 for an indirect block???
        if block_num == 0:
            continue
        elif block_num < 0 or block_num > last_allowed_block_num:
            if indirect_entry[2] == '1':
                print('INVALID INDIRECT BLOCK ' + indirect_entry[5] + ' IN INODE ' + indirect_entry[1] + ' AT OFFSET ' + indirect_entry[4])
                printed_something = 1
            elif indirect_entry[2] == '2':
                print('INVALID DOUBLE INDIRECT BLOCK ' + indirect_entry[5] + 
                    ' IN INODE ' + indirect_entry[1] + ' AT OFFSET ' + indirect_entry[4])
                printed_something = 1
            elif indirect_entry[2] == '3':
                print('INVALID TRIPLE INDIRECT BLOCK ' + indirect_entry[5] + 
                    ' IN INODE ' + indirect_entry[1] + ' AT OFFSET ' + indirect_entry[4])
                printed_something = 1
            else:
                print('Invalid level of block indirection found', file=sys.stderr)
                sys.exit(OTHER_ERROR)
        elif block_num > 0 and block_num < first_allowed_block_num:
            if indirect_entry[2] == '1':
                print('RESERVED INDIRECT BLOCK ' + indirect_entry[5] + ' IN INODE ' + indirect_entry[1] + ' AT OFFSET ' + indirect_entry[4])
                printed_something = 1
            elif indirect_entry[2] == '2':
                print('RESERVED DOUBLE INDIRECT BLOCK ' + indirect_entry[5] + 
                    ' IN INODE ' + indirect_entry[1] + ' AT OFFSET ' + indirect_entry[4])
                printed_something = 1
            elif indirect_entry[2] == '3':
                print('RESERVED TRIPLE INDIRECT BLOCK ' + indirect_entry[5] + 
                    ' IN INODE ' + indirect_entry[1] + ' AT OFFSET ' + indirect_entry[4])
                printed_something = 1
            else:
                print('Invalid level of block indirection found', file=sys.stderr)
                sys.exit(OTHER_ERROR)
        else:
            all_blocks.discard(block_num)
            referenced_blocks.add(block_num)
            if indirect_entry[2] == '1':
                if block_num not in referenced_blocks_count.keys():
                    referenced_blocks_count[block_num] = []
                block_info = RefBlock(indirect_entry[1], indirect_entry[4], 1)
                referenced_blocks_count[block_num].append(block_info)
            elif indirect_entry[2] == '2':  
                if block_num not in referenced_blocks_count.keys():
                    referenced_blocks_count[block_num] = []
                block_info = RefBlock(indirect_entry[1], indirect_entry[4], 2)
                referenced_blocks_count[block_num].append(block_info)
            elif indirect_entry[2] == '3':
                if block_num not in referenced_blocks_count.keys():
                    referenced_blocks_count[block_num] = []
                block_info = RefBlock(indirect_entry[1], indirect_entry[4], 3)
                referenced_blocks_count[block_num].append(block_info)
   
    for line in bfree_lines:
        block_num = int(line[6:])
        all_blocks.discard(block_num)
        free_blocks.add(block_num)

    alloc_on_free_list = referenced_blocks.intersection(free_blocks)
    
    for block_num in all_blocks:
        print('UNREFERENCED BLOCK ' + str(block_num))
        printed_something = 1
    # print(str(referenced_blocks))
    # print(str(free_blocks))

    for block_num in alloc_on_free_list:
        print('ALLOCATED BLOCK ' + str(block_num) + ' ON FREELIST')
        printed_something = 1

    for key in referenced_blocks_count.keys():
        if len(referenced_blocks_count[key])>1:
            reference_list = referenced_blocks_count[key]
            for entry in reference_list:
                if entry.m_block_type == 0:
                    print('DUPLICATE BLOCK ' + str(key) + ' IN INODE ' + entry.m_inode_num + ' AT OFFSET ' + entry.m_offset)
                    printed_something = 1
                elif entry.m_block_type == 1:
                    print('DUPLICATE INDIRECT BLOCK ' + str(key) + ' IN INODE ' + entry.m_inode_num + ' AT OFFSET ' + entry.m_offset)
                    printed_something = 1
                elif entry.m_block_type == 2:    
                    print('DUPLICATE DOUBLE INDIRECT BLOCK ' + str(key) + ' IN INODE ' + entry.m_inode_num + ' AT OFFSET ' + entry.m_offset)
                    printed_something = 1
                elif entry.m_block_type == 3:
                    print('DUPLICATE TRIPLE INDIRECT BLOCK ' + str(key) + ' IN INODE ' + entry.m_inode_num + ' AT OFFSET ' + entry.m_offset)
                    printed_something = 1
    # print(str(all_blocks))

    # what if we get inode with type 0? check how to deal with this case
    for inode_num in all_inodes:
        print('UNALLOCATED INODE ' + str(inode_num) + ' NOT ON FREELIST')
        printed_something = 1

    if printed_something != 0:
        sys.exit(OTHER_ERROR)
    
    sys.exit(NORMAL_EXIT)

if __name__ == "__main__":
    main()
