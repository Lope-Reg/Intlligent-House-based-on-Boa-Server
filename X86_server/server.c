#include "project.h"

extern link_t *L;
extern key_t key;

extern int msgrid;
extern int msgwid;

extern msg_t msg;
extern sem_t linksem;
extern pthread_t tid;
extern int clientfd;
extern struct sockaddr_in server_addr,client_addr;

int main(int argc, const char *argv[])
{
	if(argc < 2){
		puts("请输入服务器端口号");
		return -1;
	}
	/*网络初始化*/
	int fd = Net_init(argv[1]);
	if(-1==fd){
		return -1;
	}
	

	/*创建链表*/
	L = link_creat();
	if(NULL==L){
		puts("系统初始化失败");
		close(fd);
		return -1;
	}
	
	/*信号量初始化*/
	if(-1==sem_init(&linksem,0,1)){
		puts("信号量初始化失败");
		return -1;
	}
	
	/*消息队列初始化*/
	key = ftok(MSGPATH,'w');
	if(-1==key){
		puts("系统初始化失败");
		return -1;
	}
	msgrid = msgget(key,IPC_CREAT | 0666);
	if(-1==msgrid){
		puts("系统初始化失败");
		return -1;
	}
	
	key = ftok(MSGPATH,'r');
	if(-1==key){
		puts("系统初始化失败");
		return -1;
	}
	msgwid = msgget(key,IPC_CREAT | 0666);
	if(-1==msgwid){
		puts("系统初始化失败");
		return -1;
	}
	
	/*创建心跳包线程，检索下位机是否存活*/
	pthread_create(&tid,NULL,insprct_thread,NULL);
	pthread_detach(tid);
	
	/*创建检索下位机状态线程*/
	pthread_create(&tid,NULL,getlinkstaut,NULL);
	pthread_detach(tid);


	int lenth = sizeof(client_addr);
	while(1){
		/*等待连接*/
		if(-1==(clientfd = accept(fd,(struct sockaddr *)&client_addr,(socklen_t *)&lenth)))
		{
			puts("连接失败");
			msgctl(msgrid,IPC_RMID,NULL);
			return -1;
		}
		puts("有下位机到来");
		/*创建处理线程*/
		pthread_create(&tid,NULL,handl_thread,&clientfd);
		pthread_detach(tid);
	}
	return 0;
}
