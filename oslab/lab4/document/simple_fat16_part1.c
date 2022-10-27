#include <string.h>
#include <time.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/timeb.h>

#include "fat16.h"



const char *FAT_FILE_NAME = "fat16.img";

/**
 * @brief 读取扇区号为secnum的扇区，将数据存储到buffer中
 * 
 * @param fd      镜像文件指针
 * @param secnum  需要读取的扇区号
 * @param buffer  数据要存储到的缓冲区指针
 */
void sector_read(FILE *fd, unsigned int secnum, void *buffer)
{
  fseek(fd, BYTES_PER_SECTOR * secnum, SEEK_SET);
  fread(buffer, BYTES_PER_SECTOR, 1, fd);
}

/**
 * @brief 将buffer中的数据写入到扇区号为secnum的扇区中
 * 
 * @param fd      镜像文件指针
 * @param secnum  需要写入的扇区号
 * @param buffer  需要写入的数据
 */
void sector_write(FILE *fd, unsigned int secnum, const void *buffer)
{
  fseek(fd, BYTES_PER_SECTOR * secnum, SEEK_SET);
  fwrite(buffer, BYTES_PER_SECTOR, 1, fd);
  fflush(fd);
}

/**
 * @brief 从fuse中获取存储了文件系统元数据的FAT16指针

 * @return FAT16* 文件系统元数据指针 
 */
FAT16* get_fat16_ins() {
  struct fuse_context *context;
  context = fuse_get_context();
  return (FAT16 *)context->private_data;
}

/**
 * @brief 计算编号为cluster的簇在硬盘中的偏移量
 * 
 * @param fat16_ins FAT16元数据指针
 * @param cluster   簇号
 * @return long     簇在硬盘（镜像文件）中的偏移量
 */
long get_cluster_offset(FAT16 *fat16_ins, uint16_t cluster)
{
  if (cluster >= CLUSTER_MIN &&
      cluster <= CLUSTER_MAX &&
      cluster < (fat16_ins->FatSize / sizeof(uint16_t)))
  {
    return fat16_ins->DataOffset + fat16_ins->ClusterSize * (cluster - 2);
  }
  return -1;
}

/**
 * @brief 将输入路径按“/”分割成多个字符串，并按照FAT文件名格式转换字符串。
 * 
 * 例如：当pathInputConst为"/dir1/dir2/file.txt"时，函数将其分割为"dir1", "dir2", "file.txt"
 * 三层，三层文件名分别转换为"DIR1       ", "DIR2       "和"FILE    TXT"。
 * 所以函数将设置pathDepth_ret为3，并返回长度为3的字符串数组，返回如上如上所述的三个长度为11的字符串。
 * 当某层的文件名或拓展名过长时，会自动截断文件名和拓展名，
 * 如"/verylongname.extension"会被转换为"VERYLONGEXT"。
 * 若某层文件名为"."或者".."则会特殊处理，分别转换为".           "和"..        "，
 * 分别代表当前目录和父目录。
 * 
 * @param pathInputConst  输入的文件路径名, 如/home/user/m.c
 * @param pathDepth_ret   输出参数，将会被设置为输入路径的层数，如上面的例子中为3
 * @return char**         转换后的FAT格式的文件名，字符串数组，包含pathDepth_ret个字符串，
 *                        每个字符串长度都为11，依次为转换后每层的文件名
 */
char **path_split(const char *pathInputConst, int *pathDepth_ret)
{
  int i, j;
  int pathDepth = 0;

  char *pathInput = strdup(pathInputConst);

  for (i = 0; pathInput[i] != '\0'; i++)
  {
    if (pathInput[i] == '/')
    {
      pathDepth++;
    }
  }

  char **paths = malloc(pathDepth * sizeof(char *));

  const char token[] = "/";
  char *slice;

  /* Dividing the path into separated strings of file names */
  slice = strtok(pathInput, token);
  for (i = 0; i < pathDepth; i++)
  {
    paths[i] = slice;
    slice = strtok(NULL, token);
  }

  char **pathFormatted = malloc(pathDepth * sizeof(char *));

  for (i = 0; i < pathDepth; i++)
  {
    pathFormatted[i] = malloc(MAX_SHORT_NAME_LEN * sizeof(char));
  }

  int k;

  /* Verifies if each file of the path is a valid input, and then formats it */
  for (i = 0; i < pathDepth; i++)
  {
    for (j = 0, k = 0; k < 11; j++)
    {
      if (paths[i][j] == '.')
      {
        /* Verifies if it is a "." or ".." */
        if (j == 0 && (paths[i][j + 1] == '\0' || (paths[i][j + 1] == '.' && paths[i][j + 2] == '\0')))
        {
          pathFormatted[i][0] = '.';

          if (paths[i][j + 1] == '\0')
            pathFormatted[i][1] = ' ';
          else
            pathFormatted[i][1] = '.';

          for (k = 2; k < 11; k++)
          {
            pathFormatted[i][k] = ' ';
          }
          break;
        }
        else
        {
          for (; k < 8; k++)
          {
            pathFormatted[i][k] = ' ';
          }
        }
      }
      else if (paths[i][j] == '\0')
      { /* End of the file name, fills with ' ' character the rest of the file
         * name and the file extension fields */
        for (; k < 11; k++)
        {
          pathFormatted[i][k] = ' ';
        }
        break;
      }
      else if (paths[i][j] >= 'a' && paths[i][j] <= 'z')
      { /* Turns lower case characters into upper case characters */
        pathFormatted[i][k++] = paths[i][j] - 'a' + 'A';
      }
      else
      {
        pathFormatted[i][k++] = paths[i][j];
      }
    }
    pathFormatted[i][11] = '\0';
  }

  *pathDepth_ret = pathDepth;
  free(paths);
  free(pathInput);
  return pathFormatted;
}

/**
 * @brief 将FAT格式的文件名转换为文件名字符串，如"FILE    TXT"会被转换为"file.txt"
 * 
 * @param path    FAT格式的文件名，长度为11的字符串
 * @return BYTE*  普通格式的文件名字符串
 */
BYTE *path_decode(BYTE *path)
{

  int i, j;
  BYTE *pathDecoded = malloc(MAX_SHORT_NAME_LEN * sizeof(BYTE));

  /* If the name consists of "." or "..", return them as the decoded path */
  if (path[0] == '.' && path[1] == '.' && path[2] == ' ')
  {
    pathDecoded[0] = '.';
    pathDecoded[1] = '.';
    pathDecoded[2] = '\0';
    return pathDecoded;
  }
  if (path[0] == '.' && path[1] == ' ')
  {
    pathDecoded[0] = '.';
    pathDecoded[1] = '\0';
    return pathDecoded;
  }

  /* Decoding from uppercase letters to lowercase letters, removing spaces,
   * inserting 'dots' in between them and verifying if they are legal */
  for (i = 0, j = 0; i < 11; i++)
  {
    if (path[i] != ' ')
    {
      if (i == 8)
        pathDecoded[j++] = '.';

      if (path[i] >= 'A' && path[i] <= 'Z')
        pathDecoded[j++] = path[i] - 'A' + 'a';
      else
        pathDecoded[j++] = path[i];
    }
  }
  pathDecoded[j] = '\0';
  return pathDecoded;
}

/**
 * @brief 获得父目录的路径。如输入path = "dir1/dir2/texts"，得到"dir1/dir2"
 * 
 * @param path        路径
 * @param orgPaths    由`org_path_split`分割后的路径，如{ "dir1", "dir2", "tests" }
 * @param pathDepth   由`org_path_split`返回的路径层数，如3
 * @return char*      父目录的路径
 */
char *get_prt_path(const char *path, const char **orgPaths, int pathDepth)
{
  char *prtPath;
  if (pathDepth == 1)
  {
    prtPath = (char *)malloc(2 * sizeof(char));
    prtPath[0] = '/';
    prtPath[1] = '\0';
  }
  else
  {
    int prtPathLen = strlen(path) - strlen(orgPaths[pathDepth - 1]) - 1;
    prtPath = (char *)malloc((prtPathLen + 1) * sizeof(char));
    strncpy(prtPath, path, prtPathLen);
    prtPath[prtPathLen] = '\0';
  }
  return prtPath;
}

/**
 * @brief 从镜像文件中读取BPB，并计算所需的各类元数据
 * 
 * @param imageFilePath 镜像文件路径
 * @return FAT16*       存储元数据的FAT16结构指针
 */
FAT16 *pre_init_fat16(const char *imageFilePath)
{
  /* Opening the FAT16 image file */
  FILE *fd = fopen(imageFilePath, "rb+");

  if (fd == NULL)
  {
    fprintf(stderr, "Missing FAT16 image file!\n");
    exit(EXIT_FAILURE);
  }

  FAT16 *fat16_ins = malloc(sizeof(FAT16));

  fat16_ins->fd = fd;

  /* Reads the BPB */
  sector_read(fat16_ins->fd, 0, &fat16_ins->Bpb);

  /* First sector of the root directory */
  fat16_ins->FirstRootDirSecNum = fat16_ins->Bpb.BPB_RsvdSecCnt + (fat16_ins->Bpb.BPB_FATSz16 * fat16_ins->Bpb.BPB_NumFATS);

  /* Number of sectors in the root directory */
  DWORD RootDirSectors = ((fat16_ins->Bpb.BPB_RootEntCnt * 32) +
                          (fat16_ins->Bpb.BPB_BytsPerSec - 1)) /
                         fat16_ins->Bpb.BPB_BytsPerSec;

  /* First sector of the data region (cluster #2) */
  fat16_ins->FirstDataSector = fat16_ins->Bpb.BPB_RsvdSecCnt + (fat16_ins->Bpb.BPB_NumFATS * fat16_ins->Bpb.BPB_FATSz16) + RootDirSectors;

  fat16_ins->FatOffset =  fat16_ins->Bpb.BPB_RsvdSecCnt * fat16_ins->Bpb.BPB_BytsPerSec;
  fat16_ins->FatSize = fat16_ins->Bpb.BPB_BytsPerSec * fat16_ins->Bpb.BPB_FATSz16;
  fat16_ins->RootOffset = fat16_ins->FatOffset + fat16_ins->FatSize * fat16_ins->Bpb.BPB_NumFATS;
  fat16_ins->ClusterSize = fat16_ins->Bpb.BPB_BytsPerSec * fat16_ins->Bpb.BPB_SecPerClus;
  fat16_ins->DataOffset = fat16_ins->RootOffset + fat16_ins->Bpb.BPB_RootEntCnt * BYTES_PER_DIR;

  return fat16_ins;
}

/**
 * @brief 返回簇号为ClusterN对应的FAT表项
 * 
 * @param fat16_ins 文件系统元数据指针
 * @param ClusterN  簇号（注意合法的簇号从2开始）
 * @return WORD     FAT表项（每个表项2字节，故用WORD存储）
 */
WORD fat_entry_by_cluster(FAT16 *fat16_ins, WORD ClusterN)
{
  /* Buffer to store bytes from the image file and the FAT16 offset */
  BYTE sector_buffer[BYTES_PER_SECTOR];
  WORD FATOffset = ClusterN * 2;
  /* FatSecNum is the sector number of the FAT sector that contains the entry
   * for cluster N in the first FAT */
  WORD FatSecNum = fat16_ins->Bpb.BPB_RsvdSecCnt + (FATOffset / fat16_ins->Bpb.BPB_BytsPerSec);
  WORD FatEntOffset = FATOffset % fat16_ins->Bpb.BPB_BytsPerSec;

  /* Reads the sector and extract the FAT entry contained on it */
  sector_read(fat16_ins->fd, FatSecNum, &sector_buffer);
  return *((WORD *)&sector_buffer[FatEntOffset]);
  // return 0xffff;  //可以修改
}

/**
 * Given a cluster N, this function reads its fisrst sector,
 * then set the value of its FAT entry and the value of its first sector of cluster.
 * ============================================================================
 **/

/**
 * @brief 读取指定簇号的簇的第一个扇区，并给出相应的FAT表项和扇区号
 * 
 * @param fat16_ins               文件系统元数据指针
 * @param ClusterN                簇号
 * @param FatClusEntryVal         输出参数，对应簇的FAT表项
 * @param FirstSectorofCluster    输出参数，对应簇的第一个扇区的扇区号
 * @param buffer                  输出参数，对应簇第一个扇区的内容
 */
void first_sector_by_cluster(FAT16 *fat16_ins, WORD ClusterN, WORD *FatClusEntryVal, WORD *FirstSectorofCluster, BYTE *buffer)
{
  *FatClusEntryVal = fat_entry_by_cluster(fat16_ins, ClusterN);
  *FirstSectorofCluster = ((ClusterN - 2) * fat16_ins->Bpb.BPB_SecPerClus) + fat16_ins->FirstDataSector;

  sector_read(fat16_ins->fd, *FirstSectorofCluster, buffer);
}

/**
 * Browse directory entries in root directory.
 * @param offset_dir: the offset of corresponding dir_entry, used for write file, i.e. update file size
 * ==================================================================================
 * Return
 * 0, if we did find a file corresponding to the given path or 1 if we did not
 **/

/**
 * @brief 从根目录开始寻找path所对应的文件或目录，并设置相应目录项，以及目录项所在的偏移量
 * 
 * @param fat16_ins   文件系统元数据指针
 * @param Root        输出参数，对应的目录项
 * @param path        要查找的路径
 * @param offset_dir  输出参数，对应目录项在镜像文件中的偏移量（字节）
 * @return int        是否找到路径对应的文件或目录（0:找到， 1:未找到）
 */
int find_root(FAT16 *fat16_ins, DIR_ENTRY *Root, const char *path, off_t *offset_dir)
{
  int RootDirCnt = 1;
  BYTE buffer[BYTES_PER_SECTOR];

  int pathDepth;
  char *path_dup = strdup(path);
  char **paths = path_split((char *)path_dup, &pathDepth);

  sector_read(fat16_ins->fd, fat16_ins->FirstRootDirSecNum, buffer);

  /* We search for the path in the root directory first */
  for (uint i = 1; i <= fat16_ins->Bpb.BPB_RootEntCnt; i++)
  {
    memcpy(Root, &buffer[((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR], BYTES_PER_DIR);

    /* If the directory entry is free, all the next directory entries are also
     * free. So this file/directory could not be found */
    if (Root->DIR_Name[0] == 0x00)
    {
      return 1;
    }

    /* Check whether dir entry has been delete */
    int is_del = (Root->DIR_Name[0] == 0xe5);
    /* Comparing strings character by character */
    int is_eq = strncmp((const char *)Root->DIR_Name, paths[0], 11) == 0 ? 1 : 0;
    int is_valid = (!is_del) && is_eq;

    /* If the path is only one file (ATTR_ARCHIVE) and it is located in the
     * root directory, stop searching */
    if (is_valid && Root->DIR_Attr == ATTR_ARCHIVE)
    {
      *offset_dir = fat16_ins->RootOffset + (i - 1) * BYTES_PER_DIR;
      return 0;
    }

    /* If the path is only one directory (ATTR_DIRECTORY) and it is located in
     * the root directory, stop searching */
    if (is_valid && Root->DIR_Attr == ATTR_DIRECTORY && pathDepth == 1)
    {
      *offset_dir = fat16_ins->RootOffset + (i - 1) * BYTES_PER_DIR;
      return 0;
    }

    /* If the first level of the path is a directory, continue searching
     * in the root's sub-directories */
    if (is_valid && Root->DIR_Attr == ATTR_DIRECTORY)
    {
      return find_subdir(fat16_ins, Root, paths, pathDepth, 1, offset_dir);
    }

    /* End of bytes for this sector (1 sector == 512 bytes == 16 DIR entries)
     * Read next sector */
    if (i % 16 == 0 && i != fat16_ins->Bpb.BPB_RootEntCnt)
    {
      sector_read(fat16_ins->fd, fat16_ins->FirstRootDirSecNum + RootDirCnt, buffer);
      RootDirCnt++;
    }
  }

  /* We did not find anything */
  return 1;
}

/** TODO:
 * 从子目录开始查找path对应的文件或目录，找到返回0，没找到返回1，并将Dir填充为查找到的对应目录项
 *
 * Hint1: 在find_subdir入口处，Dir应该是要查找的这一级目录的表项，需要根据其中的簇号，读取这级目录对应的扇区数据
 * Hint2: 目录的大小是未知的，可能跨越多个扇区或跨越多个簇；当查找到某表项以0x00开头就可以停止查找
 * Hint3: 需要查找名字为paths[curDepth]的文件或目录，同样需要根据pathDepth判断是否继续调用find_subdir函数
 **/

/**
 * @brief 从子目录Dir开始递归寻找paths对应的文件
 * 
 * @param fat16_ins   文件系统元数据指针
 * @param Dir         输入当前要查找的子目录目录项，输出查找到的文件目录项
 * @param paths       要查找的路径，应该传入`path_split`返回的结果
 * @param pathDepth   路径的总层数，`path_split`输出的结果
 * @param curDepth    当前在查找的路径层数
 * @param offset_dir  输出参数，输出查找到的文件目录项在镜像文件中的偏移（字节）
 * @return int        是否找到路径对应的文件或目录（0:找到， 1:未找到）
 */
int find_subdir(FAT16 *fat16_ins, DIR_ENTRY *Dir, char **paths, int pathDepth, int curDepth, off_t *offset_dir)
{
  int DirSecCnt = 1, is_eq;
  BYTE buffer[BYTES_PER_SECTOR];

  WORD ClusterN, FatClusEntryVal, FirstSectorofCluster;

  ClusterN = Dir->DIR_FstClusLO;

  first_sector_by_cluster(fat16_ins, ClusterN, &FatClusEntryVal, &FirstSectorofCluster, buffer);

  /* Searching for the given path in all directory entries of Dir */
  for (uint i = 1; Dir->DIR_Name[0] != 0x00; i++)
  {
    memcpy(Dir, &buffer[((i - 1) * BYTES_PER_DIR) % BYTES_PER_SECTOR], BYTES_PER_DIR);

    /* Check whether dir entry has been delete */
    int is_del = (Dir->DIR_Name[0] == 0xe5);
    /* Comparing strings */
    is_eq = strncmp((const char *)Dir->DIR_Name, paths[curDepth], 11) == 0 ? 1 : 0;
    int is_valid = (!is_del) && is_eq;

    /* Stop searching if the last file of the path is located in this
     * directory */
    if ((is_valid && Dir->DIR_Attr == ATTR_ARCHIVE && curDepth + 1 == pathDepth) ||
        (is_valid && Dir->DIR_Attr == ATTR_DIRECTORY && curDepth + 1 == pathDepth))
    {
      *offset_dir = get_cluster_offset(fat16_ins, ClusterN) + (DirSecCnt - 1) * BYTES_PER_SECTOR + (i - 1) * BYTES_PER_DIR;
      return 0;
    }

    /* Recursively keep searching if the directory has been found and it isn't
     * the last file */
    if (is_valid && Dir->DIR_Attr == ATTR_DIRECTORY)
    {
      return find_subdir(fat16_ins, Dir, paths, pathDepth, curDepth + 1, offset_dir);
    }

    /* A sector needs to be readed 16 times by the buffer to reach the end. */
    if (i % 16 == 0)
    {
      /* If there are still sector to be read in the cluster, read the next sector. */
      if (DirSecCnt < fat16_ins->Bpb.BPB_SecPerClus)
      {
        sector_read(fat16_ins->fd, FirstSectorofCluster + DirSecCnt, buffer);
        DirSecCnt++;
      }
      else
      { /* Reaches the end of the cluster */

        /* Not strictly necessary, but here we reach the end of the clusters of
         * this directory entry. */
        if (FatClusEntryVal == 0xffff)
        {
          return 1;
        }

        /* Next cluster */
        ClusterN = FatClusEntryVal;

        first_sector_by_cluster(fat16_ins, ClusterN, &FatClusEntryVal, &FirstSectorofCluster, buffer);

        i = 0;
        DirSecCnt = 1;
      }
    }
  }

  /* We did not find the given path */
  return 1;
}

/**
 * @brief 分割字符串但不转换格式，如"/dir1/dir2/text"会转化为{"dir1","dir2","text"}。注意pathInput会被修改。
 * 
 * @param pathInput 输入的字符串
 * @return char**   分割后的字符串
 */
char **org_path_split(char *pathInput)
{
  int pathDepth = 0;
  for (uint i = 0; pathInput[i] != '\0'; i++)
  {
    if (pathInput[i] == '/')
    {
      pathDepth++;
    }
  }
  char **orgPaths = (char **)malloc(pathDepth * sizeof(char *));
  const char token[] = "/";
  char *slice;

  /* Dividing the path into separated strings of file names */
  slice = strtok(pathInput, token);
  for (uint i = 0; i < pathDepth; i++)
  {
    orgPaths[i] = slice;
    slice = strtok(NULL, token);
  }
  return orgPaths;
}

// ===========================文件系统接口实现===============================

/**
 * @brief 文件系统初始化，无需修改
 * 
 * @param conn 
 * @return void* 
 */
void *fat16_init(struct fuse_conn_info *conn)
{
  struct fuse_context *context;
  context = fuse_get_context();

  return context->private_data;
}

/**
 * @brief 释放文件系统，无需修改
 * 
 * @param data 
 */
void fat16_destroy(void *data)
{
  free(data);
}

/**
 * @brief 获取path对应的文件的属性，无需修改
 * 
 * @param path    要获取属性的文件路径
 * @param stbuf   输出参数，需要填充的属性结构体
 * @return int    成功返回0，失败返回POSIX错误代码的负值
 */
int fat16_getattr(const char *path, struct stat *stbuf)
{
  FAT16 *fat16_ins = get_fat16_ins();

  /* stbuf: setting file/directory attributes */
  memset(stbuf, 0, sizeof(struct stat));
  stbuf->st_dev = fat16_ins->Bpb.BS_VollID;
  stbuf->st_blksize = BYTES_PER_SECTOR * fat16_ins->Bpb.BPB_SecPerClus;
  stbuf->st_uid = getuid();
  stbuf->st_gid = getgid();

  if (strcmp(path, "/") == 0)
  {
    /* Root directory attributes */
    stbuf->st_mode = S_IFDIR | S_IRWXU;
    stbuf->st_size = 0;
    stbuf->st_blocks = 0;
    stbuf->st_ctime = stbuf->st_atime = stbuf->st_mtime = 0;
  }
  else
  {
    /* File/Directory attributes */
    DIR_ENTRY Dir;
    off_t offset_dir;
    int res = find_root(fat16_ins, &Dir, path, &offset_dir);

    if (res == 0)
    {
      /* FAT-like permissions */
      if (Dir.DIR_Attr == ATTR_DIRECTORY)
      {
        stbuf->st_mode = S_IFDIR | 0755;
      }
      else
      {
        stbuf->st_mode = S_IFREG | 0755;
      }
      stbuf->st_size = Dir.DIR_FileSize;

      /* Number of blocks */
      if (stbuf->st_size % stbuf->st_blksize != 0)
      {
        stbuf->st_blocks = (int)(stbuf->st_size / stbuf->st_blksize) + 1;
      }
      else
      {
        stbuf->st_blocks = (int)(stbuf->st_size / stbuf->st_blksize);
      }

      /* Implementing the required FAT Date/Time attributes */
      struct tm t;
      memset((char *)&t, 0, sizeof(struct tm));
      t.tm_sec = Dir.DIR_WrtTime & ((1 << 5) - 1);
      t.tm_min = (Dir.DIR_WrtTime >> 5) & ((1 << 6) - 1);
      t.tm_hour = Dir.DIR_WrtTime >> 11;
      t.tm_mday = (Dir.DIR_WrtDate & ((1 << 5) - 1));
      t.tm_mon = (Dir.DIR_WrtDate >> 5) & ((1 << 4) - 1);
      t.tm_year = 80 + (Dir.DIR_WrtDate >> 9);
      stbuf->st_ctime = stbuf->st_atime = stbuf->st_mtime = mktime(&t);
    }
    else
      return -ENOENT; // no such file
  }
  return 0;
}

// ------------------TASK1: 读目录、读文件-----------------------------------

/**
 * @brief 读取path对应的目录，结果通过filler函数写入buffer中
 * 
 * @param path    要读取目录的路径
 * @param buffer  结果缓冲区
 * @param filler  用于填充结果的函数，本次实验按filler(buffer, 文件名, NULL, 0)的方式调用即可。
 *                你也可以参考<fuse.h>第58行附近的函数声明和注释来获得更多信息。
 * @param offset  忽略
 * @param fi      忽略
 * @return int    成功返回0，失败返回POSIX错误代码的负值
 */
int fat16_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
                  off_t offset, struct fuse_file_info *fi)
{
  FAT16 *fat16_ins = get_fat16_ins();

  BYTE sector_buffer[BYTES_PER_SECTOR];
  int RootDirCnt = 1, DirSecCnt = 1;
 
  // 如果要读取的目录是根目录
  if (strcmp(path, "/") == 0)
  {
    DIR_ENTRY Root;
    /** TODO:
     * 将root directory下的文件或目录通过filler填充到buffer中
     * 注意不需要遍历子目录
     **/
    // Hint: 读取根文件目录区域所在的第一个扇区
    sector_read(fat16_ins->fd, fat16_ins->FirstRootDirSecNum, sector_buffer);
    // Hint: 依次读取根目录中每个目录项，目录项的最大条数是RootEntCnt
    for (uint i = 1; i <= fat16_ins->Bpb.BPB_RootEntCnt; i++)
    {
      /** TODO: 这段代码中，从扇区数据（存放在sector_buffer）中正确位置位置读取到相应的目录项，
       *        并从目录项中解析出正确的文件名/目录名，然后通过filler函数填充到buffer中。
       *        filler函数的用法：filler(buffer, 文件名, NULL, 0)
       *        解析出文件名可使用path_decode函数，使用方法请参考函数注释
       **/
      /*** BEGIN ***/


      /*** END ***/

      // 当前扇区所有条目已经读取完毕，将下一个扇区读入sector_buffer
      if (i % 16 == 0 && i != fat16_ins->Bpb.BPB_RootEntCnt)
      {
        sector_read(fat16_ins->fd, fat16_ins->FirstRootDirSecNum + RootDirCnt, sector_buffer);
        RootDirCnt++;
      }

    }
  }
  else  // 要查询的目录不是根目录
  {
    DIR_ENTRY Dir;
    off_t offset_dir;
    /** TODO:
     * 本段代码的逻辑类似上一段代码，但是需要先找到path所对应的目录的目录区域位置。
     * 我们提供了find_root函数，来获取这个位置，
     * 获得了目录项位置后，与上一段代码一样，你需要遍历其下目录项，将文件或目录名通过filler填充到buffer中，
     * 同样注意不需要遍历子目录
     * Hint: 需要考虑目录大小，子目录的目录区域不一定连续，可能跨扇区，跨簇
     **/

    /* Finds the first corresponding directory entry in the root directory and
     * store the result in the directory entry Dir */
    find_root(fat16_ins, &Dir, path, &offset_dir);

    /* Calculating the first cluster sector for the given path */
    WORD ClusterN;              // 当前读取的簇号
    WORD FatClusEntryVal;       // 该簇的FAT表项（大部分情况下，代表下一个簇的簇号，请参考实验文档对FAT表项的说明）
    WORD FirstSectorofCluster;  // 该簇的第一个扇区号

    ClusterN = Dir.DIR_FstClusLO; //目录项中存储了我们要读取的第一个簇的簇号
    first_sector_by_cluster(fat16_ins, ClusterN, &FatClusEntryVal, &FirstSectorofCluster, sector_buffer);

    /* Start searching the root's sub-directories starting from Dir */
    for (uint i = 1; Dir.DIR_Name[0] != 0x00; i++)
    {
      // TODO: 读取对应目录项，并用filler填充到buffer
      /*** BEGIN ***/


      /*** END ***/

      // 当前扇区的所有目录项已经读完。
      if (i % 16 == 0)
      {
        // 如果当前簇还有未读的扇区
        if (DirSecCnt < fat16_ins->Bpb.BPB_SecPerClus)
        {
          // TODO: 将下一个扇区的数据读入sector_buffer
          /*** BEGIN ***/


          /*** END ***/
        }
        else // 当前簇已经读完，需要读取下一个簇的内容
        {
          // Hint: 下一个簇的簇号已经存储在FatClusEntryVal当中
          // Hint: 为什么？你可以参考first_sector_by_cluster的函数注释，以及文档中对FAT表项的说明

          // 已经到达最后一个簇（0xFFFF代表什么？请参考文档中对FAT表项的说明）
          if (FatClusEntryVal == 0xffff)
          {
            return 0;
          }

          // TODO: 读取下一个簇（即簇号为FatClusEntryVal的簇）的第一个扇区
          // Hint: 你需要通过first_sector_by_cluster函数读取下一个簇，注意将FatClusEntyVal需要存储下一个簇的簇号
          // Hint: 你可模仿函数开头对first_sector_by_cluster的调用方法，但注意传入正确的簇号
          // Hint: 最后，你需要将i和DirSecCnt设置到正确的值
          /*** BEGIN ***/


          /*** END ***/
        }
      }
    }
  }
  return 0;
}

/**
 * @brief 从path对应的文件的offset字节处开始读取size字节的数据到buffer中，并返回实际读取的字节数。
 * Hint: 文件大小属性是Dir.DIR_FileSize。
 * 
 * @param path    要读取文件的路径
 * @param buffer  结果缓冲区
 * @param size    需要读取的数据长度
 * @param offset  要读取的数据所在偏移量
 * @param fi      忽略
 * @return int    成功返回实际读写的字符数，失败返回0。
 */
int fat16_read(const char *path, char *buffer, size_t size, off_t offset,
               struct fuse_file_info *fi)
{
  /* Gets volume data supplied in the context during the fat16_init function */
  FAT16 *fat16_ins = get_fat16_ins();

  /** TODO: 在这个函数中，你需要将path代表的文件从offset位置开始size个字节读取到buffer中。
   *  为此，你需要完成以下几个步骤:
   *  1. 由于文件是按簇组织的，簇又是由扇区组成的，要读取的偏移量可能在簇和扇区的中间，
   *     所以本函数的关键就是读取范围的开始和结束的簇、扇区、扇区中的偏移，为此，你需要计算出：
   *     1. 要读取数据的在文件中的范围（字节）：这很简单，beginBytes =  offset, endBytes = offset + size - 1
   *     2. beginBytes在文件中哪一个簇，endBytes在文件中哪一个簇？
   *     3. beginBytes在簇中第几个扇区，endBytes在簇中第几个扇区？
   *     4. beginBytes在扇区中哪个位置，endBytes在扇区中哪个位置？
   *  2. 计算出上述值后，你需要通过find_root找到文件对应的目录项，并找到文件首个簇号。
   *     这个步骤和readdir中比较相似，可以参考。
   *  3. 找到首个簇号后，通过FAT表依次遍历找到下一个簇的簇号，直到beginBytes所在簇。
   *     遍历簇的方法是通过fat_entry_by_cluster读取FAT表项（也就是下一个簇号）。注意模仿readdir处理文件结束。
   *     （哪一个簇号代表文件结束？）
   *  4. 读取beginBytes对应的扇区，然后循环读取簇和扇区，同时填充buffer，直到endBytes所在的扇区。
   *  5. 注意第一个和最后一个扇区中，根据偏移量，我们不一定需要扇区中所有数据，请根据你在1.4中算出的值将正确的部分填入buffer。
   *     而中间所有扇区，一定需要被全部读取。
   * 
   *  HINT: 如果你觉得对上述位置的计算很困难，你也可以考虑将将所有簇全部依次读入内存，再截取正确的部分填入buffer。
   *        我们不推荐那么做，但不做强制要求，如果你用这种方法实现了正确的功能，同样能获得该部分全部分数。
   **/

  /*** BEGIN ***/


  /*** END ***/
  return 0;
}

// ------------------TASK2: 创建/删除文件-----------------------------------
/**
 * @brief 在path对应的路径创建新文件
 * 
 * @param path    要创建的文件路径
 * @param mode    要创建文件的类型，本次实验可忽略，默认所有创建的文件都为普通文件
 * @param devNum  忽略，要创建文件的设备的设备号
 * @return int    成功返回0，失败返回POSIX错误代码的负值
 */
int fat16_mknod(const char *path, mode_t mode, dev_t devNum)
{
  /* Gets volume data supplied in the context during the fat16_init function */
  FAT16 *fat16_ins = get_fat16_ins();

  // 查找需要创建文件的父目录路径
  int pathDepth;
  char **paths = path_split((char *)path, &pathDepth);
  char *copyPath = strdup(path);
  const char **orgPaths = (const char **)org_path_split(copyPath);
  char *prtPath = get_prt_path(path, orgPaths, pathDepth);

  /** TODO:
   * 查找可用的entry，注意区分根目录和子目录
   * 下面提供了一些可能使用到的临时变量
   * 如果觉得不够用，可以自己定义更多的临时变量
   * 这块和前面有很多相似的地方，注意对照来实现
   **/
  BYTE sector_buffer[BYTES_PER_SECTOR];
  DWORD sectorNum;
  int offset, i, findFlag = 0, RootDirCnt = 1, DirSecCnt = 1;
  WORD ClusterN, FatClusEntryVal, FirstSectorofCluster;

  /* If parent directory is root */
  if (strcmp(prtPath, "/") == 0)
  {
    /**
     * 遍历根目录下的目录项
     * 如果发现有同名文件，则直接中断，findFlag=0
     * 找到可用的空闲目录项，即0x00位移处为0x00或者0xe5的项, findFlag=1
     * 并记录对应的sectorNum, offset等可能会用得到的信息
     **/    
    /*** BEGIN ***/


    /*** END ***/
  }
  /* Else if parent directory is sub-directory */
  else
  {
    /**
     * 遍历根目录下的目录项
     * 如果发现有同名文件，则直接中断，findFlag=0
     * 找到可用的空闲目录项，即0x00位移处为0x00或者0xe5的项, findFlag=1
     * 并记录对应的sectorNum, offset等可能会用得到的信息
     * 注意跨扇区和跨簇的问题，不过写到这里你应该已经很熟了
     **/    
    /*** BEGIN ***/


    /*** END ***/
  }

  //没有同名文件，且找到空闲表项时，调用函数创建目录项
  /* Add the DIR ENTRY */
  if (findFlag == 1) {
    // TODO: 正确调用dir_entry_create创建目录项
    /*** BEGIN ***/


    /*** END ***/
  }
  return 0;
}

/* (This is a new function) Add an entry according to specified sector and offset
 * ==================================================================================
 * exp: sectorNum = 1000, offset = 1*BYTES_PER_DIR
 * The 1000th sector content before create: | Dir Entry(used)   |  Dir Entry(free)  | ... |
 * The 1000th sector content after create:  | Dir Entry(used)   | Added Entry(used) | ... |
 * ==================================================================================
 */
/**
 * @brief 创建目录项
 * 
 * @param fat16_ins       文件系统指针
 * @param sectorNum       目录项所在扇区号
 * @param offset          目录项在扇区中的偏移量
 * @param Name            文件名（FAT格式）
 * @param attr            文件属性
 * @param firstClusterNum 文件首簇簇号
 * @param fileSize        文件大小
 * @return int            成功返回0
 */
int dir_entry_create(FAT16 *fat16_ins, int sectorNum, int offset, char *Name, BYTE attr, WORD firstClusterNum, DWORD fileSize)
{
  /* Create memory buffer to store entry info */
  //先在buffer中写好表项的信息，最后通过一次IO写入到磁盘中
  BYTE *entry_info = malloc(BYTES_PER_DIR * sizeof(BYTE));

  /**
     * TODO:为新表项填入文件名和文件属性
     **/
  /*** BEGIN ***/


  /*** END ***/

  /**
     * 关于时间信息部分的工作我们本次实验不要求
     * 代码已经给出，可以参考实验文档自行理解
     **/
  time_t timer_s;
  time(&timer_s);
  struct tm *time_ptr = localtime(&timer_s);
  int value;

  /* Unused */
  memset(entry_info + 12, 0, 10 * sizeof(BYTE));

  /* File update time */
  /* 时间部分可以阅读实验文档 */
  value = time_ptr->tm_sec / 2 + (time_ptr->tm_min << 5) + (time_ptr->tm_hour << 11);
  memcpy(entry_info + 22, &value, 2 * sizeof(BYTE));

  /* File update date */
  value = time_ptr->tm_mday + (time_ptr->tm_mon << 5) + ((time_ptr->tm_year - 80) << 9);
  memcpy(entry_info + 24, &value, 2 * sizeof(BYTE));
  
  /**
     * TODO:为新表项填入文件首簇号与文件大小
     **/
  /* First Cluster Number & File Size */
  /*** BEGIN ***/


  /*** END ***/

  /**
     * TODO:将创建好的新表项信息写入到磁盘
     **/
  /* Write the above entry to specified location */
  /*** BEGIN ***/


  /*** END ***/
  free(entry_info);
  return 0;
}

/**
 * @brief 释放簇号对应的簇，只需修改FAT对应表项，并返回下一个簇的簇号。
 * 
 * @param fat16_ins   文件系统指针
 * @param ClusterNum  要释放的簇号
 * @return int        下一个簇的簇号
 */
int free_cluster(FAT16 *fat16_ins, int ClusterNum)
{
  BYTE sector_buffer[BYTES_PER_SECTOR];
  WORD FATClusEntryval, FirstSectorofCluster;
  first_sector_by_cluster(fat16_ins, ClusterNum, &FATClusEntryval, &FirstSectorofCluster, sector_buffer);

  FILE *fd = fat16_ins->fd;
  /** TODO:
   * 修改FAT表
   * 注意两个表都要修改
   * FAT表1和表2的偏移地址怎么计算参考实验文档
   * 每个表项为2个字节
   **/
  /*** BEGIN ***/


  /*** END ***/

  return FATClusEntryval;
}

/**
 * @brief 删除path对应的文件
 * 
 * @param path  要删除的文件路径
 * @return int  成功返回0，失败返回POSIX错误代码的负值
 */
int fat16_unlink(const char *path)
{
  /* Gets volume data supplied in the context during the fat16_init function */
  FAT16 *fat16_ins = get_fat16_ins();

  DIR_ENTRY Dir;
  off_t offset_dir;
  //释放使用过的簇
  if (find_root(fat16_ins, &Dir, path, &offset_dir) == 1)
  {
    return 1;
  }
  
  /** TODO: 回收该文件所占有的簇（注意你可能需要先完善free_cluster函数）
   *  你需要先获得第一个簇的簇号（你可以在Dir结构体中找到它），调用使用free_cluster释放它。
   *  并获得下一需要释放的簇的簇号，直至文件末尾。
   * 在完善了free_cluster函数后，此处代码量很小
   * 你也可以不使用free_cluster函数，通过自己的方式实现 */
  /*** BEGIN ***/


  /*** END ***/

  // 查找需要删除文件的父目录路径
  int pathDepth;
  char **paths = path_split((char *)path, &pathDepth);
  char *copyPath = strdup(path);
  const char **orgPaths = (const char **)org_path_split(copyPath);
  char *prtPath = get_prt_path(path, orgPaths, pathDepth);

  /** TODO:
   * 定位文件在父目录中的entry，注意区分根目录和子目录
   * 下面提供了一些可能使用到的临时变量
   * 如果觉得不够用，可以自己定义更多的临时变量
   * 这块和前面有很多相似的地方，注意对照来实现
   * 流程类似，大量代码都和mknode一样，注意复用
   **/

  BYTE sector_buffer[BYTES_PER_SECTOR];
  DWORD sectorNum;
  int offset, i, findFlag = 0, RootDirCnt = 1, DirSecCnt = 1;
  WORD FatClusEntryVal, FirstSectorofCluster;

  /* If parent directory is root */
  if (strcmp(prtPath, "/") == 0)
  {
    /**
     * 遍历根目录下的目录项
     * 查找是否有相同文件名的文件
     * 存在则findFlag=1
     * 并记录对应的sectorNum, offset等可能会用得到的信息
     **/
    /*** BEGIN ***/


    /*** END ***/
  }
  else /* Else if parent directory is sub-directory */
  {
    /**
     * 遍历根目录下的目录项
     * 查找是否有相同文件名的文件
     * 存在则findFlag=1
     * 并记录对应的sectorNum, offset等可能会用得到的信息
     * 注意跨扇区和跨簇的问题，不过写到这里你应该已经很熟了
     **/
    /*** BEGIN ***/


    /*** END ***/
  }

  /** TODO:
   * 删除文件，对相应entry做标记
   * 思考要修改entry中的哪些域
   * 注意目录项被删除时的标记为曾被使用但目前已删除
   * 和一个完全没用过的目录项的标记是不一样的
   **/

  /* Update file entry, change its first byte of file name to 0xe5 */
  if (findFlag == 1)
  {
    /*** BEGIN ***/


    /*** END ***/
  }
  return 0;
}

/**
 * @brief 修改path对应文件的时间戳，本次实验不做要求，可忽略该函数
 * 
 * @param path  要修改时间戳的文件路径
 * @param tv    时间戳
 * @return int 
 */
int fat16_utimens(const char *path, const struct timespec tv[2])
{
  return 0;
}


struct fuse_operations fat16_oper = {
    .init = fat16_init,
    .destroy = fat16_destroy,
    .getattr = fat16_getattr,

    // TASK1: tree [dir] / ls [dir] ; cat [file] / tail [file] / head [file]
    .readdir = fat16_readdir,
    .read = fat16_read,

    // TASK2: touch [file]; rm [file]
    .mknod = fat16_mknod,
    .unlink = fat16_unlink,
    .utimens = fat16_utimens,

    // TASK3: mkdir [dir] ; rm -r [dir]
    .mkdir = fat16_mkdir,
    .rmdir = fat16_rmdir,

    // TASK4: echo "hello world!" > [file] ;  echo "hello world!" >> [file]
    .write = fat16_write,
    .truncate = fat16_truncate
    };

int main(int argc, char *argv[])
{
  int ret;

  /* Starting a pre-initialization of the FAT16 volume */
  FAT16 *fat16_ins = pre_init_fat16(FAT_FILE_NAME);

  ret = fuse_main(argc, argv, &fat16_oper, fat16_ins);

  return ret;
}