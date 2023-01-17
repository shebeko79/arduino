#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 65536

#define URL_HEADER "/usr/bin/curl \"https://api.telegram.org/"
#include "telegram_bot.h"

void error(char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char **argv)
{
	int sockfd;
	int portno;
	int clientlen;
	struct sockaddr_in serveraddr;
	struct sockaddr_in clientaddr;
	struct hostent *hostp;
	char input_buf[sizeof(URL_HEADER) + BUFSIZE + 2];
	char output_cmd[BUFSIZE];
	int output_pos;
	char *hostaddrp;
	int optval;
	int n;
	FILE *shell_pc;
	
	char* input_cmd = input_buf + sizeof(URL_HEADER) - 1;
	
	strcpy(input_buf,URL_HEADER);

	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}
	portno = atoi(argv[1]);

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");

	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval, sizeof(int));

	bzero((char *)&serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)portno);

	if (bind(sockfd, (struct sockaddr *)&serveraddr,
		 sizeof(serveraddr)) < 0)
		error("ERROR on binding");

	clientlen = sizeof(clientaddr);
	while (1)
	{
		n = recvfrom(sockfd, input_cmd, BUFSIZE, 0,(struct sockaddr *)&clientaddr, &clientlen);
		if (n < 0)
			error("ERROR in recvfrom");
		
		input_cmd[n]='"';
		input_cmd[n+1]=0;
		printf("n=%d buf=\"%s\"\n",n,input_buf);
		
		if(strstr(input_cmd,botStr) == 0)
			continue;

		char input_cmd[BUFSIZE];
		
		shell_pc = popen(input_buf, "r");
		
		if (shell_pc  == NULL)
		{
			error("Error opening pipe!\n");
			continue;
		}
		
		output_pos = 0;
		while(1)
		{
			size_t ct = fread(output_cmd + output_pos, 1, BUFSIZE - output_pos, shell_pc);
			if(ct<=0)
				break;
			
			output_pos +=ct;
		}
		
		pclose(shell_pc);

		n = sendto(sockfd, output_cmd, output_pos, 0,(struct sockaddr *)&clientaddr, clientlen);
		if (n < 0)
			error("ERROR in sendto");
    }
}