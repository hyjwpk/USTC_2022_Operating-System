cmd_/home/wangc/lab3/part2/modules.order := {   echo /home/wangc/lab3/part2/ktest.ko; :; } | awk '!x[$$0]++' - > /home/wangc/lab3/part2/modules.order
