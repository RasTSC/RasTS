/*
 ============================================================================
 Created on  : 2014. 9. 21.
 Name        : raspberryServer.c
 Author      : JeongHakOh
 Version     :
 Copyright   : JeongHakOh
 Description : C Language based Server, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "Times.h"

#define BUF_SIZE 1024
#define MAX_CLIENT 5

void error_handling(char *message);
void read_childproc(int sig);

int main(int argc, char *argv[])
{
	int server_sock, client_sock;
	struct sockaddr_in server_addr, client_addr;

	pid_t pid;
	struct sigaction act;
	socklen_t addr_size;
	int str_len;
	int childcnt=0;

	if(argc!=2) {
		printf("Usage : ./%s <port>\n", argv[0]);
		exit(1);
	}

	// signal setting
	act.sa_handler = read_childproc;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if(sigaction(SIGCHLD, &act, 0) != 0)
		error_handling("LOG ( Sigaction() error. )\n");

	// socket create
	if((server_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		error_handling("LOG ( Can't open stream socket. )\n");

	// socket addr setting
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	server_addr.sin_port=htons(atoi(argv[1]));

	// socket bind
	if(bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) == -1)
		error_handling("LOG ( Bind() error )\n");

	// stay for client
	if(listen(server_sock, MAX_CLIENT) == -1)
		error_handling("LOG ( Listen() error )\n");

	while(1)
	{
		// client connetion
		addr_size = sizeof(client_addr);

		// try to client accept
		if((client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size)) == -1)
			continue;
		else
			puts("LOG ( New client connected... )");

		if(pid = fork())
			childcnt++;

		if(pid == -1)
		{
			close(client_sock);
			continue;
		}

		// child process
		if(pid == 0)
		{
			close(server_sock);

			int tempSec = getSec();
			int modSec = tempSec % 2;
			char buf[BUF_SIZE];
			memset(buf, NULL, sizeof(buf));

			sprintf(buf, "test[ %d ] = 0101010101010101010101010\n", childcnt );

			while(1)
			{
				char strTemp[256] = { 0, };

				int y = getYear();
				int mon = getMonth();
				int d = getDay();
				int h = getHour();
				int min = getMin();
				int s = getSec();

				if (s % 2 == modSec && tempSec != s) {
					sprintf(strTemp,
							"%04d-%02d-%02d %02dhour %02dmin %02dsec\n", y, mon,
							d, h, min, s);
					strcat(strTemp, buf);
					tempSec = s;

					if(write(client_sock, buf, strlen(buf)) < 0)
						break;
					printf("send to client : %s\n", strTemp);

					memset(strTemp, NULL, sizeof(strTemp));
				}
			}

			close(client_sock);
			puts("LOG ( Client disconnected... )");
			return 0;
		}
		else
			close(client_sock);
	}
	close(server_sock);
	return 0;
}

void read_childproc(int sig)
{
	pid_t pid;
	int status;
	pid=waitpid(-1, &status, WNOHANG);
	printf("LOG ( Removed proc id: %d ) \n", pid);
}

void error_handling(char *message)
{
	fputs(message, stderr);
	exit(1);
}
