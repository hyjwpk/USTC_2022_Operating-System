#!/bin/bash
if [[ $# -lt 1 ]]; then
    echo "Shell Err! Please input function No. Right shell: $0 [func No.]"
    exit
else
    func_no=${1}
fi
echo "1. disable transparent huge"
sudo bash -c "echo never > /sys/kernel/mm/transparent_hugepage/enabled"
sudo bash -c "echo never > /sys/kernel/mm/transparent_hugepage/defrag"
workload_pid=$(ps -ef | grep lab3_workload | grep -v grep | awk '{print $2}')
if [[ ! -z "${workload_pid}" ]]; then
    sudo kill -9 ${workload_pid}
fi
module_loaded=$(lsmod | grep ktest | wc -l)
if [[ ${module_loaded} -ge 1 ]]; then
    sudo bash -c "echo 0 > /sys/kernel/mm/ktest/ktestrun"
    sudo rmmod ktest
fi

echo "2. update follow_page & page_referenced addr & output file path."
cur_dir=$(pwd)
follow_page_addr=$(sudo cat /proc/kallsyms | grep -w follow_page | awk '{print $1}')
page_referenced_addr=$(sudo cat /proc/kallsyms | grep -w page_referenced | awk '{print $1}')
follow_page_line=$(cat ktest.c | grep -n "follow_page)0x" | awk -F':' '{print $1}')
page_referenced_line=$(cat ktest.c | grep -n "page_referenced)0x" | awk -F':' '{print $1}')
output_file_line=$(cat ktest.c | grep -n "#define OUTPUT_FILE" | awk -F':' '{print $1}')
sed -i "${follow_page_line}c static my_follow_page mfollow_page = (my_follow_page)0x${follow_page_addr};" ktest.c
sed -i "${page_referenced_line}c static my_page_referenced mpage_referenced = (my_page_referenced)0x${page_referenced_addr};" ktest.c
sed -i "${output_file_line}c #define OUTPUT_FILE \"${cur_dir}/expr_result.txt\"" ktest.c
err=$(make | grep Error | wc -l)
if [[ ${err} -gt 1 ]]; then
    echo "Compile Error!"
    exit
fi
echo "3. load linux module."
sudo insmod ktest.ko
sudo bash -c "echo ${func_no} > /sys/kernel/mm/ktest/func"
cd ${cur_dir}/trace
echo "4. compile workload."
g++ -std=c++11 workload.cc -o lab3_workload -lpthread
chmod +x lab3_workload
echo "5. run workload."
./lab3_workload >/dev/null &
workload_pid=$(ps -ef | grep lab3_workload | grep -v grep | awk '{print $2}')
if [[ -z "${workload_pid}" ]]; then
    echo "Workload pid is empty!"
    exit
fi
echo >expr_result.txt
echo "6. run linux module, func=${func_no}"
sudo bash -c "echo ${workload_pid} > /sys/kernel/mm/ktest/pid"
sudo bash -c "echo 5000 > /sys/kernel/mm/ktest/sleep_millisecs"
sudo bash -c "echo 1 > /sys/kernel/mm/ktest/ktestrun"

if [[ ${func_no} != 2 ]]; then
    sleep 5
    sudo bash -c "echo 0 > /sys/kernel/mm/ktest/ktestrun"
    ps -ef | grep lab3_workload | grep -v grep | awk '{print $2}' | xargs sudo kill -9
    echo "7. rename expr_result.txt"
    cd ${cur_dir} && mv expr_result.txt "expr_result_${func_no}_$(date +%Y%m%d)_$(date +%H%M).txt"
    exit
fi

while :; do
    workload_num=$(ps -ef | grep lab3_workload | grep -v grep | wc -l)
    if [[ ${workload_num} -eq 0 ]]; then
        echo "7. workload end!"
        sudo bash -c "echo 0 > /sys/kernel/mm/ktest/ktestrun"
        break
    fi
    sleep 1
done

if [[ ${func_no} == 2 ]]; then
    echo "8. draw pic"
    cd ${cur_dir} && mv expr_result.txt active_page_log.txt
    python3 draw.py
fi
