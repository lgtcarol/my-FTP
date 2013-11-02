/*
 * =====================================================================================
 *
 *       Filename:  server.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2013年08月04日 15时16分11秒
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  lgt, lgtcarol@gmail.com
 *        Company:  Class 1203
 *
 * =====================================================================================
 */
#include "server.h"
#include "sonserver.c"

int main(int argc, char *argv[])
{
	int 			sockfd, confd;      
	pid_t 			pid; 				//send the son to deal with
	socklen_t 		clilen; 			//to save the info of the client,should know the size
	struct sockaddr_in      cliaddr, servaddr;               //the struct of the port
	int 			optval;
	/*建立套接字并进行监听*/
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	      err("socket", __LINE__);

	optval = 1;
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (void *)&optval, sizeof(int)) < 0)
	      err("setsockopt", __LINE__);

	memset(&servaddr, 0, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERVPORT);
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(sockfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) < 0)
	      err("bind", __LINE__);

	if(listen(sockfd, LISTENQ) < 0)
	      err("listen", __LINE__);

	printf("--------------  the ftp server has been opened --------------\n");
	clilen = sizeof(struct sockaddr_in);
	while(1)
	{
		confd = accept(sockfd, (struct sockaddr*)&cliaddr, &clilen);
		if(confd < 0)
		      err("accept", __LINE__);
		printf("accept a new client, ip : %s\n", inet_ntoa(cliaddr.sin_addr));
		if((pid = fork()) == 0)
		      	sonproc(confd, inet_ntoa(cliaddr.sin_addr));
		else
			close(confd);
		
	}
	return EXIT_SUCCESS;
}

