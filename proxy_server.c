#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <dirent.h>
#include <pthread.h>
#include <stdbool.h>

void handle_request(int fd)
{
	printf("Server loading to connect to remote server...\n");
	struct sockaddr_in remote_server_structure;
	struct hostent* host;
	int port = 80;
	char ip[20];
	int remote_socket_fd;
	char input_buffer[510];
	char method[300];
	char url[300];
	char final_link[300];
	char* temp=NULL;
	char version[300];
	char* sample = NULL;
	int i=0;
	int n=0;
	bool port_present = false;
	recv(fd,input_buffer,500,0);
	sscanf(input_buffer,"%s %s %s",method,url,version);
	if((strncmp(method,"GET",3) == 0) && ((strncmp(version,"HTTP/1.0",8) == 0) || (strncmp(version,"HTTP/1.1",8) == 0)) && (strncmp(url,"http://",7) == 0))
	{
		
		
		
		strcpy(method,url);
		temp = strtok(url,"//");
		temp = strtok(NULL,"/");
		printf("\nURL: %s\n",temp);
		sample = strtok(NULL,"\0");
		host=gethostbyname(temp);
		strcpy(final_link,temp);
		
		int fd_file = open(temp,O_RDONLY);		
		if(fd_file < 0 || sample != NULL){
		printf("NO CACHE AVAILABLE\n");
		FILE* fp = fopen(temp,"w");
		fclose(fp);
		fd_file = open(temp,O_WRONLY);
		strcat(method,"^]");
		temp=strtok(method,"//");
		
		temp=strtok(NULL,"/");
		if(temp!=NULL)
			temp=strtok(NULL,"^]");
		
		printf("path = %s\nPort = %d\n",temp,port);


		bzero((char*)&remote_server_structure,sizeof(remote_server_structure));
		remote_server_structure.sin_port = htons((port));
		remote_server_structure.sin_family = AF_INET;
		bcopy((char*)host->h_addr,(char*)&remote_server_structure.sin_addr.s_addr,host->h_length);

		remote_socket_fd = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
		if((connect(remote_socket_fd,(struct sockaddr*)&remote_server_structure,sizeof(struct sockaddr))) < 0)
		{
			printf("Error in connecting to remote server\n");
			return;
		}
		printf("\nConnected to %s  IP - %s\n",url,inet_ntoa(remote_server_structure.sin_addr));
		printf("Connected to remote server\n");
		bzero((char*)input_buffer,sizeof(input_buffer));
		if(temp!=NULL)
			sprintf(input_buffer,"GET /%s %s\r\nHost: %s\r\nConnection:alive\r\n\r\n",temp,version,final_link);
		else
			sprintf(input_buffer,"GET / %s\r\nHost: %s\r\nConnection: alive\r\n\r\n",version,final_link);
		write(remote_socket_fd,input_buffer,strlen(input_buffer));
		printf("%s\n",input_buffer);
		do
		{
			bzero((char*)input_buffer,500);
			n=recv(remote_socket_fd,input_buffer,500,0);
			if(!(n<=0)){
				send(fd,input_buffer,n,0);
				write(fd_file,input_buffer,n);
			}
		}while(n>0);
		close(fd_file);
		}
		else
		{
			printf("CACHE MEMORY FOUND\n");
			do
			{
			bzero((char*)input_buffer,500);
			n=read(fd_file,input_buffer,500);
			if(!(n<=0)){
				send(fd,input_buffer,n,0);
			}
			}while(n>0);
			printf("DATA SENT FROM CACHE\n");
			close(fd_file);
		}
		
	}
	else
	{
		write(fd,"400 : BAD REQUEST\n",19);
	}
	
	
}

int main(int argc,char* argv[])
{
	struct sockaddr_in structure_server,structure_client;
	int server_fd = 0;
	if((server_fd = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		printf("Couldnt create a server\n");
		exit(0);
	}
	

	structure_server.sin_family = AF_INET;
	structure_server.sin_port = htons(atoi(argv[1]));
	structure_server.sin_addr.s_addr = inet_addr("127.0.0.1");
	socklen_t server_length = sizeof(structure_server);
	socklen_t client_length = sizeof(structure_client);
	if(bind(server_fd,(struct sockaddr*)&structure_server,server_length) < 0)
	{
		printf("Binding error\n");
		exit(0);
	}
	listen(server_fd,10);
	while(1)
	{
	    printf("Server in waiting State\n");
		int client_fd  = accept(server_fd,(struct sockaddr*)&structure_client,&client_length);
		if(fork() == 0)
		{
			printf("Connected to client with fd: %d\n",client_fd);
			close(server_fd);
			handle_request(client_fd);
			close(client_fd);
			exit(0);
		}
		//close(client_fd);
		
	}
	
}

