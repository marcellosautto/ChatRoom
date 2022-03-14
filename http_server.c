#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h> // for close

#define PORT 8080

int main(int argc, char const *argv[]){

    //open and read data from html file
    FILE *html_data;
    html_data = fopen("index.html", "r");

    char response_data[1024];
    fgets(response_data, 1024, html_data);

    char http_header[2048] = "HTTP/1.1 200 OK\r\n\n";
    strcat(http_header, response_data);

    // create a socket
    int server_socket;
    server_socket = socket(AF_INET, SOCK_STREAM, 0);

    //error check for socket creation
    if(server_socket == -1){
        printf("failed to create socket...\n");
        exit(0);
    }

    //define address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    //bind socket
    if((bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address))) == -1){
        printf("socket bind failed...\n");
        exit(0);
    }
    else{
         printf("socket binded successfully...");
    }
       

    listen(server_socket, 5);

    int client_socket;

    while(1){
        client_socket = accept(server_socket, NULL, NULL);
        if(client_socket < 0){
            printf("server accept failed");
            exit(0);
        }
        send(client_socket, http_header, sizeof(http_header), 0);
        close(client_socket);
    }

    return 0;

}