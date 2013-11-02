/*
 * =====================================================================================
 *
 *       Filename:  sonserver.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年08月24日 19时45分46秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lgt, lgtcarol@gmail.com
 *        Company:  Class 1203
 *
 * =====================================================================================
 */

#ifndef _SONSERVER_H
#define _SONSERVER_H
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
	#define N 			2                       //当前用户数
	#define FAIL                    0 			//函数调用失败
	#define SUCCESS    		1 			//调用成功
	#define ISFILE    		2 			//该路径是文件
	#define ISDIR 			3 			//该路径是目录
	#define SIZE 			160 			//一些短串的数组大小
	#define BUFSIZE 		1024 			//the size of buffer
	#define USERNAM                 20                       //the num of normal user
	#define LINEMAX 		150 			//全屏下一行所能容纳的字符数
	#define LS 			4                       //为与文件夹下载想区分
	struct info
	{
		char name[USERNAM];
		char pasw[7];
	};

	struct info zcmember[N] = {{"lgt", "851554"}, {"bob","654321"}};

	int  recvdata(int confd, char *recvbuf);  //接受处理函数
	void recvfile(int confd, char *recvbuf);  //文件接收函数
	int  senddata(int confd, char *msg);      //简单字符串发送函数
	void sendfile(int confd, char  *filename);//文件传输
	int  pickpath(char recvbuf[], char path[], int n); //提取路径
	int  fndpath(char path[]); 		//查找当前目录下有无此路径
	void trvdir(int confd, char path[], int cmd); //进行目录遍历，获取所有文件
	void display(char filenames[][PATH_MAX], int n, char path[]);
	void idcheck(int confd); 		  //非匿名用户身份验证
	void wterrlog(int confd, char *msg); 		  //记录错误日志
	void wtdialog(int confd, char recvbuf[]);           //记录操作日志
	void pickdir(char src[], char save[]); 		//保存地所在目录
#endif

