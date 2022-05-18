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

const char* get_socket_ip(int socket);

void create_server_socket(){

    // AF_INET      = IPv4                               | domain
    // SOCK_STREAM  = sequenced, two-way communication   | type specifying communication semantics
    // 0            = same protocol as socket type       | protocol
    int socket_descriptor = socket(AF_INET,SOCK_STREAM,0);

    if(socket_descriptor == -1){
        perror("couldn't create socket descriptor");
        exit(1);
    }

    if (setsockopt(socket_descriptor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0){
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
    const char* client_IP = get_socket_ip(client_socket);
    printf("[CLIENT ADDRESS]: %s\n",client_IP);

    puts("Server : Connection accepted...\n");

    int read_size;
    int number;
    // continue to send data as long as we receive a message from client
    while(1){    
        if(recv(client_socket,&number,sizeof(int),0) < 0){
            puts("ERROR SERVER : didn't receive message from client...\n");
            break;
        }
        printf("SERVER : received message \"%d\"\n",number);
        number++;
        if(number == 100) break;
        if(send(client_socket,&number,sizeof(number),0) < 0){
            puts("ERROR SERVER: cannot send message");
            exit(1);
        }
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

const char* get_socket_ip(int socket){
    
    struct sockaddr_in  addr;
    // Get my ip address and port
    bzero(&addr, sizeof(addr));
    socklen_t len = sizeof(addr);
    getsockname(socket, (struct sockaddr *) &addr, &len);
    return inet_ntoa(addr.sin_addr);
}
