#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 8080


int main(int argc, char const *argv[]){
    
    //create socket
    int net_socket;
    net_socket = socket(AF_INET, SOCK_STREAM, 0);

    //specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    server_address.sin_addr.s_addr = INADDR_ANY; //connect to address on local machine (0.0.0.0)

    int connection_flag = connect(net_socket, (struct sockaddr*) &server_address, sizeof(server_address));

    //check if connection was successful
    if(connection_flag == -1){
        printf("Error making connection to remote socket\n");

    }

    //recieve data from server
    char server_response_buffer[256];
    recv(net_socket, &server_response_buffer, sizeof(server_response_buffer), 0);

    //print data sent from server
    printf("Data recieved from server: %s\n", server_response_buffer);

    //close socket connection
    close(net_socket);
    
    return 0;
}
