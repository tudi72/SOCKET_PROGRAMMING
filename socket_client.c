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

void* routine_client(void* args){
    PC_info* arg = (PC_info*)args;

    int socket_descriptor = socket(AF_INET,SOCK_STREAM,0);

    if(socket_descriptor == -1){
        perror("couldn't create socket descriptor");
        exit(1);
    }

    struct sockaddr_in server_address;

    server_address.sin_family       = AF_INET; 
    server_address.sin_port         = htons(9002);
    server_address.sin_addr.s_addr  = inet_addr(arg->host);

    int connection_status = connect(socket_descriptor,(struct sockaddr*)&server_address,sizeof(server_address));

    if(connection_status < 0){
        perror("ERROR : binding socket to address");
        exit(1);
    }
    int server_response;
    int client_message = 1;
    while(1){
        if(send(socket_descriptor,&client_message,sizeof(int),0) < 0){
            perror("ERROR CLIENT : send message failed...\n");
            exit(1);
        }
        if(recv(socket_descriptor,&server_response, sizeof(int),0) < 0){
            printf("[%s] CLIENT : wait reponse...\n",arg->host);
        }
        else printf("[%s] CLIENT : received %d\n",arg->host,server_response);

        client_message = server_response + 1;
        printf("[%s] CLIENT : sends %d\n",arg->host,client_message);

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
        if(pthread_create(&thread[i],NULL,&routine_client,(void*)info_1[i]) != 0){
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
