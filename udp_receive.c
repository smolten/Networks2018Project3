#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>

int main(int argc, char *argv[]) {

	//Set mode
	int isClient = 1;
	if (argc < 2)
	{
		printf("Running as Client by default.\nYou can specify a mode with \"server\" or \"client\" argument (No quotes)\n");
	} else if (argc == 2) {
		int serverCompare = strcasecmp(argv[1], "server");
		int clientCompare = strcasecmp(argv[1], "client");
		if (serverCompare == 0) {
			isClient = 0;
		} else if (clientCompare != 0) {
			printf ("ERROR: Invalid argument \"%s\"\nYou can specify a mode with \"server\" or \"client\" argument (No quotes)\nExiting...\n", argv[1]);
			return -1;
		}

		char* mode = isClient == 1 ? "Client" : "Server";
		printf("Running as %s...\n", mode);
	} else {
		printf("ERROR: Too many arguments. Exiting...\n");
		return -1;
	}

    //Claim listen socket
    int socket_listen = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_listen < 0)
    {
        printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }

    //Claim outbound socket
    /*int socket_send = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_send < 0)
    {
        printf("socket() error: %s.\n", strerror(errno));
        return -1;
    }*/

    //make listen address (any)
    struct sockaddr_in listen_address;
    memset(&listen_address, 0, sizeof(listen_address));
    listen_address.sin_family = AF_INET;
    listen_address.sin_addr.s_addr = htonl(INADDR_ANY);
    listen_address.sin_port = htons(32000);

    //make server address
	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_address.sin_port = htons(32000);

    //Attempt to bind to address
    if (isClient == 0) {
        int retBind = bind(socket_listen, (struct sockaddr *) &listen_address, sizeof(listen_address));
        if (retBind < 0) {
            printf("bind() error: %s.\n", strerror(errno));
            return -1;
        } else {
            printf("Successfully bound to socket...\n");
        }
    }

    //Run the server
    char message_buffer[1024];
    struct sockaddr_in client_address;
    while (1) {

    	//Client sends message first
    	if (isClient) {
    		fgets(message_buffer, sizeof(message_buffer), stdin);

			int ret = sendto(socket_listen, 
				message_buffer, 
				sizeof(message_buffer), 
				0,
				(struct sockaddr *) &server_address,
				sizeof(server_address));

			if (ret <= 0)
			{
				printf("recvfrom() error: %s.\n", strerror(errno));
				return -1;
			}
    	}

    	//Listen for message from client
        socklen_t len = sizeof(client_address);
        int retRecv = recvfrom(socket_listen, 
            message_buffer, 
            sizeof(message_buffer), 
            0,
            (struct sockaddr *) &client_address,
            &len);

        //Error message
        if (retRecv <= 0) {
            printf("recvfrom() error: %s.\n", strerror(errno));
            return -1;
        }

        //Print message
        char* srcAddress = inet_ntoa(client_address.sin_addr);
        printf("[from %s] %s\n", srcAddress, message_buffer);

        /*
        //Respond
        int retSnd = sendto(socket_listen, 
			message_buffer, 
			sizeof(message_buffer), 
			0,
			(struct sockaddr *) &server_address,
			sizeof(server_address));

		if (retSnd <= 0)
		{
			printf("recvfrom() error: %s.\n", strerror(errno));
			return -1;
		}*/
    }

    return 0;
}

