#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <random>
#include <iostream>

int main(){
    char name[20] = "fat16.img";

    // 获取文件大小
    struct stat statbuf;
	stat(name, &statbuf);
	size_t filesize = statbuf.st_size;

    FILE *img = fopen(name,"rb+");

    size_t offset;
    offset = 0x14000;
    //生成随机偏移
    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::uniform_int_distribution<size_t> dis(0, filesize);
    // offset = dis(gen);
    // std::cout << offset << std::endl;

    char byte;
    // 0x14000原始字符为 #
    fseek(img,offset,SEEK_SET);
    fread(&byte,1,1,img);

    // 0x14000翻转末尾，变为 "
    byte = byte & (char)~1;

    //写回
    fseek(img,offset,SEEK_SET);
    fwrite(&byte,1,1,img);
    fclose(img);

	return 0;
}