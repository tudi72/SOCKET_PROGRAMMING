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

#define MESSAGE_LENGTH 256
pthread_t PC_1,PC_2,PC_3;
sem_t semaphore;

typedef struct{
    int number;
    char* host;
}PC_info;

void error(int output,const char* msg){
    if(output < 0){
        perror(msg);
        exit(0);
    }
}

struct sockaddr_in init_socket(const char* address){
    struct sockaddr_in socket_address;
    socket_address.sin_family       = AF_INET; 
    socket_address.sin_port         = htons(9002);
    socket_address.sin_addr.s_addr  = inet_addr(address); 
    return socket_address;
}

void* routine_server(void* args){

    PC_info* arg = (PC_info*)args;
    // AF_INET      = IPv4                               | domain
    // SOCK_STREAM  = sequenced, two-way communication   | type specifying communication semantics
    // 0            = same protocol as socket type       | protocol
    int socket_descriptor = socket(AF_INET,SOCK_STREAM,0);

    if(socket_descriptor == -1){
        perror("couldn't create socket descriptor");
        exit(1);
    }

    // initialize server address IPV4 using sockaddr_in
    struct sockaddr_in server_address;

    // sin_family   = IPV4                      | required in LInux 
    // sin_port     = port                      | port in network byte order
    // sin_addr     = inet_addr("127.0.0.1");   | IP host address
    server_address.sin_family       = AF_INET; 
    server_address.sin_port         = htons(9002);
    server_address.sin_addr.s_addr  = inet_addr(arg->host);


    // binding the socket descriptor to the server address 
    // int bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen) 
    int bind_output = bind(socket_descriptor,(struct sockaddr*)&server_address,sizeof(server_address));
    if(bind_output < 0){
        perror("SERVER ERROR : cannot bind address to socket...\n");
        exit(1);
    }

    listen(socket_descriptor,5);
    printf("[%s] : accept client...\n",arg->host);
    int client_socket;
    client_socket = accept(socket_descriptor,NULL,NULL);

    int read_size;
    int client_message;
    while(1){
        while((read_size = recv(client_socket,&client_message,sizeof(int), 0)) > 0){
            
            printf("[%s] : server received %d\n",arg->host,client_message);
            client_message = client_message + 1;
            printf("[%s] : server sends %d\n",arg->host,client_message);
            send(socket_descriptor,&client_message,sizeof(int),0);
        }
    }

    close(socket_descriptor);
}   


PC_info** allocate_threads_args(int n,const char* host[n]){

    PC_info** PCs_info  = (PC_info**)malloc(n * sizeof(PC_info*));

    if(PCs_info == NULL){
        perror("SYSTEM : cannot allocate so many clients");
        exit(0);
    }
    for(int i = 0;i < n;i++){
        PCs_info[i] = (PC_info*)malloc(sizeof(PC_info));
        
        PCs_info[i]->host = (char*)malloc(sizeof(char) * MESSAGE_LENGTH);

        memcpy(PCs_info[i]->host,host[i],sizeof(char) * MESSAGE_LENGTH);
        PCs_info[i]->number= 1;

    }
    
    return PCs_info;
}

int main(){

    const char* host[3] = {     "127.0.0.1",
                                "127.0.0.2",
                                "127.0.0.3"};
    int n = 3;
    PC_info** info_1 = allocate_threads_args(n,host);

    pthread_t* thread = (pthread_t*)malloc(sizeof(pthread_t)* n);

    for(int i = 0;i < n;i++){
        if(pthread_create(&thread[i],NULL,&routine_server,(void*)info_1[i]) != 0){
            perror("SYSTEM : cannot create thread\n");
            exit(1);
        }
    }

    for(int i = 0;i < n;i++){

        if(pthread_join(thread[i],NULL) != 0){
            perror("SYSTEM : cannot join thread\n");
            exit(2);
        }


    }

    return 0;
}
