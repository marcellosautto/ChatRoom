compile:
	gcc -Wall -g3 -fsanitize=address -pthread chatroom_server.c -o chatroom_server
	gcc -Wall -g3 -fsanitize=address -pthread chatroom_client.c -o chatroom_client