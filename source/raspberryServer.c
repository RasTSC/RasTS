/*
 ============================================================================
 Created on	:	2014. 9. 21.
 Name			:	raspberryServer.c
 Author		:	JeongHakOh
 Version		:
 Copyright		:	JeongHakOh
 Description	:	C Language based Server, Ansi-style
 URL			:	https://github.com/RasTSC/RasTS
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
#include "protocol.h"

#define BUF_SIZE 1024
#define MAX_CLIENT 5

int childcnt = 0;

void error_handling(char *message);
void ctrl_childproc(int sig);

int main(int argc, char *argv[]) {
	int server_sock, client_sock;
	char header[2] = { '$', '#' };
	struct sockaddr_in server_addr, client_addr;

	pid_t pid;
	struct sigaction act;
	socklen_t addr_size;

	if (argc != 2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}

	// signal setting
	act.sa_handler = ctrl_childproc;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	if (sigaction(SIGCHLD, &act, 0) != 0)
		error_handling("LOG ( Sigaction() error. )\n");

	// socket create
	if ((server_sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
		error_handling("LOG ( Can't open stream socket. )\n");

	// socket addr setting
	bzero((char *) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));

	// socket bind
	if (bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr))
			== -1)
		error_handling("LOG ( Bind() error )\n");

	// stay for client
	if (listen(server_sock, MAX_CLIENT) == -1)
		error_handling("LOG ( Listen() error )\n");

	while (1) {
		// client connetion
		addr_size = sizeof(client_addr);

		// try to client accept
		if ((client_sock = accept(server_sock, (struct sockaddr*) &client_addr,
				&addr_size)) == -1)
			continue;
		else
			puts("LOG ( New client connected... )");

		if ((pid = fork()) > 0)
			childcnt++;

		if (pid == -1) {
			close(client_sock);
			continue;
		}

		// child process
		if (pid == 0) {
			close(server_sock);

			int tempSec = getSec();
			int OESec = tempSec % 2;
			char buf[BUF_SIZE];
			memset(buf, 0, sizeof(buf));

			sprintf(buf, "%c%d_0123456789*\n", header[childcnt], childcnt);

			while (1) {
				char strTemp[256] = { 0, };

				int y = getYear();
				int mon = getMonth();
				int d = getDay();
				int h = getHour();
				int min = getMin();
				int s = getSec();

				if (s % 2 == OESec && tempSec != s) {

					//////////////////////////////
					//add checksum byte...
					char setData[1024] = { 0, };

					int index = 1;
					unsigned char checksum = 0;

					while (index < strlen(buf) - 1) {
						if (c_state == IDLE) {
							c_state =
									(buf[0] == '$' || buf[0] == '#') ?
											PAYLOAD : IDLE;
						} else if (c_state == PAYLOAD) {
							printf(
									"add buf[%d] = %c, checksum = (%%c)%c, (%%x)%x\n",
									index, buf[index], checksum, checksum);
							checksum ^= (buf[index++] & 0xFF);

							c_state = (buf[index] == '*') ? CHECKSTAR : PAYLOAD;
						} else if (c_state == CHECKSTAR) {

							index++;
							printf("check_star = checksum(%x)\n", checksum);

						}

					}

					printf("checksum = %x\n", checksum);
					////////////////////////////////////////////////////////////////

					sprintf(setData, "%s%c", buf, checksum);

					sprintf(strTemp,
							"%04d-%02d-%02d %02dhour %02dmin %02dsec\n", y, mon,
							d, h, min, s);
					strcat(strTemp, setData);
					tempSec = s;

					if (write(client_sock, setData, strlen(setData)) < 0)
						break;
					printf("send to client : %s\n", strTemp);

					memset(strTemp, 0, sizeof(strTemp));
					c_state = IDLE;
				}
			}

			close(client_sock);
			puts("LOG ( Client disconnected... )");
			return 0;
		} else
			close(client_sock);
	}
	close(server_sock);
	return 0;
}

void ctrl_childproc(int sig) {
	pid_t pid;
	int status;
	pid = waitpid(-1, &status, WNOHANG);
	printf("LOG ( Removed proc id: %d ) \n", pid);
	childcnt--;
}

void error_handling(char *message) {
	fputs(message, stderr);
	exit(1);
}
