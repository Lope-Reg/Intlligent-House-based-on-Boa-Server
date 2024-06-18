#include "project.h"

int fd;
char arg[2][128];
Login_t  buf;
pthread_t tid;
struct sockaddr_in server_addr;


int Net_init(const char *IP,const char *PROT)
{
	int nfd = socket(AF_INET,SOCK_STREAM,0);
	if(nfd < 0){
		puts("网络初始化失败");
		return -1;
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(IP);
	server_addr.sin_port = htons(atoi(PROT));

	return nfd;

}

/*读取配置文件*/
int read_config(msg_t *buf)
{
	char data[128] = {0};
	char *s = NULL;
	char id[20] = {0};
	int i = 0;
	FILE *fp = fopen(CONFIGPATH,"r");
	if(NULL==fp){
		puts("打开文件失败");
		return -1;
	}

	while(fgets(data,128,fp)){
		/*获取 IP 和 端口号*/
		if(strstr(data,"ip")){
			s = data;
			memset(id,0,20);
			while(*(s++) != '{');
			i = 0;
			while(*s != ','){
				arg[0][i] = *s;
				i++;s++;
			}
			arg[0][i] = '\0';
			printf("ip:%s\n",arg[0]);
			i=0;s++;
			memset(id,0,20);
			while(*s != '}'){
				arg[1][i] = *s;
				i++;s++;
			}
			arg[1][i] = '\0';
			printf("prot:%s\n",arg[1]);
			memset(data,0,128);
			continue;
		}
		/*获取登录ID&密码*/
		if(strstr(data,"login")){
			s = data;
			while(*(s++)!='{');
			i=0;
			while(*s!=','){
				buf->login.id[i] = *s;
				i++;s++;
			}
			buf->login.id[i] = '\0';
			printf("id:%s\n",buf->login.id);
			i=0;
			s++;
			while(*s != '}'){
				buf->login.ps[i] = *s;
				i++;s++;
			}
			buf->login.ps[i] = '\0';
			printf("ps:%s\n",buf->login.ps);
			continue;
		}
		memset(data,0,sizeof(data));

	}
	fclose(fp);
	puts("读取文件完成");
	return 0;
}


/*获取环境数据线程*/
void *envgetthread(void *argv)
{
	msg_t buf;
	buf.envdata.temp = 30.2;
	buf.envdata.hume = 55;
	buf.envdata.lux = 350;
	buf.envdata.devstuat = 0;

	send(fd,&buf,sizeof(msg_t),0);
	puts("~~~~~~~~~~");
	pthread_exit(NULL);
}

/*设置环境阈值线程*/
void *envsetthread(void *argv)
{

}

/*设备控制线程*/
void *devctlthread(void *argv)
{

}

/*根据用户阈值环境维护线程*/
void *holdenvthread(void *argv)
{
	while(1){
		sleep(1);
	}
}
