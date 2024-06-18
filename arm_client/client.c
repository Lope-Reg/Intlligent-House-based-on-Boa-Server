#include "project.h"

extern int fd;
extern msg_t buf;
extern char arg[2][128];
extern struct sockaddr_in server_addr;
extern pthread_t tid;

int main(int argc, const char *argv[])
{
	/*读取环境设置配置文件，用于保持环境线程初始化使用*/
	if(read_config(&buf)){
		puts("读取文件失败");
		return -1;
	}
	/*根据用户阈值环境维护线程*/
	pthread_create(&tid,NULL,holdenvthread,NULL);

	/*网络初始化*/
	fd = Net_init(arg[0],arg[1]);
	if(-1==fd){
		return -1;
	}

	/*连接服务器*/
	int lenth = sizeof(server_addr);
	if(-1==connect(fd, (const struct sockaddr *)&server_addr,sizeof(server_addr)))
	{
		puts("连接失败");
		return -1;
	}
	puts("连接成功");

	/*登录*/
	send(fd,&buf,sizeof(msg_t),0);
	recv(fd,&buf,sizeof(msg_t),0);

	if(buf.login.flags){
		puts("登录成功");

		msg_t threadbuf;
		/*等待用户下发指令，执行指令，返回结果*/
		while(1){
			memset(&buf, 0, sizeof(msg_t));
			if (0 == recv(fd, &buf, sizeof(msg_t), 0))
			{
				return -1;
			}
			threadbuf = buf;
			switch(buf.commd){
			case 1:
				pthread_create(&tid,NULL,envgetthread,NULL);
				pthread_detach(tid);
				break;
			case 2:
				pthread_create(&tid,NULL,envsetthread,&threadbuf);
				pthread_detach(tid);
				break;
			case 3:
				pthread_create(&tid,NULL,devctlthread,&threadbuf);
				pthread_detach(tid);
				break;
			case 255:
				send(fd, &buf, sizeof(msg_t), 0);
				break;
			default:
				send(fd, &buf, sizeof(msg_t), 0);
				break;
			}

		}
	}else{
		return -1;
	}

	return 0;
}
