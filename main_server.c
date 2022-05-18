//Example code: A simple server side code, which echos back the received message. 
//Handle multiple socket connections with select and fd_set on Linux  
#include <stdio.h>  
#include <string.h>   //strlen  
#include <stdlib.h>  
#include <errno.h>  
#include <unistd.h>   //close  
#include <arpa/inet.h>    //close  
#include <sys/types.h>  
#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  
#include <time.h>
#define TRUE   1  
#define FALSE  0  
#define PORT 9002  
     

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

int* bublesort(int n,int arr[n]){
    for(int i = 1;i < n-1;i++){
        for(int j = i;j < n;j++){
            if(arr[i] < arr[j]){
                int ax = arr[i];
                arr[i] = arr[j];
                arr[j] = ax; 
            }
        }
    }
    return arr;
}

int main(int argc , char *argv[])   
{   
    int opt = TRUE;   
    int master_socket   , addrlen , new_socket  ,  
        max_clients = 30, activity,     i       , valread , sd;   
    int max_sd;
    int* client_port = (int*)calloc(sizeof(int),30); 
    int* client_socket = (int*)calloc(sizeof(int),30);
    int max_port = 0;   
    struct sockaddr_in address;   
    char buffer[1025];          // data buffer of 1K  
    int number = 0;             // number to be sent to clients
    fd_set readfds;             // set of socket descriptors   
    fd_set writefds;            // set of sockets for writting

    for (i = 0; i < max_clients; i++)  client_socket[i] = 0;   
         
    //create a master socket  
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)   
    {   
        perror("socket failed");   
        exit(EXIT_FAILURE);   
    }   

    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   
     
    address.sin_family = AF_INET;   
    address.sin_addr.s_addr = INADDR_ANY;   
    address.sin_port = htons( PORT );   
         
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)   
    {   
        perror("bind failed");   
        exit(EXIT_FAILURE);   
    }   
    printf("Listener on port %d \n", PORT);   
         
    if (listen(master_socket, 3) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
         
    //accept the incoming connection  
    addrlen = sizeof(address);   
    puts("Waiting for connections ...");   
         
    while(1)   
    {   
        //setting up the client sockets for being accept then reading/writing 
        {
            //clear the socket set  
            FD_ZERO(&readfds);
        
            //add master socket to set  
            FD_SET(master_socket, &readfds);
            max_sd = master_socket;   
                
            //add child sockets to set  
            for ( i = 0 ; i < max_clients ; i++)   
            {   
                //socket descriptor  
                sd = client_socket[i];   
                    
                //if valid socket descriptor then add to read list  
                if(sd > 0) {
                    FD_SET( sd , &readfds);
                }   
                    
                //highest file descriptor number, need it for the select function  
                if(sd > max_sd)  max_sd = sd;   
            }   
        
            //wait for an activity on one of the sockets , timeout is NULL ,  
            //so wait indefinitely  
            activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);   
        
            if ((activity < 0) && (errno!=EINTR))   
            {   
                printf("select error");   
            }   
        }
        
        //If something happened on the master socket ,  
        //then its an incoming connection  
        if (FD_ISSET(master_socket, &readfds))   
        {   
            if ((new_socket = accept(master_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)   
            {   
                perror("accept");   
                exit(EXIT_FAILURE);   
            }   
            printf("New connection , socket fd is %d , ip is : %s , port : %d  \n" , new_socket , inet_ntoa(address.sin_addr), ntohs(address.sin_port));   

            //add new socket to array of sockets  
            for (i = 0; i < max_clients; i++)   
            {   
                //if position is empty  
                if( client_socket[i] == 0 )   
                {   
                    client_socket[i] = new_socket;   
                    client_port[i] = ntohs(address.sin_port);
                    client_port = bublesort(max_clients,client_port);
                    client_socket = bublesort(max_clients,client_socket);
                    if(max_port < client_port[i]) max_port = client_port[i];

                    //send new connection the number 
                    if(i == 0) {
                        send(new_socket, &number,sizeof(number), 0);   
                    }
                    printf("Adding to list of sockets as %d\n" , i);   
                    break;   
                }   
            }
        }   
             
        //else its some IO operation on some other socket 
        for (i = 0; i < max_clients-1; i++)   
        {   
            sd = client_socket[i];   

            if (FD_ISSET( sd , &readfds))   
            {   
                if ((valread = read( sd , &number, sizeof(number))) == 0)   
                {   
                    getpeername(sd , (struct sockaddr*)&address, (socklen_t*)&addrlen);   
                    printf("Host disconnected , ip %s , port %d \n" ,inet_ntoa(address.sin_addr) , ntohs(address.sin_port));   
                         
                    close( sd );   
                    client_socket[i] = 0;   
                }   
                else 
                { 
                    //right now two ports are exchanging values
                    if(client_port[i+1] != 0)
                        send(client_socket[(i+1)],&number,sizeof(number),0);
                    else 
                        send(client_socket[(0)],&number,sizeof(number),0);
                }   
            }

        } 


    }   
         
    return 0;   
}   
