#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_PORT 9002
#define BUFSIZE 4096
#define SOCKETERROR (-1)
#define SERVER_BACKLOG 1

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void handle_connection(int client_socket);
struct sockaddr_in init_socket(const char* address);
void check(int output,const char* msg);

int main(int argc,char** argv){

	int		number = 0,connection_status;
	int		client_socket;
	SA_IN	client_address;
	

	check((client_socket = socket(AF_INET,SOCK_STREAM,0)),"[SERVER] : cannot create socket");
	client_address = init_socket("127.0.0.1");
	check((connection_status = connect(client_socket,(struct sockaddr*)&client_address,sizeof(client_address))),"[CLIENT]--(connect)--->[SERVER] ");
	printf("[CLIENT] : connection status = %d\n",connection_status);
     while(1){ 
        if(sendto(client_socket,&number,sizeof(number),0,(struct sockaddr*)&client_address,sizeof(client_address)),"[CLIENT]-------->[SERVER] " > 0){

			number++;
			if(number == 100) break;
			printf("[CLIENT] : %d\n",number);

			check(recv(client_socket,&number,sizeof(int),0),"[CLIENT]-------->[SERVER] ");
		}
    }
    close(client_socket);
    return 0;
}

struct sockaddr_in init_socket(const char* address){
    struct sockaddr_in server_address;
    server_address.sin_family       = AF_INET; 
    server_address.sin_port         = htons(SERVER_PORT);
    server_address.sin_addr.s_addr  = INADDR_ANY;
    // server_address.sin_addr.s_addr  = inet_addr(address);
    return server_address;
}

void check(int output,const char* msg){
    if(output < 0){
		perror(msg);
        exit(1);
    }
}