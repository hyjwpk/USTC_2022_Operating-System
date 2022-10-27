#ifndef FAT16_H
#define FAT16_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define FUSE_USE_VERSION 26
#include <fuse.h>

/* Unit size */
#define BYTES_PER_SECTOR 512
#define BYTES_PER_DIR 32

// 簇号（FAT表项）
#define CLUSTER_FREE    0x0000  // 未分配的簇号
#define CLUSTER_MIN     0x0002  // 第一个可以用的簇号
#define CLUSTER_MAX     0xFFEF  // 最后一个可用的簇号
#define CLUSTER_END     0xffff  // 文件结束的簇号

/* File property */
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20

#define MAX_SHORT_NAME_LEN 13

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;

/* FAT16 BPB Structure */
typedef struct {
  BYTE BS_jmpBoot[3];
  BYTE BS_OEMName[8];
  WORD BPB_BytsPerSec;
  BYTE BPB_SecPerClus;
  WORD BPB_RsvdSecCnt;
  BYTE BPB_NumFATS;
  WORD BPB_RootEntCnt;
  WORD BPB_TotSec16;
  BYTE BPB_Media;
  WORD BPB_FATSz16;
  WORD BPB_SecPerTrk;
  WORD BPB_NumHeads;
  DWORD BPB_HiddSec;
  DWORD BPB_TotSec32;
  BYTE BS_DrvNum;
  BYTE BS_Reserved1;
  BYTE BS_BootSig;
  DWORD BS_VollID;
  BYTE BS_VollLab[11];
  BYTE BS_FilSysType[8];
  BYTE Reserved2[448];
  WORD Signature_word;
} __attribute__ ((packed)) BPB_BS;

/* FAT Directory Structure */
typedef struct {
  BYTE DIR_Name[11];
  BYTE DIR_Attr;
  BYTE DIR_NTRes;
  BYTE DIR_CrtTimeTenth;
  WORD DIR_CrtTime;
  WORD DIR_CrtDate;
  WORD DIR_LstAccDate;
  WORD DIR_FstClusHI;
  WORD DIR_WrtTime;
  WORD DIR_WrtDate;
  WORD DIR_FstClusLO;
  DWORD DIR_FileSize;
} __attribute__ ((packed)) DIR_ENTRY;

/* FAT16 volume data with a file handler of the FAT16 image file */
typedef struct
{
  FILE *fd;                   // 镜像文件指针
  DWORD FirstRootDirSecNum;   // 根目录区域所在扇区号
  DWORD FirstDataSector;      // 首个数据区域所在扇区号
  DWORD FatOffset;            // 文件分配表（FAT）所在的偏移量（字节）
  DWORD RootOffset;           // 根目录所在区域的偏移量（字节）
  DWORD DataOffset;           // 数据区域的偏移量（字节）
  DWORD FatSize;              // 单个FAT的大小
  DWORD ClusterSize;          // 单个簇的大小(字节)
  BPB_BS Bpb;
} FAT16;  // 存储发文件系统所需要的元数据的数据结构

void sector_read(FILE *fd, unsigned int secnum, void *buffer);
void sector_write(FILE *fd, unsigned int secnum, const void *buffer);
size_t io_read(FILE *fd, void *buf, long offset, size_t size);
size_t io_write(FILE *fd, const void *buf, long offset, size_t size);

int find_root(FAT16 *, DIR_ENTRY *Dir, const char *path, off_t *dir_offset);
int find_subdir(FAT16 *, DIR_ENTRY *Dir, char **paths, int pathDepth, int curDepth, off_t *dir_offset);
char **path_split(const char *pathInput, int *pathSz);
BYTE *path_decode(BYTE *);
char **org_path_split(char *pathInput);
char *get_prt_path(const char *path, const char **orgPaths, int pathDepth);


FAT16 *pre_init_fat16(const char* imageFilePath);
WORD fat_entry_by_cluster(FAT16 *fat16_ins, WORD ClusterN);
void first_sector_by_cluster(FAT16 *fat16_ins, WORD ClusterN, WORD *FatClusEntryVal, WORD *FirstSectorofCluster, BYTE *buffer);
long get_cluster_offset(FAT16 *fat16_ins, uint16_t cluster);
int dir_entry_create(FAT16 *fat16_ins, int sectorNum, int offset, char *Name, BYTE attr, WORD firstClusterNum, DWORD fileSize);
int free_cluster(FAT16 *fat16_ins, int ClusterNum);

void *fat16_init(struct fuse_conn_info *conn);
void fat16_destroy(void *data);
int fat16_getattr(const char *path, struct stat *stbuf);
int fat16_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi);
int fat16_read(const char *path, char *buffer, size_t size, off_t offset,
               struct fuse_file_info *fi);
int fat16_mknod(const char *path, mode_t mode, dev_t devNum);
int fat16_unlink(const char *path);
int fat16_utimens(const char *path, const struct timespec tv[2]);
int fat16_mkdir(const char *path, mode_t mode);
int fat16_rmdir(const char *path);
int fat16_write(const char *path, const char *data, size_t size, off_t offset,
                struct fuse_file_info *fi);
int fat16_truncate(const char *path, off_t size);

void run_tests();

#endif