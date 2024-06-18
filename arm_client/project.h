#ifndef __PROJECT_H__
#define __PROJECT_H__

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define CONFIGPATH "./Iotconf"
#define MAXLINK 10

/* 数据统一结构*/

#define GETENV 1                                                                                 
#define SETENV 2                                                                                 
#define DEVCTL 3                                                                                 
                                                                                                 
typedef struct{                                                                                  
    char id[20];                                                                                 
    char ps[20];                                                                                 
    unsigned char flags;                                                                         
} Login_t;                                                                                       
                                                                                                 
typedef struct{                                                                                  
   float temp;                                                                                   
   unsigned char hume;                                                                           
   unsigned short lux;                                                                           
   unsigned char devstuat; // 0 bit 照明设备   1-2 bit 温控设备   3 加湿器设备/  4-7 bit保留     
} Env_t;                                                                                         
                                                                                                 
typedef struct{                                                                                  
    float uptemp;                                                                                
    float downtemp;                                                                              
    unsigned char uphume;                                                                        
    unsigned char downhume;                                                                      
    unsigned short uplux;                                                                        
    unsigned short downlux;                                                                      
} Limit_t;                                                                                       
                                                                                                 
typedef struct{                                                                                  
    long long msgtype;                                                                                
    long long rcvtype;                                                                                
    Login_t login;                                                                               
    Env_t envdata;                                                                               
    Limit_t limitdata;                                                                           
    unsigned char devctl;                                                                        
    unsigned char commd;                                                                         
} msg_t;                                                                                         



int Net_init(const char*IP ,const char *PROT);

/*读取配置文件*/
int read_config(msg_t *buf);

/*获取环境数据线程*/
void *envgetthread(void *argv);

/*设置环境阈值线程*/
void *envsetthread(void *argv);

/*设备控制线程*/
void *devctlthread(void *argv);

/*根据用户阈值环境维护线程*/
void *holdenvthread(void *argv);



#endif
