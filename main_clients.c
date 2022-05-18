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
#include <time.h>

#define MESSAGE_LENGTH 256

pthread_t PC_1,PC_2,PC_3;
sem_t semaphore;

typedef struct{
    int number;
    char* host;
    char* host2;
}PC_info;

void error(int output,const char* msg){
    if(output < 0){
        perror(msg);
        exit(0);
    }
}

void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;
  
    // Storing start time
    clock_t start_time = clock();
  
    // looping till required time is not achieved
    while (clock() < start_time + milli_seconds)
        ;
}

struct sockaddr_in init_socket(const char* address){
    struct sockaddr_in socket_address;
    socket_address.sin_family       = AF_INET; 
    socket_address.sin_port         = htons(9002);
    socket_address.sin_addr.s_addr  = inet_addr(address); 
    return socket_address;
}

void* routine_client_main(void* args){
    const char* arg = (const char*)args;

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
    server_address.sin_addr.s_addr  = inet_addr(arg);


    int connection_status = connect(socket_descriptor,(struct sockaddr*)&server_address,sizeof(server_address));


    if(connection_status < 0 ){
        perror("ERROR : binding socket to address");
        exit(1);
    }


    // receive data from the server
    int number;
    while(number <= 100){
        

        
        if((errno = recv(socket_descriptor,&number,sizeof(int),0)) < 1)
        {
           fprintf(stderr,"[SERVER]------->[%s]: recv(socket) : %d\n",arg,errno);
        }
        number++;
        if(number == 100) break;
        printf("[SERVER]----->[%s] : %d\n",arg,number);

        if((errno = send(socket_descriptor,&number,sizeof(int),0))  < 1)
        {
            fprintf(stderr,"[ERROR CLIENT]: send(socket2) : %d\n",errno);
        }

    }
    close(socket_descriptor);
}  

int main(){

    const char* host[3] = {     "127.0.0.1",
                                "127.0.0.2",
                                "127.0.0.3"};
    int n = 3;
        
    pthread_t* thread = (pthread_t*)malloc(sizeof(pthread_t)* n);

    for(int i = 0;i < n;i++){
        
        if(pthread_create(&thread[i],NULL,&routine_client_main,(void*)host[i]) != 0){
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
