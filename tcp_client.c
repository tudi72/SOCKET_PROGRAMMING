#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h> // for close

#define MAX 80
#define PORT 8080
#define SA struct sockaddr

void create_client_socket(){

    // AF_INET      = IPv4                               | domain
    // SOCK_STREAM  = sequenced, two-way communication   | type specifying communication semantics
    // 0            = same protocol as socket type       | protocol
    int socket_descriptor = socket(AF_INET,SOCK_STREAM,0);

    if(socket_descriptor == -1 ){
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
    server_address.sin_addr.s_addr  = inet_addr("127.0.0.1");

    int connection_status = connect(socket_descriptor,(struct sockaddr*)&server_address,sizeof(server_address));


    if(connection_status < 0 ){
        perror("ERROR : binding socket to address");
        exit(1);
    }


    // receive data from the server
    int number = 1;
    while(1){
        printf("[CLIENT ] : %d\n",number);
        
        if(send(socket_descriptor,&number,sizeof(int),0) < 0){
            perror("ERROR CLIENT : send message failed...\n");
            exit(1);
        }

        if(recv(socket_descriptor,&number,sizeof(int),0) < 0){
            puts("ERROR CLIENT : didn't receive message from server...\n");
            break;
        }
        number++;

        if(number == 100){
            puts("stop joc");
            break;
        }
    }
    puts("here");
    close(socket_descriptor);
}   


int main(){
    //  CLIENT SOCKET WORKFLOW
    // 1.create socket
    // 2. connect to server
    // 3. retrieve data 
    create_client_socket();
    return 0;
}