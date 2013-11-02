/*
 * =====================================================================================
 *
 *       Filename:  client.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年08月04日 19时08分08秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lgt, lgtcarol@gmail.com
 *        Company:  Class 1203
 *
 * =====================================================================================
 */

#include "client.h"
#include "sonclient.c"
int  g_idflg = 0;
char g_warncode[80]; 				//用来存放不断输出的命令提示符
char g_path[SIZE]; 				//下载保存路径或上传路径（）
int main(int argc, char *argv[])
{
	char 			*pnta; 		//指向@的指针
	char 			usr[NAMLEN];
	int 			confd;
	struct sockaddr_in      servaddr;       //save the server's ip
	char 			recvbuf[BUFSIZE];
	char 			inptbuf[BUFSIZE];
	int 			cmd; 					//保存命令种类所带表的宏
	int 			predealcmdout; 			// 保存处理输入命令的返回值，若不用会导致多一遍无用的函数调用。
	char 			pwd[SIZE]; 			//保存当前目录

	/*初始化地址结构*/
	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVPORT);
	/*获取服务器ip*/
	if(argc != 2)
	{
		printf("用法：[可执行程序] [用户名@ip](或只跟ip作为匿名用户) \n");
		exit(0);
	}
	else
	{
		pnta = strchr(argv[1], '@');
		if(strchr(argv[1], '@') != NULL)
		{
	            if(inet_aton(pnta+1, &servaddr.sin_addr) == 0)
	            {
	      		printf("ip地址获取失败！\n");
			exit(1);
		    }
		    g_idflg = 1; 			//标识此非匿名用户
		    strncpy(usr, argv[1], pnta-argv[1]);
		    usr[pnta-argv[1]] = '\0'; 			//将要给客户端发送的用户名
	      	}
		else
		{
	            if(inet_aton(argv[1], &servaddr.sin_addr) == 0)
	            {
	      		printf("ip地址获取失败！\n");
			exit(1);
		    }
		}
	}
	/*建立客户端套接字并进行连接*/
	confd = socket(AF_INET, SOCK_STREAM, 0);
	if(confd < 0)
	      err("socket", __LINE__);
		
	if(connect(confd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr)) < 0)
	      err("connect", __LINE__);
	memset(inptbuf, '\0', BUFSIZE);
	/*要开始用户进行身份验证*/
	if(g_idflg == 1)
		if(senddata(confd, "usr"))
			if(recvdata(confd, recvbuf) && strncmp(recvbuf, "name", 4) == 0)
			      if(senddata(confd, usr)) 			  	//发送用户名
			      {
					if(recvdata(confd, recvbuf) && strncmp(recvbuf, "code", 4) == 0)
					{
					    printf("口令：");
					    system ("stty -echo");//关闭回显
					    getinpt(inptbuf); 			//输入处理函数
					    system ("stty echo");//打开回显
					    printf("\n");
				            if(senddata(confd, inptbuf))
					    	if(!recvdata(confd, recvbuf) || strncmp(recvbuf, "login", 5) != 0)
						      exit(0); 			//口令错误退出
					}
					   else
						 exit(0); 			//用户名错误退
			      }
	if(g_idflg == 0)
	{
		if(senddata(confd, "anony"))
		      cmd = recvdata(confd, recvbuf);
	}
	/*进行操作*/
	strcpy(g_warncode, argv[0]);
	strcat(g_warncode, " ");
	strcat(g_warncode, argv[1]);
	strcat(g_warncode, ":> ");
	while(1)
	{
		printf("%s", g_warncode); 					//命令提示行
		getinpt(inptbuf); 					//输入命令
		cmd = fndcmd(inptbuf); 					//提取出命令种类
		switch(cmd)
		{
			case HELP :  dealcmd( confd, HELP, inptbuf); break; 			//此处并不用输出字节数，故投机掩饰
			case PWD  :  dealcmd( confd, PWD, inptbuf);  break;
			case FIND :  dealcmd(confd, FIND, inptbuf);  break;
			case LS   :  dealcmd(confd, LS, inptbuf);    break;
			case CD   :  dealcmd(confd, CD, inptbuf);    break;
			case RM   :  dealcmd(confd, RM, inptbuf);    break;
			case MKDIR:  dealcmd(confd, MKDIR, inptbuf); break;		
			case LLS  :  if(predealcmd(inptbuf, cmd))
				     {  
				     	getcwd(pwd, SIZE);
			 		picklcpath(inptbuf, usr);
			 		if(fndpath(usr) == ISDIR) 
			 		{				//该目录存在(借用usr)
				     		trvdir(usr);
			                        chdir(pwd);
			                 }
				     	else
				     		printf("本地无此目录!\n");
				     }
				     else 
				         printf("输入格式有误!\n"); 
				     break;		
			case LCD : if(predealcmd(inptbuf, cmd))
				    {
					picklcpath(inptbuf, usr);
					if(fndpath(usr) == ISDIR)
					{
						chdir(usr);
						getcwd(pwd, SIZE);
						printf("本地目录已转到：%s\n", pwd);
					}
					else
						printf("本地获取目录失败!\n");
				   }
			           else
					printf("输入格式有误！\n");
				    break;
			case GET :  predealcmdout = predealcmd(inptbuf, GET);
				    if(predealcmdout ==  SUCCESS)
				   {
					 if(senddata(confd, inptbuf))
					 {
					       if(recvdata(confd, recvbuf) && strncmp(recvbuf, "fail", 4) != 0)
					       {
					       		printf("欲下载的文件字节数：%s\n", recvbuf);
							senddata(confd,"already");
					       		recvfile(confd, g_path);
					       	}
					       else
						     printf("%s\n", recvbuf);
					 }
				   }
				   if(predealcmdout == SPECIAL)
					 printf("您的本地路径名有误！\n");
				   if(predealcmdout == FAIL)
					 printf("输入格式有误！\n");
				    break;
			case PUT :  predealcmdout = predealcmd(inptbuf, PUT);
				    if(predealcmdout ==  SUCCESS)
				   {
					 if(senddata(confd, inptbuf))
					 {
					       if(recvdata(confd, recvbuf) && strncmp(recvbuf, "fail", 4) != 0)
					       {
					       	       sendfile(confd, g_path);
						       printf("发送成功\n");
					       }
					       else
						     printf("%s\n", recvbuf);
					 }
				   }
				   if(predealcmdout == SPECIAL)
					 printf("获取本地文件失败！\n");
				   if(predealcmdout == FAIL)
					 printf("输入格式有误\n");
				    break;
			case BYE  : if(senddata(confd, "bye"))
				    	exit(0);
			case SPECIAL : printf("权限不够\n");
				       	break;
			default   :  printf("未知命令\"%s\"\n",inptbuf);
				     	break;
		}
	}

	return EXIT_SUCCESS;
}
void picklcpath(char inptbuf[], char path[])
{
	int 		i = 0; 					//用来找第n个串
	char 		*token, *cur = inptbuf;
	while(token = strsep(&cur, " "))
	{
		i++;
		if(i == 2)
			strcpy(path, token);
	}

}
/*上传文件 */
void sendfile(int confd, char filename[])
{
	int 			fd;
	int 			ret;
	struct stat 		buf;
	char 			inptbuf[BUFSIZE];
	long  			filesize = 0;
	if((fd = open(filename, O_RDONLY)) < 0)
	      err("open", __LINE__);
        if(lstat(filename, &buf) == -1)               //文件属性的获得
              err("lstat", __LINE__);
        
        printf("%ld 字节上传中.....\n",buf.st_size);
	while(1)
	{
		memset(inptbuf, '\0', BUFSIZE);
		if((ret = read(fd, inptbuf, BUFSIZE)) < 0)
		      err("read", __LINE__);
		if(send(confd, inptbuf, ret, 0) < 0)
		      err("read", __LINE__);
		if(ret < BUFSIZE)
		{
		      filesize += ret;
		      printf("%ld 字节已上传成功\n",filesize);
		      return ;
		}
		filesize += ret;
	}
}
/*发送数据*/
int senddata(int confd, char *str)
{
	char 			inptbuf[BUFSIZE];
	if(send(confd, str, strlen(str), 0) < 0)
	      err("send", __LINE__);
	return SUCCESS;
}
/*接收下载文件*/
void recvfile(int confd, char path[])
{
	int 			fd;
	int 			ret; 						//接收的字符数
	struct stat 		buf;
	char 			recvbuf[BUFSIZE];
	long 			filesize = 0; 					//接收的文件大小
loop:	if((fd = open(path, O_RDWR|O_CREAT|O_EXCL,S_IRWXU)) == -1) 	//文件存在则打开错误
	{
		if(fndpath(path) == ISFILE)
		{
		     strcat(path, "new");
		     printf("重名出路后的文件名: %s\n", path);
		     goto loop;
		}
		else
		     err("open",__LINE__);		   
	}

	while(1)
	{
		memset(recvbuf, '\0', BUFSIZE);
		if((ret = recv(confd, recvbuf, BUFSIZE, 0)) < 0)
		      err("recv", __LINE__);
		if((write(fd, recvbuf, ret)) < 0)
		      err("write", __LINE__);
		if(ret < BUFSIZE)
		{
			filesize += (long)ret;
			printf("已成功接收完毕%ld字节\n", filesize);
			close(fd);
			return ;
		}
		filesize += (long)ret;
	}
}
/*接受数据*/
int recvdata(int confd, char *recvbuf)
{
	int 			ret;
	memset(recvbuf, '\0', BUFSIZE);
	if((ret = recv(confd, recvbuf, BUFSIZE, 0)) < 0)
	      err("recv", __LINE__);
	if(strncmp(recvbuf, "bye", 3) == 0)
	{
		printf("处理异常，请检查后重新输入！\n");
		return 0;
	} 							//此处理要求调用函数一旦不能正常接受必须结束！！！！！！！
}
/*检查路径或文件是否存在*/
int fndpath(char path[])
{
	struct stat 		buf;
	if(stat(path, &buf) == -1)
	{
		perror("stat");
		return FAIL;
	}
	if(S_ISDIR(buf.st_mode))
	      return ISDIR;
	else
	      return ISFILE;
}
/*输入函数*/
void getinpt(char *inptbuf)
{
	char 			ch; 			//记录每个字符
	int 			i = 0;

	memset(inptbuf, '\0', BUFSIZE);
	while((ch = getchar()) != '\n' && i < BUFSIZE)           
	{
	      inptbuf[i] = ch;
	      i++;
	}
	inptbuf[i] = '\0';
}
/*查看有无此命令*/
int fndcmd(char inptbuf[])
{
	int 			cmd; 				//记录返回宏
	char 			*head = inptbuf;
	if(strncmp(inptbuf, "help", 4) == 0)
	      cmd = HELP;
	else if(strncmp(inptbuf, "ls", 2) == 0 )
	      cmd = LS;
	else if(strncmp(inptbuf, "lls", 3) == 0 )
	      cmd = LLS;
	else if(strncmp(inptbuf, "lcd", 3) == 0 )
	      cmd = LCD;
	else if(strncmp(inptbuf, "pwd", 3) == 0)
	      cmd = PWD;
	else if(strncmp(inptbuf, "cd", 2) == 0)
	      cmd = CD;
	else if(strncmp(inptbuf, "find", 4) == 0)
	      cmd = FIND;
	else if(strncmp(inptbuf, "get", 3) == 0)
		cmd = GET;
	else if(strncmp(inptbuf, "rm", 2) == 0)
		cmd = chkright(RM);
	else if(strncmp(inptbuf, "mkdir", 5) == 0)
		cmd = chkright(MKDIR);
	else if(strncmp(inptbuf, "put", 3) == 0)
		cmd = chkright(PUT);
	else if(strncmp(inptbuf, "bye", 3) == 0)
	      cmd = BYE;
	else 
	      cmd = FAIL;
	return cmd;
}

/*发送命令前的预处理*/
int predealcmd(char inptbuf[], int cmd)
{
	int 			i = 0; 						//记录有效的处理串数
	char 			bakinptbuf[BUFSIZE]; 			//副本，避免破坏原数据， 后面借来保存当前目录
	char 			*token, *cur;
	char 			savetoken[SIZE][SIZE]; 			//保存解析出的串，以便用到
	char 			dirpath[SIZE]; 				//下载到的本地目录的路径
	strcpy(bakinptbuf, inptbuf);
	cur = bakinptbuf;
	while(token = strsep(&cur, " "))
		strcpy(savetoken[i++], token);
	switch(cmd)
	{
		case PWD  :
		case HELP : if(i == 1)
				return SUCCESS;
			    else
			    	break;
		case RM   :
		case CD   :
		case LCD  :
		case MKDIR:
		case FIND : if(i == 2)
				return SUCCESS;
			    else 
				  break;
		case LLS  :
		case LS   : if(i == 1)
			    {
				  strcat(inptbuf, " .");
				  return SUCCESS;
			    }
			    if(i == 2)
				  return SUCCESS;
			    else 
				  break;
		case GET :  if(i == 4)
			    {
			      if(strcmp(savetoken[2], "-o") == 0)
			      {
			    	pickdir(savetoken[3], dirpath);
			    	if(fndpath(dirpath) == ISDIR) 				//该目录存在
				{
				     strcpy(g_path, savetoken[3]);
				     return SUCCESS;
				}
				else 
				      return SPECIAL; 					//路径有误
			      }
			      else 
				  break; 						//第三个参数不是"-o"
			    }
			   if(i == 2)
			   {
				memset(bakinptbuf, '\0', BUFSIZE);
				pickfle(savetoken[1], bakinptbuf); 			//提取出文件名
				strcpy(g_path, "./download/");
			   	strcat(g_path, bakinptbuf);
				return SUCCESS;
			   }
			   break;
		case PUT : if(fndpath(savetoken[1]) == ISFILE)
			   {
				strcpy(g_path, savetoken[1]);
				if(i == 4 && strcmp(savetoken[2], "-o") == 0)
					return SUCCESS;
				else if(i == 2)
				{
				     char  		temp[SIZE]; 			//用来暂存上传的文件名
				     strcat(inptbuf, " -o /home/lgt/lgtftp/upload/");
				     pickfle(savetoken[1], temp);
				     strcat(inptbuf, temp);
				     return SUCCESS;
				}
				else 
					break; 					//格式不正确
			   }
			   else
				 return SPECIAL; 				//无法获取文件
		default   : break;
	}
	return FAIL;
}

void pickfle(char src[], char filename[])
{
	char 			*head = src; 				//记录该串的开始位置
	puts(src);
	while(*src != '\0')
	      src++;           						//找到该串的末尾
	while(*src != '/' && src != head)
	      src--; 							//指向最后一个分割符
	strcpy(filename, src); 						//获取该文件名
}
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
void err(char *str, int line)
{
	fprintf(stderr, "line: %d", line);
	perror(str);
	exit(1);
}
void dealcmd(int confd,int cmd, char inptbuf[])
{
	char 			recvbuf[BUFSIZE];
	if(predealcmd(inptbuf, cmd))
		if(senddata(confd, inptbuf))
			if(recvdata(confd, recvbuf))
				printf("%s\n", recvbuf);
}
int chkright(int cmd)
{
	if(g_idflg)
		      return cmd;
	else
		      return SPECIAL;
}

