/*
 * =====================================================================================
 *
 *       Filename:  sonserver.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年08月20日 15时15分53秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lgt, lgtcarol@gmail.com
 *        Company:  Class 1203
 *
 * =====================================================================================
 */
//#include "sonserver.h"
static char g_baseinfo[1024]; 				//为记录用户的ip,名字（在写入之前先粘时间，再遇到操作也会被粘进去）
static int  g_idflg = 0; 				//默认为匿名登录
static char g_lsbuf[BUFSIZE] = {"目录如下：\n"}; 				//记录递归结果
static char g_stsize[SIZE]; 				//保存文件大小转换后的串
/*子进程中的主函数*/
void sonproc(int confd, char *ip)
{
	char 			recvbuf[BUFSIZE];
	char 			pwd[SIZE]; 		//记录PWD的结果
	char 			path[SIZE]; 		//记录输入命令中的操作对象 
	int 			fndpthout , i = 0; 		//find path out查找路径的返回值

	strcpy(g_baseinfo, ip); 			//先在记录条例里记下ip
	strcat(g_baseinfo, "  ");
	/*非匿名用户的身份验证*/
	if(recvdata(confd, recvbuf))
	{
		if(!strcmp(recvbuf, "usr"))
		{
		     if(senddata(confd, "name"))
			   idcheck(confd); 				//所有信息正确正式开始服务（在检测中把用户名粘上,成功时改g_idflg)
		}
		else
		{
		      strcat(g_baseinfo, "anony");
		      senddata(confd, "anony");
		}
	}
	/*进行客户端请求的处理*/
	while(1)
	{
	   if(recvdata(confd, recvbuf))
	   {
		char 				recvcmd[BUFSIZE]; 			//保存收到的命令
		strcpy(recvcmd, recvbuf);
		if( strncmp(recvbuf, "help", 4) == 0)
		{
		      sendfile(confd, "/home/lgt/lgtftp/.src/help.txt");
		      wtdialog(confd, recvbuf);
		}
		else if(strncmp(recvbuf, "ls", 2) == 0)
		{
			getcwd(pwd, SIZE);
			pickpath(recvbuf, path, 2);
			if(fndpath(path) == ISDIR)
			{
			      trvdir(confd, path,LS);  				  //traver directory遍历目录，并将结果写入文件 	
			      printf("ls即将要切回：%s\n", pwd);
			      chdir(pwd);
			      senddata(confd, g_lsbuf);
			      strcpy(g_lsbuf, "目录如下：\n");
			      wtdialog(confd, recvbuf);
			}
			else
			      senddata(confd, "该目录不存在\n");
		}
		else if(strncmp(recvbuf, "cd" , 2) == 0)
		{
			pickpath(recvbuf, path, 2);
			if(fndpath(path) == ISDIR)
			{
				chdir(path);
		      		getcwd(pwd, SIZE);
				if(strcmp(pwd, "/home/lgt") == 0 || strcmp(pwd, "/home") == 0 || strcmp(pwd, "/") == 0)
				{
					senddata(confd, "bye");
					chdir("/home/lgt/lgtftp");					
				}
		      		else	
				{
					senddata(confd, pwd);
		      			wtdialog(confd, recvbuf);
				}
			}
			else
				senddata(confd, "切换目录失败，请检查后重试\n");
		}
		else if(strncmp(recvbuf, "find", 4) == 0)
		{
			pickpath(recvbuf, path, 2);
			fndpthout = fndpath(path);
			if(fndpthout == ISFILE)
			      senddata(confd, path);
			if(fndpthout == ISDIR) 				//是目录的暂不处理
			{
			      getcwd(pwd, SIZE);
			      trvdir(confd, path, LS);
			      chdir(pwd);
			      senddata(confd, g_lsbuf);
			      strcpy(g_lsbuf, "目录如下：\n");
			      wtdialog(confd, recvbuf);
			}
			if(fndpthout == FAIL)
			      senddata(confd, "no such file or directory");
		}
		else if(strncmp(recvbuf, "mkdir", 5) == 0)
		{
			pickpath(recvbuf, path, 2);
			if(!fndpath(path))
			{
				mkdir(path, 0777);
				senddata(confd, "创建目录成功!\n");
				wtdialog(confd, recvbuf);
			}
			else
				senddata(confd, "创建失败，可能有重名文件或目录！");
		}
		else if(strncmp(recvbuf, "get", 3) == 0)
		{
			pickpath(recvbuf, path, 2);
			fndpthout = fndpath(path);
			if(fndpthout == ISFILE)
			{

			      struct stat 		buf;
			      if(lstat(path, &buf) == -1) 		//文件属性的获得
		     			 wterrlog(confd, recvbuf);
			      sprintf(g_stsize, "%d", (int)buf.st_size);
			      if(senddata(confd, g_stsize))
			     {
					recvdata(confd, recvbuf);
					printf("接受到服务器的确认：%s\n", recvbuf);
			      		sendfile(confd, path);
					wtdialog(confd, recvcmd);
			     }
			}
			if(fndpthout == FAIL)
			      senddata(confd, "fail,no such file or directory");
			if(fndpthout == ISDIR)
			      senddata(confd, "fail, 对象是目录");
		}
		else if(strncmp(recvbuf, "put", 3) == 0)
		{
			char 			dirpath[SIZE]; 				//提取保存地的目录名，判断是否存在
			pickpath(recvbuf, path, 4);
			pickdir(path, dirpath);
			if(fndpath(dirpath) == ISDIR)
			{
				if(senddata(confd, "already"))
				 		recvfile(confd, path);
				wtdialog(confd, recvbuf);
			}
			else
			      senddata(confd, "fail, 服务器不存在该目录");
		}
		else if(strncmp(recvbuf, "rm", 2) == 0)
		{
			pickpath(recvbuf, path, 2);
			fndpthout = fndpath(path);
                        if(fndpthout == ISFILE)
			{
				unlink(path);
				senddata(confd, "成功删除文件！\n");
				wtdialog(confd, recvbuf);
			}
			else if(fndpthout == ISDIR)
			{
				if(rmdir(path) != 0)
					senddata(confd, "失败！该目录非空！\n");
				else
				        senddata(confd, "成功删除目录！\n");
				wtdialog(confd, recvbuf);
			}
			else
				senddata(confd, "无此目录或文件！\n");
		}
		else if(strncmp(recvbuf, "bye", 3) == 0)
		{
			printf("ip为%s的用户已经退出\n", ip);
			close(confd);
			exit(0);
		}     
		else if(strncmp(recvbuf, "pwd", 3) == 0)
		{
		      getcwd(pwd, SIZE);
		      senddata(confd, pwd);
		      wtdialog(confd, recvbuf);
		}    
	   }
	}
}

/*递归遍历目录*/
void trvdir(int confd, char path[], int cmd)
{
	DIR 			*dir;
	struct stat 		det;
	struct dirent 		*det1;
	char 			filenames[256][PATH_MAX];
	int 			i = 0;

	if((dir = opendir(path)) == NULL)
	{
		wterrlog(confd, strerror(errno));
		return ;
	}
	chdir(path);
	while((det1 = readdir(dir)) != NULL )
	{
		if(strcmp(det1->d_name, ".") == 0 || strcmp(det1->d_name, "..") == 0 || strncmp(det1->d_name, ".", 1) == 0)
		      continue;
		strcpy(filenames[i++], det1->d_name);
		if(lstat(det1->d_name, &det) == -1)
		{
			perror("lstat");
			wterrlog(confd, strerror(errno));
			return ;
		}
		if(S_ISDIR(det.st_mode) && cmd == LS)
		     trvdir(confd, det1->d_name, LS);
	}
	if(cmd == LS)
	      display(filenames, i, path);
	chdir("..");
	closedir(dir);
}
/*将所有信息写入文件g_lsbuf中，避免来回写时出错*/
void display(char filenames[][PATH_MAX], int n, char path[])
{
	int 			i, k, j = 0;
	int 			linemax = 0, nlinemax;
	int 			fd;
	for(i = 0; i < n; i++)
	{
		if(strlen(filenames[i]) > linemax)
		      linemax = strlen(filenames[i]); 				//获取最长文件名的长度
	}
	nlinemax = LINEMAX / (linemax + 2);
	strcat(g_lsbuf, path);
	strcat(g_lsbuf, "\n");
	for(i = 0; i < n; i++)
	{
		strcat(g_lsbuf, filenames[i]);
		for(k = 0; k < linemax + 2 - strlen(filenames[i]); k++)
		      strcat(g_lsbuf, " ");
		if((++j) % nlinemax == 0)
		      strcat(g_lsbuf, "\n");
	}
	strcat(g_lsbuf, "\n\n");
}

/*简单的数据接受*/
int recvdata(int confd, char *recvbuf)
{
	int 			ret; 					//接受到的字符数
	
	memset(recvbuf, '\0', BUFSIZE);
	if((ret = recv(confd, recvbuf, BUFSIZE, 0))<= 0)
	{
		wterrlog(confd, strerror(errno));		//将错误信息放到一个数组，将来传给错误日志函数（将在写入后关闭本次连接，并告知client）
		return 0;
	}
	return ret; 						//返回读取字符数

}
/*发送一些字符串*/
int senddata(int confd, char *str)
{
	char 			inptbuf[BUFSIZE];
	char 			errinfo[BUFSIZE];
	memset(inptbuf, '\0', BUFSIZE);
	strcpy(inptbuf, str);
	if(send(confd, inptbuf, strlen(str), 0) < 0)
	{
		wterrlog(confd, strerror(errno));
		return ;
	}
	return SUCCESS;
}

/*文件的传送*/
void sendfile(int confd, char *filename)
{
	int 			fd;
	int 			mid;  			//同时作为发送数据的统变量
	struct stat 		buf;
	char 			inptbuf[BUFSIZE];
	char 			recvbuf[BUFSIZE];
	if( (fd = open(filename, O_RDONLY)) < 0 )
	{
		puts(strerror(errno));
		wterrlog(confd, strerror(errno)); 		//自己就写完发送bye
		return ;
	}
		printf("~~~~~~~~~~~~~~~~~~~~~~sendfile: %s~~~~~~~~~~~~~~~~~~~\n",filename);
	while(1)
	{
		memset(inptbuf, '\0', BUFSIZE);
		if((mid = read(fd, inptbuf, BUFSIZE)) < 0)
		{
		      printf("读文件失败\n");
		      wterrlog(confd, strerror(errno));
		      return ;
		}
		if(send(confd, inptbuf, mid, 0) < 0)
		{
		      printf("发送失败\n");
		      wterrlog(confd, strerror(errno));
		      return ;
		}
		if(mid < BUFSIZE)
                {
			printf("发送完毕\n");
			close(fd);
			return ;
		}
	}
}
/*文件的接收*/
void recvfile(int confd, char path[])
{
	int 			fd;
	int 			ret;
	char 			recvbuf[BUFSIZE];
	struct stat 		buf;
	printf("~~~~~~~~~~~~~~~~~~~~~~~~recvfile: %s~~~~~~~~~~~~~~~~~~~~~~~~~~\n", path);
loop: 	if((fd = open(path, O_RDWR|O_CREAT|O_EXCL, S_IRWXU)) == -1)
	{
		if(fndpath(path) == ISFILE)
		{
			strcat(path, "xin");
			printf("自己修改后的文件名: %s\n", path);
			goto loop;
		}
		else
		      wterrlog(confd, strerror(errno));
	}
	while(1)
	{
		memset(recvbuf, '\0', BUFSIZE);
		if((ret = recv(confd, recvbuf, BUFSIZE, 0)) < 0)
		{
			wterrlog(confd, strerror(errno));
			return ;
		}
		if(write(fd, recvbuf, ret) < 0)
		{
			wterrlog(confd, strerror(errno));
			return ;
		}
		if(ret < BUFSIZE)
		      return ;
	}
}
/*从发送的字符串中提取路径*/
int pickpath(char recvbuf[], char path[], int n)
{
	int 		i = 0; 					//用来找第n个串
	char 		*token, *cur = recvbuf;
	while(token = strsep(&cur, " "))
	{
		i++;
		if(i == n)
			strcpy(path, token);
	}

}
/*解析出目录*/
void pickdir(char src[], char save[])
{
	char 			*head = src;
	while(*src != '\0')
	      src++;
	while(*src != '/' && src != head)
	      src--;
	strncpy(save, head, src - head + 1);
	save[src - head + 1] = '\0';
	if(src == head)
	      strcpy(save, "");
}
/*查找路径是否存在*/
int fndpath(char path[])
{
	struct stat 			buf;
	char 				pwd[SIZE];
	if(lstat(path, &buf) == -1)
	{
		perror("stat");
		return FAIL;
	}
	printf("检查过是否能打开\n");
	if(S_ISDIR(buf.st_mode))
	      return ISDIR;
	return ISFILE;
}


void wterrlog(int confd, char *msg)
{
	int 			fd;
	time_t 			t;
	struct tm 		*area;
	char 			errstr[BUFSIZE];

	//时间处理，也粘在串中
	t = time(NULL);
	area = localtime(&t);
	strcpy(errstr, msg);
	strcat(errstr, " ");
	strcat(errstr, asctime(area));
	if((fd = open("/home/lgt/lgtftp/.log/errlog.txt", O_WRONLY | O_APPEND)) < 0 )
	{
	      printf("打开错误日志失败\n");
	      senddata(confd, "bye");
	      return ;
	}
	if(write(fd, errstr, strlen(errstr)) < 0)
		printf("写错误日志失败！\n");
	
         close(fd);
	 senddata(confd, "bye");
	      
}
/*用户操作日志*/
void wtdialog(int confd, char msg[])
{
	int 			fd;
	time_t 			t;
	struct tm 		*area;
	char 			diastr[BUFSIZE];
	//时间处理，也粘在串中
	t = time(NULL);
	area = localtime(&t);
	strcpy(diastr, g_baseinfo);
	strcat(diastr, "  ");
	strcat(diastr, msg);
	strcat(diastr, "  ");
	strcat(diastr, asctime(area));
	if((fd = open("/home/lgt/lgtftp/.log/dialog.txt", O_WRONLY | O_APPEND)) < 0 )
	{
	      wterrlog(confd, strerror(errno));
	      return ;
	}
	if(write(fd, diastr, strlen(diastr)) < 0)
	{
	      wterrlog(confd, strerror(errno));
	      close(fd);
	      return ;
	}
              close(fd);
}


/*登录前的用户身份验证*/
void idcheck(int confd)
{
	char 			recvbuf[BUFSIZE];
	int 			i;

	if(recvdata(confd,recvbuf))
	{
		for(i = 0; i < N; i++)
		{
			if(strcmp(recvbuf, zcmember[i].name) == 0)
			{
			    if(senddata(confd, "code")) 				//用户名正确
			    {
				  if(recvdata(confd, recvbuf) && strcmp(recvbuf, zcmember[i].pasw) == 0)
				  {
					if(senddata(confd, "login")) 			//密码正确
					{
					      strcat(g_baseinfo, zcmember[i].name); 	//记上用户名
					      g_idflg = 1; 				//标识此非匿名用户
					}
				  }
				  else
					if(senddata(confd, "fail"))
					      return;
			    }
					
			    break;
			}
		}
		if(i == N)
			if(senddata(confd, "fail"))
		       		return ;
	}

}


