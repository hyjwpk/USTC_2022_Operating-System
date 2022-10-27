# -*- coding: UTF-8 -*-
import os
import matplotlib.pyplot as plt


def readHotFile(fileName):
    cycle = 1
    rs = os.path.exists(fileName)
    if rs == True:
        file_h =open(fileName,mode='r')
        for line in file_h.readlines():
            line = line.strip()
            if not len(line) or line.startswith('\n'):
                continue
            temp = [int(i,16) for i in line.split(',')]
            x = [cycle for i in range(0,len(temp))]
            plt.scatter(temp, x, s = 1, color = 'black', marker=',')
            cycle += 1
        plt.xlabel('Physical Page No',fontsize=15)  #设置x，y轴的标签
        plt.ylabel('Time (s)',fontsize=15)
        plt.savefig('page_fre_pic.jpg', dpi=400)
        plt.show()

readHotFile("active_page_log.txt")