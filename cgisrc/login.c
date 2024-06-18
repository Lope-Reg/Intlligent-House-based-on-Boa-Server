#include "project.h"

key_t key;

long msgsum = 0;

/*读队列id*/
int msgrid;

/*发队列id*/
int msgwid;

msg_t msg;

char idbuf[20] = {0};
char psbuf[20] = {0};

char id[] = "yanghanze";
char ps[] = "123456";

void alarmhandl(int argc)
{
	/*查找消息队列中自己的数据*/
	while (-1 == msgrcv(msgwid, &msg, sizeof(msg_t) - sizeof(long), 1, IPC_NOWAIT))
	{
		if (msgsum == msg.rcvtype)
		{
			break;
		}
		msgsnd(msgwid, &msg, sizeof(msg_t) - sizeof(long), 0);
		memset(&msg, 0, sizeof(msg_t));
	}
	FILE *fp = fopen("./neterror.txt", "r");
	if (NULL == fp)
	{
		exit(0);
	}

	char buf[512] = {0};
	printf("Content-type: text/html;charset=\"UTF-8\"\n\n"); // 固定格式 必须要加
	while (fgets(buf, sizeof(buf), fp))
	{
		printf("%s", buf);
		memset(buf, 0, 512);
	}
	fclose(fp);

	exit(0);
}

int cgiMain(int argc, const char *argv[])
{
	/*消息队列初始化*/
	key = ftok(MSGPATH, 'r');
	if (-1 == key)
	{
		return -1;
	}

	msgrid = msgget(key, IPC_CREAT | 0666);
	if (-1 == msgrid)
	{
		return -1;
	}

	key = ftok(MSGPATH, 'w');
	msgwid = msgget(key, IPC_CREAT | 0666);

	/*alarm信号注册*/
	signal(SIGALRM, alarmhandl);
	/*数据库初始化*/
	/*获取网页数据*/
	/*获取用户id*/
	memset(idbuf, 0, sizeof(idbuf));
	cgiFormString("ID", idbuf, 20);
	char *s = idbuf;
	while (*s)
	{
		msgsum += *s;
		s++;
	}

	/*用户密码*/
	cgiFormString("PASSWORD", psbuf, 20);

	/*查表操作*/
	/*根据查表操作决定是否查询下位机在线状态*/
	if (0 == strcmp(id, idbuf) && 0 == strcmp(ps, psbuf))
	{

		memset(&msg, 0, sizeof(msg_t));
		msg.msgtype = 1;
		msg.rcvtype = msgsum;
		strcpy(msg.login.id, idbuf);
		/*发出下位机在线查询请求*/
		msgsnd(msgwid, &msg, sizeof(msg_t) - sizeof(long), 0);

		/*等待下位机在线查询结果*/
		memset(&msg, 0, sizeof(msg_t));
		alarm(5);
		msgrcv(msgrid, &msg, sizeof(msg_t) - sizeof(long), msgsum, 0);

		/*根据结果返回网页*/
		if (!msg.login.flags)
		{
			// printf("下位机在线");
			printf("Set-Cookie:username=%s;path=/;", idbuf);							 // 设置cookie
			printf("Content-type: text/html;charset=\"UTF-8\"\n\n");					 // 固定格式 必须要加
			printf("<script>window.location.href = '/home/living_room.html';</script>"); // 自动跳转
			return 0;
		}
		else
		{
			printf("Content-type: text/html;charset=\"UTF-8\"\n\n"); // 固定格式 必须要加
			FILE *fp = fopen("./NOTLerror.txt", "r");
			if (NULL == fp)
			{
				exit(0);
			}

			char buf[512] = {0};
			while (fgets(buf, sizeof(buf), fp))
			{
				printf("%s", buf);
				memset(buf, 0, 512);
			}
			fclose(fp);
			return 0;
		}
	}
	else
	{
		printf("Content-type: text/html;charset=\"UTF-8\"\n\n"); // 固定格式 必须要加
		FILE *fp = fopen("./IDPSerror.txt", "r");
		if (NULL == fp)
		{
			exit(0);
		}

		char buf[512] = {0};
		while (fgets(buf, sizeof(buf), fp))
		{
			printf("%s", buf);
			memset(buf, 0, 512);
		}
		fclose(fp);
	}
	return 0;
}
