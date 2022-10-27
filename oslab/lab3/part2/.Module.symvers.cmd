cmd_/home/wangc/lab3/part2/Module.symvers := sed 's/\.ko$$/\.o/' /home/wangc/lab3/part2/modules.order | scripts/mod/modpost -m -a  -o /home/wangc/lab3/part2/Module.symvers -e -i Module.symvers   -T -
