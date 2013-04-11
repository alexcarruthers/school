#include <slack/std.h>
#include <slack/list.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "sfs_api.h"
#include "disk_emu.h"

#define disk_name "sfs_filesystem"
#define NUM_BLOCKS 256
#define BLOCK_SIZE 2048
#define MAX_FILE_BLOCKS 64 //64 blocks
#define MAX_NUM_FILES 128
#define MAX_FILE_NAME_LENGTH 12

#define unused -1
#define eof -1
#define open 0
#define read_ptr 1
#define write_ptr 2
#define block_index 0
#define next 1

typedef int bool;
#define true 1
#define false 0

typedef struct{
	char name[13];
	bool created;
	int size;
	int fatIndex;
}file;

int fat[NUM_BLOCKS][2]; //fat table
int fdt[MAX_NUM_FILES][3];
file files[MAX_NUM_FILES];

List *free_blocks;

int firstFreeFat(){
	int i;
	for (i=0; i<NUM_BLOCKS; i++){
		if (fat[i][block_index] == unused){
			return i;
		}
	}
	return -1;
}

void write_fs_blocks(){
	write_blocks(0,1, &files);
	write_blocks(1,1, &fat);
	write_blocks(NUM_BLOCKS-1, 1, &free_blocks);
}

void mksfs(int fresh){
	int i;
	if (fresh){
		init_fresh_disk(disk_name, BLOCK_SIZE, NUM_BLOCKS);
		
		free_blocks = list_create(NULL);
		
		for (i=2; i<NUM_BLOCKS-1; i++){
			list_append_int(free_blocks, i);
		}
		for (i=0; i<NUM_BLOCKS; i++){
			fat[i][block_index] = unused;
			fat[i][next] = eof;
		}
		for (i=0; i<MAX_NUM_FILES; i++){
			files[i].created = false;
			files[i].fatIndex = eof;
		}
	}
	
	else{
		init_disk(disk_name, BLOCK_SIZE, NUM_BLOCKS);
		read_blocks(0,1, &files);
		read_blocks(1,1, &fat);
		read_blocks(NUM_BLOCKS-1, 1, &free_blocks);
	}
	for(i=0; i<MAX_NUM_FILES; i++){
		fdt[i][open] = false;
		fdt[i][read_ptr] = false;
		fdt[i][write_ptr] = files[i].size;
	}
}

void sfs_ls(void){
	printf("_____ROOT_____\n");
	printf("Name\t\t\tsize\n");
	int i=0;
	for(i=0; i<MAX_NUM_FILES; i++){
		if (files[i].created){
			printf("%s\t%iBytes\n", files[i].name, files[i].size);
		}
	}
}
int sfs_open(char *name){
	int i, found = false;
	for (i=0; i<MAX_NUM_FILES; i++){
		if (files[i].created && strcmp(name, files[i].name) == 0){
			found = true;
			break;
		}
	}
	if (found){ //now have to open the file
		fdt[i][open]=true;
		fdt[i][read_ptr]=0;
		fdt[i][write_ptr]=files[i].size;
		return i;
	}
	
	else{ //now have to create a new file
		//find the next free file id
		int fileid = -1;
		for (i=0; i<MAX_NUM_FILES; i++){
			if (files[i].created == -1){
				fileid = i;
				break;
			}
		}
		if (fileid == -1) return -1;
		
		//set all the file information
		strcpy(files[fileid].name, name);
		files[fileid].created = true;
		int nextFat = firstFreeFat();
		if (nextFat == -1) return -1;
		files[fileid].fatIndex = nextFat;
		files[fileid].size = 0;
		fat[files[fileid].fatIndex][block_index] = unused;
		fat[files[fileid].fatIndex][next] = eof;
		
		//write all the system blocks
		write_fs_blocks();
		
		fdt[fileid][open]=true;
		fdt[fileid][read_ptr]=0;
		fdt[fileid][write_ptr]=0;
		return fileid;
	}
}
int sfs_close(int fileID){
	if(fdt[fileID][open] == false) return -1;
	if(fileID > MAX_NUM_FILES || fileID < 0) return -1;
	fdt[fileID][open]=false;
	fdt[fileID][read_ptr]=0;
	fdt[fileID][write_ptr]=0;
	return 0;
}
int sfs_write(int fileID, char *buf, int length){
	if(fdt[fileID][open] == false) return -1;
	if(fileID > MAX_NUM_FILES || fileID < 0) return -1;
	
	int fatEntry = files[fileID].fatIndex;
	//if this is the first time writing to this file, give it a fat index
	if(fat[fatEntry][block_index] == unused){
		fat[fatEntry][block_index] = list_shift_int(free_blocks);
		fat[fatEntry][next] = eof;
	}
	
	//we must find the block where the file ends
	int numWrittenBlocks = files[fileID].size % BLOCK_SIZE;
	
	int i;
	for(i=0; i < numWrittenBlocks; i++)
		fatEntry = fat[fatEntry][next];
	
	//need to append to the rest of this block
	char block[BLOCK_SIZE];
	read_blocks(fat[fatEntry][block_index], 1, &block);
	int numBytes;
	if (length < (BLOCK_SIZE - files[fileID].size % BLOCK_SIZE))
		numBytes = length;
	else
		numBytes = BLOCK_SIZE - files[fileID].size % BLOCK_SIZE;
	
	memcpy(block + (files[fileID].size % BLOCK_SIZE), buf, numBytes);
	write_blocks(fat[fatEntry][block_index], 1, &block);
	int numBytesWritten = numBytes;
	
	//now we can write entire blocks until no more bytes need to be
	//written
	while(numBytesWritten < length){
		fat[fatEntry][next] = firstFreeFat();
		fatEntry = fat[fatEntry][next];
		fat[fatEntry][block_index] = list_shift_int(free_blocks);
		fat[fatEntry][next] = eof;
		write_blocks(fat[fatEntry][block_index], 1, buf+numBytesWritten);
		if((length - numBytesWritten) >= BLOCK_SIZE){
			numBytesWritten +=BLOCK_SIZE;
		}
		else{
			numBytesWritten += length-numBytesWritten;
		}
	}
	
	//update sizes and pointers and write file system blocks to disk
	files[fileID].size += numBytesWritten;
	fdt[fileID][write_ptr] += numBytesWritten;
	write_fs_blocks();
	return numBytesWritten;
}
int sfs_read(int fileID, char *buf, int length);
