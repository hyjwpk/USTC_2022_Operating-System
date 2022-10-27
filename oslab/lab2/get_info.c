#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

typedef struct Task {
    int pid;
    char comm[16];
    int isrunning;
    double cpu;
} Task;

int cmp(const void *a, const void *b) {
    return ((Task *)a)->cpu < ((Task *)b)->cpu;
}

int main(void) {
    Task task[500];
    int result_1, result_2;
    int pid_1[500], pid_2[500];
    char comm_1[500][16], comm_2[500][16];
    int isrunning_1[500], isrunning_2[500];
    unsigned long long sum_exec_runtime_1[500], sum_exec_runtime_2[500];
    int counter = 0;
    if (syscall(332, &result_1, pid_1, comm_1, isrunning_1, sum_exec_runtime_1) < 0)
        return -1;
    while (1) {
        counter = 0;
        sleep(1);
        if (syscall(332, &result_2, pid_2, comm_2, isrunning_2, sum_exec_runtime_2) < 0)
            return -1;
        int i = 0, j = 0;
        while (i < result_1 && j < result_2) {
            if (pid_2[i] == pid_1[j]) {
                task[counter].pid = pid_2[i];
                strcpy(task[counter].comm, comm_2[i]);
                task[counter].isrunning = isrunning_2[i];
                task[counter].cpu = (sum_exec_runtime_2[i] - sum_exec_runtime_1[i]) / 10000000.0;
                counter++;
                i++;
                j++;
            } else if (pid_1[i] < pid_2[j])
                i++;
            else if (pid_1[i] > pid_2[j])
                j++;
        }
        qsort(task, counter, sizeof(Task), cmp);
        system("clear");
        printf("%-5s%-16s%-10s%-5s\n", "PID", "COMM", "ISRUNNING", "CPU");
        for (int i = 0; i < counter && i < 20; i++) {
            printf("%-5d%-16s%-10d%-2.3lf%%\n", task[i].pid, task[i].comm, task[i].isrunning, task[i].cpu);
        }

        result_1 = result_2;
        memcpy(pid_1, pid_2, sizeof(int) * 500);
        memcpy(comm_1, comm_2, sizeof(char) * 500 * 16);
        memcpy(isrunning_1, isrunning_2, sizeof(int) * 500);
        memcpy(sum_exec_runtime_1, sum_exec_runtime_2, sizeof(unsigned long long) * 500);
    }
}
