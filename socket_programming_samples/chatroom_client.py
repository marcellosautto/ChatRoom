###### IN PROGRESS ######

from concurrent.futures import thread
from sre_constants import FAILURE, SUCCESS
import threading
from signal import SIGINT, signal
import socket
import struct

CLIENT_CAPACITY = 100
BUFFER_SIZE = 2048
NAME_LEN = 32

flag = 0
sockfd = 0
name = ""

class Thread(threading.Thread):
    def run(self):
        pass

def catch_ctr_c_and_exit():
    flag = 1

def string_overwrite():
    print("\r%s", "> ")
    fflush(stdout)


def string_trim(arr, length):

    for i in range(length):
        if(arr[i] == '\n'):
            arr[i] = '\0'
            break

def send_message_handler():
    message = ""
    buffer = ""

    while 1:
        string_overwrite()
    
        string_trim(message, BUFFER_SIZE)
        recieve = socket.recv_fds(sockfd, message, BUFFER_SIZE, 0)

        if "exit" in buffer:
            pass

def recv_message_handler():
    message = ""

    while 1:
        recieve = socket.recv_fds(sockfd, message, BUFFER_SIZE, 0)

        if recieve > 0:
            print(message)
            string_overwrite()
        elif recieve == 0:
            break

        message = ""
        
def main(argc, argv):
    if argc != 2:
        print("Usage: %s <port>\n", argv[0])
        return -1

    IP = "127.0.0.1"
    PORT = argv[0]
    ADDRESS = (IP, PORT)
    SIZE = 1024
    FORMAT = "utf-8"
    
    name = input("Enter your name: ")

    signal(SIGINT, catch_ctr_c_and_exit)
    string_trim(name, len(name))

    if len(name) > NAME_LEN - 1 or len(name) < 2:
        print(f"ERROR: Invalid Name. Must be between 2-{NAME_LEN} characters \n")

    err = socket.connect(ADDRESS)

    if err == -1:
        print("ERROR: connect\n")
        return -1

    # send the name
    socket.send_fds(sockfd, name, NAME_LEN, 0)

    print("~~~~~Welcome to the Chat Room!~~~~~\n")

    send_msg_thread = Thread(name="Thread-Send-{}".format(name))

    if not send_msg_thread.is_alive:
        print("ERROR: send_msg_thread failed to run\n")
        return FAILURE

    recv_msg_thread = Thread(name="Thread-Recieve-{}".format)

    if not recv_msg_thread.is_alive:
        print("ERROR: recv_msg_thread failed to run\n")
        return FAILURE
    
    while 1:
        if flag:
            print("\nGoodbye\n")
            break

        socket.close(sockfd)

        return SUCCESS

if __name__ == "__main__":
    main()