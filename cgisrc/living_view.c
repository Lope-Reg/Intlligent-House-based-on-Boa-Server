#include "project.h"
#include "cgic.h"
key_t key;

/*读队列id*/
int msgrid;

/*发队列id*/
int msgwid;

long msgsum = 0;

msg_t msg;
void alarmhandl(int argc)
{
    msgrcv(msgwid,&msg,sizeof(msg_t)-sizeof(long),msgsum,IPC_NOWAIT);
    FILE *fp = fopen("./neterror.txt","r");
    if(NULL==fp){
        exit(0);
    }

    char buf[512] = {0};
    //printf("Content-type: text/html;charset=\"UTF-8\"\n\n");//固定格式 必须要加
    while(fgets(buf,sizeof(buf),fp)){
        printf("%s",buf);
        memset(buf,0,512);
    }
    fclose(fp);
    exit(0);
}
int cgiMain(int argc,const char *argv[])
{
    printf("Content-type: text/html;charset=\"UTF-8\"\n\n");//固定格式 必须要加
    /*消息队列初始化*/
    key = ftok(MSGPATH,'r');
    if(-1==key){
        return -1;
    }
    msgrid = msgget(key,IPC_CREAT|0666);
    if(-1==msgrid){
        return -1;
    }

    key = ftok(MSGPATH,'w');
    msgwid = msgget(key,IPC_CREAT|0666);
    /*alarm信号注册*/
    signal(SIGALRM, alarmhandl);
    char idbuf[20] = {0};
    char cookiebuf[20] = {0};
    memcpy(cookiebuf,cgiCookie,20);
    char *s = cookiebuf;
    int i = 0;
    while(*s!='='){
        s++;
    }
    s++;
    while(*s){
        idbuf[i]=*s;
    i++;
	s++;
    }
    s = idbuf;
    while(*s){
        msgsum+=*s;
    s++;
    }
    /*发送获取环境数据的请求消息*/
    msg.msgtype = msgsum;
    msg.commd = GETENV;
    msgsnd(msgwid,&msg,sizeof(msg_t)-sizeof(long),0);
    alarm(5);
    /*等待环境数据结果*/
    msgrcv(msgrid,&msg,sizeof(msg_t)-sizeof(long),msgsum,0);
    printf("<!DOCTYPE html>");
    printf("<html>");
    printf("<body>");
    printf("<center>");
    printf("<table border = \"1\">");
    printf("<tr >");
    printf("<td bgcolor=\"yellow\" width=\"80\" height=\"20\">用户ID</td>");
    printf("<td bgcolor=\"yellow\" width=\"80\" height=\"20\">%s</td>",idbuf);
    printf("</tr>");
    printf("</table>");
    printf("<h3>环境数据</h3>");
    printf("<table border = \"1\">");
    printf("<th bgcolor=\"blue\">温度</th>");
    printf("<th bgcolor=\"blue\">湿度</th>");
    printf("<th bgcolor=\"blue\">光强</th>");
    printf("<tr >");
    printf("<td bgcolor=\"red\" width=\"100\" height=\"20\">up %.2f</td>",msg.limitdata.uptemp);
    printf("<td bgcolor=\"red\" width=\"100\" height=\"20\">up %hhd%%</td>",msg.limitdata.uphume);
    printf("<td bgcolor=\"red\" width=\"100\" height=\"20\">up %hdlux</td>",msg.limitdata.uplux);
    printf("</tr>");
    printf("<tr >");
    printf("<td bgcolor=\"green\" width=\"100\" height=\"20\">%.2f</td>",msg.envdata.temp);
    printf("<td bgcolor=\"green\" width=\"100\" height=\"20\">%hhd%%</td>",msg.envdata.hume);
    printf("<td bgcolor=\"green\" width=\"100\" height=\"20\">%hdlux</td>",msg.envdata.lux);
    printf("</tr>");
    printf("<tr >");
    printf("<td bgcolor=\"yellow\" width=\"100\" height=\"20\">down %.2f</td>",msg.limitdata.downtemp);
    printf("<td bgcolor=\"yellow\" width=\"100\" height=\"20\">down %hhd%%</td>",msg.limitdata.downhume);
    printf("<td bgcolor=\"yellow\" width=\"100\" height=\"20\">down %hdlux</td>",msg.limitdata.downlux);
    printf("</tr>");
    printf("</table>");
    printf("<h3>设备状态</h3>");
    printf("<table>");
    printf("<th bgcolor=\"blue\">照明</th>");
    printf("<th bgcolor=\"blue\">温控</th>");
    printf("<th bgcolor=\"blue\">加湿器</th>");
    printf("<tr >");
    if(msg.envdata.devstuat&(0x01<<0)){
        printf("<td bgcolor=\"red\" width=\"100\" height=\"20\">开</td>");
    }else{
        printf("<td bgcolor=\"red\" width=\"100\" height=\"20\">关</td>");
    }
    switch((msg.envdata.devstuat>>1)&0x03){
        case 0x01:
            printf("<td bgcolor=\"red\" width=\"100\" height=\"20\">1 档</td>");
        break;
        case 0x02:
            printf("<td bgcolor=\"red\" width=\"100\" height=\"20\">2 档</td>");
        break;
        case 0x03:
            printf("<td bgcolor=\"red\" width=\"100\" height=\"20\">3 档</td>");
        break;
        case 0x00: 
            printf("<td bgcolor=\"red\" width=\"100\" height=\"20\">关</td>");
        break;
    }
    if(msg.envdata.devstuat&(0x01<<3)){
        printf("<td bgcolor=\"red\" width=\"100\" height=\"20\">开</td>");
    }else{
        printf("<td bgcolor=\"red\" width=\"100\" height=\"20\">关</td>");
    }
    printf("</tr>");
    printf("</table>");
    printf("<br>");
    printf("<form action=\"/cgi-bin/living_view.cgi\" method=\"POST\">");
    printf("<input type=\"submit\" name=\"button\" value=\"点击刷新数据\">");
    printf("</form>");
    printf("</center>");
    printf("</body>");
    printf("</html>");
    return 0;
}