#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 2048
#define NAME_LEN 32

// Global variables
volatile sig_atomic_t flag = 0;
int sockfd = 0;
char name[32];

void str_overwrite_stdout()
{
    printf("%s", "> ");
    fflush(stdout);
}

void string_trim(char *msg, int length)
{
    int i;
    for (i = 0; i < length; i++)
    {
        if (msg[i] == '\n')
        {
            msg[i] = '\0';
            break;
        }
    }
}

void catch_ctrl_c_and_exit()
{
    flag = 1;
}

void send_msg_handler()
{
    char buffer[BUFFER_SIZE] = {};
    char message[BUFFER_SIZE + NAME_LEN] = {};

    while (1)
    {
        str_overwrite_stdout();
        fgets(message, BUFFER_SIZE, stdin);
        string_trim(message, BUFFER_SIZE);

        if (strcmp(message, "exit") == 0)
        {
            break;
        }
        else
        {
            sprintf(buffer, "%s: %s\n", name, message);
            send(sockfd, buffer, strlen(buffer), 0);
        }

        bzero(buffer, BUFFER_SIZE);
        bzero(message, BUFFER_SIZE + NAME_LEN);
    }
    catch_ctrl_c_and_exit();
}

void recv_msg_handler()
{
    char message[BUFFER_SIZE] = {};

    while (1)
    {
        int receive = recv(sockfd, message, BUFFER_SIZE, 0);
        if (receive > 0)
        {
            printf("%s", message);
            str_overwrite_stdout();
        }
        else if (receive == 0)
        {
            break;
        }
        bzero(message, BUFFER_SIZE);
    }
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *ip = "127.0.0.1";
    int port = atoi(argv[1]);

    signal(SIGINT, catch_ctrl_c_and_exit);

    printf("Enter your name: ");
    fgets(name, 32, stdin);
    string_trim(name, strlen(name));

    if (strlen(name) > 32 || strlen(name) < 2)
    {
        printf("Name must be less than 30 and more than 2 characters.\n");
        return EXIT_FAILURE;
    }

    struct sockaddr_in server_address;

    /* Socket settings */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(ip);
    server_address.sin_port = htons(port);

    // Connect to Server
    int err = connect(sockfd, (struct sockaddr *)&server_address, sizeof(server_address));
    if (err == -1)
    {
        printf("ERROR: connect\n");
        return EXIT_FAILURE;
    }

    // Send name
    send(sockfd, name, 32, 0);

    printf("~~~~~ Welcome to the Chat Room ~~~~~\n");

    pthread_t send_msg_thread;
    if (pthread_create(&send_msg_thread, NULL, (void *)send_msg_handler, NULL) != 0)
    {
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    pthread_t recv_msg_thread;
    if (pthread_create(&recv_msg_thread, NULL, (void *)recv_msg_handler, NULL) != 0)
    {
        printf("ERROR: pthread\n");
        return EXIT_FAILURE;
    }

    while (1)
    {
        if (flag)
        {
            printf("\nSalutations %s...\n", name);
            break;
        }
    }

    close(sockfd);

    return EXIT_SUCCESS;
}