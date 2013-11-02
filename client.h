#ifndef _CLIENT_H
#define _CLIENT_H
	#include <unistd.h>
	#include <assert.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <sys/types.h>
	#include <sys/stat.h>
	#include <fcntl.h>
	#include <sys/socket.h>
	#include <string.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <errno.h>
	#include <time.h>
	#include <limits.h>
	#include<dirent.h>
	#define NAMLEN 			20                       //当前用户数
	#define SPECIAL 		-1 
	#define FAIL                    0 			//函数调用失败
	#define SUCCESS    		1 			//调用成功
	#define ISFILE    		2 			//该路径是文件
	#define ISDIR 			3 			//该路径是目录
	#define SIZE 			160 			//一些短串的数组大小
	#define BUFSIZE 		1024 			//the size of buffer
	#define SERVPORT  		4507 			//the port of server
	//#define USERNAM                 20                       //the num of normal user
	//#define LISTENQ                 10                      //the longest connect queue
	#define LINEMAX 		150 			//全屏下一行所能容纳的字符数
	#define LS 			4                       //为与文件夹下载想区分
	#define PWD 			5
	#define FIND 			6  
	#define GET 			7			
	#define PUT 			8  
	#define HELP 			9  
	#define BYE 			10  
	#define CD 			11 
	#define MKDIR 			12
	#define RM                      13
	#define LLS                     14
	#define LCD                     15
	void getinpt(char path[]); 
	int fndcmd(char inptbuf[]);
	int predealcmd(char inptbuf[], int cmd);
	void pickfle(char src[], char filename[]);
	void err(char *str, int line);	
	int chkright(int cmd);  
	void dealcmd(int confd, int cmd,char inptbuf[]);  //一些命令的处理程序一样
	int  recvdata(int confd, char *recvbuf);  //接受处理函数===========================
	void recvfile(int confd, char *recvbuf);  //文件接收函数==============================
	int  senddata(int confd, char *msg);      //简单字符串发送函数============================
	void sendfile(int confd, char  *filename);//文件传输=======================================
	void  pickpath(char recvbuf[], char path[], int n); //提取路径
	int  fndpath(char path[]); 		//查找当前目录下有无此路径=============================
	void pickdir(char src[], char save[]); 		//保存地所在目录=========================
	void display(char filenames[][PATH_MAX], int n, char path[]);
	void trvdir( char path[]);
	void picklcpath(char inptbuf[], char path[]);
#endif

