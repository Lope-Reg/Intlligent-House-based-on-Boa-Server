#include "project.h"

link_t *L;
key_t key;
sem_t linksem;
pthread_t tid;

int msgrid;
int msgwid;

msg_t msg;
int clientfd;
struct sockaddr_in server_addr, client_addr;

static int opt = 1;

struct timeval timeout;
// timeout.tv_sec = 0; // 将超时时间设置为0，取消超时设置
// timeout.tv_usec = 0;

int Net_init(const char *PROT)
{
	int nfd = socket(AF_INET, SOCK_STREAM, 0);
	if (nfd < 0)
	{
		puts("网络初始化失败");
		return -1;
	}
	/*设置端口复用*/
	if (setsockopt(nfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
	{
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}
	/*填充信息结构体*/
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(atoi(PROT));
	/*绑定*/
	if (bind(nfd, (struct sockaddr *)&server_addr, sizeof(server_addr)))
	{
		puts("网络初始化失败");
		close(nfd);
		return -1;
	}
	/*设置监听*/
	if (listen(nfd, MAXLINK) < 0)
	{
		puts("网络初始化失败");
		close(nfd);
		return -1;
	}

	return nfd;
}

void *handl_thread(void *argv)
{
	int fd = *((int *)argv);
	FILE *fp;
	char filedata[128];
	char ID[20];
	char PS[20];
	char *s;
	int i;
	long msgsum = 0;

	msg_t buf;

	// 处理登录
	if (0 == recv(fd, &buf, sizeof(msg_t), 0))
	{
		close(fd);
		pthread_exit(NULL);
	}
	printf("用户%s下位机请求登录系统\n", buf.login.id);
	printf("密码:%s\n", buf.login.ps);

	fp = fopen("./login/userinfo.txt", "a+");
	if (NULL == fp)
	{
		puts("系统启动失败，用户信息管理文件不存在");
		exit(0);
	}

	memset(filedata, 0, 128);
	while (fgets(filedata, 128, fp))
	{
		s = filedata;
		i = 0;
		memset(ID, 0, 20);
		memset(PS, 0, 7);
		while (*s)
		{
			if (*s == ' ')
			{
				s++;
				ID[i] = '\0';
				strncpy(PS, s, 7);
				PS[6] = '\0';
				break;
			}
			ID[i++] = *s;
			s++;
		}

		if (0 == strcmp(ID, buf.login.id) && 0 == strcmp(PS, buf.login.ps))
		{

			buf.login.flags = 1;
			break;
		}
		buf.login.flags = 0;
	}

	fclose(fp);

	if (buf.login.flags)
	{

		pthread_t tid = pthread_self();

		/*获取信号量 插入链表*/
		sem_wait(&linksem);
		insert_data(L, fd, buf.login.id, tid);
		sem_post(&linksem);

		send(fd, &buf, sizeof(msg_t), 0);

		s = buf.login.id;
		while (*s)
		{
			msgsum += *s;
			s++;
		}

		while (1)
		{
			/*等待读队列中的消息*/
			msgrcv(msgrid, &buf, sizeof(msg_t) - sizeof(long), msgsum, 0);
			/*将消息发送至智能硬件*/
			send(fd, &buf, sizeof(msg_t), 0);
			/*等待智能硬件执行结果*/
			memset(&buf, 0, sizeof(msg_t));
			recv(fd, &buf, sizeof(msg_t), 0);
			/*将执行结果返回到写消息队列*/
			buf.msgtype = msgsum;
			msgsnd(msgwid, &buf, sizeof(msg_t) - sizeof(long), 0);
		}
	}
	else
	{
		printf("登录失败\n");
		buf.login.flags = 0;
		send(fd, &buf, sizeof(msg_t), 0);
		close(fd);
		pthread_exit(NULL);
	}
}

void *insprct_thread(void *argv)
{
	while (1)
	{
		show_list(L);
		puts("10秒后检查下位机在线状态");
		sleep(10);
	}
}

/*检索下位机在线状态线程*/
void *getlinkstaut(void *argv)
{
	msg_t buf;
	int ret;
	long rcvtype = 0;

	while (1)
	{
		memset(&buf, 0, sizeof(msg_t));
		msgrcv(msgrid, &buf, sizeof(msg_t) - sizeof(long), 1, 0);
		rcvtype = buf.rcvtype;

		sem_wait(&linksem);
		ret = idfind(L, buf.login.id);
		sem_post(&linksem);

		if (ret)
		{
			buf.login.flags = 1;
			buf.msgtype = rcvtype;
		}
		else
		{
			buf.login.flags = 0;
			buf.msgtype = rcvtype;
		}
		msgsnd(msgwid, &buf, sizeof(msg_t) - sizeof(long), 0);
	}
}

/*创建链表*/
link_t *link_creat(void)
{
	link_t *L = (link_t *)malloc(sizeof(link_t));
	if (NULL == L)
	{
		return L;
	}

	memset(L, 0, sizeof(link_t));
	L->next = NULL;
	return L;
}

/*增加数据*/
int insert_data(link_t *L, int fd, const char *name, pthread_t tid)
{
	link_t *p = link_creat();
	if (NULL == p)
	{
		return -1;
	}

	p->fd = fd;
	p->tid = tid;
	strcpy(p->id, name);
	p->next = NULL;

	p->next = L->next;
	L->next = p;

	return 0;
}

/*根据fd删除*/
int delete_data(link_t *L, int fd)
{
	link_t *p = L;
	while (p->next)
	{
		if (p->next->fd == fd)
		{
			link_t *q = p->next;
			p->next = q->next;
			free(q);
			return 0;
		}
		p = p->next;
	}
	return -1;
}

/*根据ID查找节点*/
int idfind(link_t *L, const char *name)
{
	link_t *p = L->next;

	while (p)
	{
		if (0 == strcmp(p->id, name))
		{
			return 0;
		}
		p = p->next;
	}

	return -1;
}

int show_list(link_t *L)
{
	msg_t buf;
	link_t *q = L->next;
	while (q)
	{
		memset(&buf, 0, sizeof(msg_t));
		buf.commd = 255;
		send(q->fd, &buf, sizeof(msg_t), 0);

		/*设置描述符超时检测3秒钟*/
		timeout.tv_sec = 3; // 将超时时间设置为0，取消超时设置
		timeout.tv_usec = 0;
		if (setsockopt(q->fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0)
		{
			perror("setsockopt failed");
			continue;
		}

		/*返回分为两种情况，小于等于 0 描述符出现异常，否则正常*/
		if (0 >= recv(q->fd, &buf, sizeof(msg_t), 0))
		{
			timeout.tv_sec = 0; // 将超时时间设置为0，取消超时设置
			timeout.tv_usec = 0;
			if (setsockopt(q->fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0)
			{
				perror("setsockopt failed");
				continue;
			}
			printf("%d用户离线\n", q->fd);
			/*将处理此描述符的线程干掉*/
			pthread_cancel(q->tid);
			close(q->fd);
			/*将节点删除掉*/
			sem_wait(&linksem);
			delete_data(L, q->fd);
			sem_post(&linksem);

			/*此处可以优化效率*/
			q = L->next;
		}
		else
		{						/*正常收到返回包  清除此描述符的超时设置*/
			timeout.tv_sec = 0; // 将超时时间设置为0，取消超时设置
			timeout.tv_usec = 0;
			if (setsockopt(q->fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout)) < 0)
			{
				perror("setsockopt failed");
				continue;
			}
			printf("%d用户在线\n", q->fd);

			q = q->next;
		}
	}
	return 0;
}

/*释放链表*/
int free_link(link_t **L)
{
	link_t *q = *L;
	while (q)
	{
		q = (*L)->next;
		free(*L);
		(*L) = q;
	}

	*L = NULL;

	return 0;
}
