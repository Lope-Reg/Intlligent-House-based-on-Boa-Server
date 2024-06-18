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
#include <semaphore.h>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>

#define MSGPATH "/home/linux/"
#define MAXLINK 100

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
    long msgtype;
    long rcvtype;
    Login_t login;
    Env_t envdata;
    Limit_t limitdata;
    unsigned char devctl;
    unsigned char commd;
} msg_t;

/*链表节点*/
typedef struct node{
	int fd;
	char id[20];
	pthread_t tid;
	struct node *next;
} link_t;

/*网络初始化*/
int Net_init(const char *PROT);

/*处理线程*/
void *handl_thread(void *argv);

/*心跳包线程*/
void *insprct_thread(void *argv);

/*检索下位机在线状态线程*/
void *getlinkstaut(void *argv);


/*创建链表*/
link_t *link_creat(void);

/*增加数据*/
int insert_data(link_t *L,int fd,const char *name,pthread_t tid);

/*根据ID查找节点*/
int idfind(link_t *L,const char *name);

/*根据fd删除*/
int delete_data(link_t *L,int fd);

/*遍历链表，用于心跳包检测使用*/
int show_list(link_t *L);

/*释放链表*/
int free_link(link_t **L);

#endif
