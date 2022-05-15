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

void create_server_socket(){

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
    server_address.sin_addr.s_addr  = INADDR_ANY;


    // binding the socket descriptor to the server address 
    // int bind(int sockfd, const struct sockaddr *addr,socklen_t addrlen) 
    int bind_output = bind(socket_descriptor,(struct sockaddr*)&server_address,sizeof(server_address));
    if(bind_output < 0){
        perror("SERVER ERROR : cannot bind address to socket...\n");
        exit(1);
    }

    // listen to clients
    listen(socket_descriptor,5);


    int client_socket;
    // accept - extracts the first connection request from the queue of pending connections 
    client_socket = accept(socket_descriptor,NULL,NULL);

    puts("Server : Connection accepted...\n");

    //send data to client
    char server_message[256] = "Server sends this message ...";
    char client_message[256];
    int read_size;

    // continue to send data as long as we receive a message from client    
    while((read_size = recv(client_socket,client_message,2000, 0)) > 0){
        
        printf("SERVER : received client message %s\n",client_message);

        // send message to client
        write(socket_descriptor,server_message,sizeof(server_message));
    }

    if(read_size == 0){
        perror("ERROR SERVER: client disconnected...\n");
        fflush(stdout);
    }
    else if(read_size == -1){
        perror("ERROR SERVER: receive message from client failed...\n");
    }

    close(socket_descriptor);

}   


int main(){
    //  SERVER SOCKET WORKFLOW
    // 1.create socket
    // 2. bind socket to one and only one IP address
    // 3. listen to different clients(ip addresses)
    // 4. accept a connection and then send / recv data
    create_server_socket();
    return 0;
}