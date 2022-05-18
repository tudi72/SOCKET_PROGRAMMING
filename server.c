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
#define SERVER_BACKLOG 5

typedef struct sockaddr_in SA_IN;
typedef struct sockaddr SA;

void handle_connection(int client_socket);
struct sockaddr_in init_socket();
void check(int output,const char* msg);

int main(){

	int		number;
	int		server_socket	, client_socket;
	SA_IN	server_address	,client_address;
	
	check((server_socket = socket(AF_INET,SOCK_STREAM,0)),"[SERVER] : cannot create socket");
	server_address = init_socket();
    check(bind(server_socket,(SA*)&server_address,sizeof(server_address)),"[SERVER] : binding error");
	check((client_socket = listen(server_socket,SERVER_BACKLOG)),"[SERVER] : listen failed");
    printf("client socket = %d\n",client_socket);
    while(recv(client_socket,&number,sizeof(int),0) > 0){


            number++;
            if(number == 100) break;
            printf("[SERVER] : %d\n",number);

            check(send(client_socket,&number,sizeof(number),0),"[SERVER]-------->[CLIENT]: ERROR");
    }
    close(server_socket);
    return 0;
}

struct sockaddr_in init_socket(){
    struct sockaddr_in server_address;
    server_address.sin_family       = AF_INET; 
    server_address.sin_port         = htons(SERVER_PORT);
    server_address.sin_addr.s_addr  = INADDR_ANY;
    return server_address;
}

void check(int output,const char* msg){
    
    if(output < 0){
        fprintf(stderr,"%s\n",msg);
        exit(1);
    }
}