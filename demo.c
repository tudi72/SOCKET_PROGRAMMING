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
pthread_mutex_t mutex;
pthread_cond_t cond;

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

void* routine_server(void* args){

    PC_info* arg = (PC_info*)args;
    pthread_mutex_lock(&mutex);
    int socket_descriptor = socket(AF_INET,SOCK_STREAM,0);
    if(socket_descriptor == -1){
        perror("couldn't create socket descriptor");
        exit(1);
    }
    struct sockaddr_in server_address;
    server_address.sin_family       = AF_INET; 
    server_address.sin_port         = htons(9002);
    server_address.sin_addr.s_addr  = inet_addr(arg->host);
    int bind_output = bind(socket_descriptor,(struct sockaddr*)&server_address,sizeof(server_address));
    if(bind_output < 0){
        perror("SERVER ERROR : cannot bind address to socket...\n");
        exit(1);
    }
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&cond);

    listen(socket_descriptor,5);
    int client_socket;
    client_socket = accept(socket_descriptor,NULL,NULL);

    int number = 0;
     while(number <= 100){ 

        if((errno = recv(client_socket,&number,sizeof(int),0)) < 0){
            fprintf(stderr,"[%s] : ERROR\t%d\n",arg->host, errno);
        }

        number++;
        if(number == 100) break;
        printf("[%s] : %d\n",arg->host,number);


        if((errno = send(client_socket,&number,sizeof(number),0)) < 0){
            fprintf(stderr,"[%s]: ERROR\t%d\n",arg->host, errno);
        }
    }
    close(socket_descriptor);
}   


void* routine_client(void* args){
    PC_info* arg = (PC_info*)args;

  // AF_INET      = IPv4                               | domain
    // SOCK_STREAM  = sequenced, two-way communication   | type specifying communication semantics
    // 0            = same protocol as socket type       | protocol
    int socket_descriptor = socket(AF_INET,SOCK_STREAM,0);
    int socket_descriptor2 = socket(AF_INET,SOCK_STREAM,0);

    if(socket_descriptor == -1 || socket_descriptor2 == -1){
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

    struct sockaddr_in server_address2;
    // sin_family   = IPV4                      | required in LInux 
    // sin_port     = port                      | port in network byte order
    // sin_addr     = inet_addr("127.0.0.1");   | IP host address
    server_address2.sin_family       = AF_INET; 
    server_address2.sin_port         = htons(9002);
    server_address2.sin_addr.s_addr  = inet_addr(arg->host2);

    while(pthread_cond_wait(&cond, &mutex) == 0){
        printf("wait...\n");
    } // waiting for the servers to be created

    int connection_status = connect(socket_descriptor,(struct sockaddr*)&server_address,sizeof(server_address));
    int connection_status2 = connect(socket_descriptor2,(struct sockaddr*)&server_address2,sizeof(server_address2));


    if(connection_status < 0 || connection_status2 < 0 ){
        perror("ERROR : binding socket to address");
        exit(1);
    }


    // receive data from the server
    int number = 1;
    while(number <= 100){

        if((errno = send(socket_descriptor2,&number,sizeof(int),0))  == -1)
        {
            fprintf(stderr,"[ERROR CLIENT]: send(socket2) : %d\n",errno);
        }

        printf("[CLIENT] :    send  to [%s] data %d\n",arg->host2,number);
        
        if((errno = recv(socket_descriptor,&number,sizeof(int),0)) == -1)
        {
           fprintf(stderr,"[ERROR CLIENT]: recv(socket) : %d\n",errno);
        }
        number++;
        if(number == 100) break;
        printf("[%s]----->[CLIENT] : %d\n",arg->host,number);

    }
    close(socket_descriptor);
    close(socket_descriptor2);
}  

PC_info** allocate_threads_args(int n,const char* host[n],const char* host2[n]){

    PC_info** PCs_info  = (PC_info**)malloc(n * sizeof(PC_info*));

    if(PCs_info == NULL){
        perror("SYSTEM : cannot allocate so many clients");
        exit(0);
    }
    for(int i = 0;i < n;i++){
        PCs_info[i] = (PC_info*)malloc(sizeof(PC_info));
        
        PCs_info[i]->host = (char*)malloc(sizeof(char) * MESSAGE_LENGTH);
        PCs_info[i]->host2 = (char*)malloc(sizeof(char) * MESSAGE_LENGTH);

        memcpy(PCs_info[i]->host,host[i],sizeof(char) * MESSAGE_LENGTH);
        memcpy(PCs_info[i]->host2,host2[i],sizeof(char) * MESSAGE_LENGTH);
        PCs_info[i]->number= 1;

    }
    
    return PCs_info;
}

int main(){

    const char* host[3] = {     "127.0.0.1",
                                "127.0.0.2",
                                "127.0.0.3"};

    const char* host2[3] = {     "127.0.0.2",
                                "127.0.0.3",
                                "127.0.0.1"};
    int n = 3;
    PC_info** info_1 = allocate_threads_args(n,host,host2);
    pthread_mutex_init(&mutex,NULL);
    pthread_cond_init(&cond,NULL);
    pthread_t* thread = (pthread_t*)malloc(sizeof(pthread_t)* (n * 2));

    for(int i = 0;i < n;i++){

        if(pthread_create(&thread[i],NULL,&routine_server,(void*)info_1[i]) != 0){
            perror("SYSTEM : cannot create thread\n");
            exit(1);
        }
     
    }

    for(int i = 0;i < n;i++){

        if(pthread_create(&thread[i+n],NULL,&routine_client,(void*)info_1[i]) != 0){
            perror("SYSTEM : cannot create thread\n");
            exit(1);
        }
    }

    for(int i = 0;i < 2*n;i++){

        if(pthread_join(thread[i],NULL) != 0){
            perror("SYSTEM : cannot join thread\n");
            exit(2);
        }


    }
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    return 0;
}
