#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>
#include <pthread.h>

char* stringToLower(char* str) {
	for (int i = 0; str[i]; i++) {
		str[i] = tolower(str[i]);
	} // str[i] returns true or false depending on whether index i exists
	return str;
}

void* handle_client(int fd) {
	printf("client handle reached");
	// Recieving network stream from incoming socket connection
	char* network_stream = malloc(1025);
	int* ret = (int*)recv(fd, network_stream, 1024, 0);
	// if (ret < 0) {
	// 	printf("Error recieving socket stream: %s \n", strerror(errno));
	// 	return NULL;
	// }

	// Check url
	char* network_stream_method = strdup(network_stream);
	char* method = strtok(network_stream_method, " ");
	char* request_target = strtok(NULL, " ");
	char response[1024];

	
	//// These are literally routes being created
	if (strlen(request_target) > 6 && strncmp(request_target, "/echo/", 6) == 0) {
		// Get the request body
		char* body = request_target + 6;
        int content_length = strlen(body);
		// Dynamically a request
		snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", content_length ,body);
	}
    else if (strlen(request_target) == 10 && strncmp(request_target, "/user-agent", 10) == 0) {
        // Read the User-Agent Header line
		char* user_agent_value;
        char* line = strtok(network_stream, "\r\n");
        while (line != NULL) {
            if (strncmp(stringToLower(line), "user-agent:", 10) == 0) {
                user_agent_value = line + 12;
				break;
            }
            line = strtok(NULL, "\r\n");
			// printf("Line: {%s}\n", line);
        }
		if (user_agent_value)
		{
			int content_length = strlen(user_agent_value);
			snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s", content_length, user_agent_value);
		}
		else {
			snprintf(response, sizeof(response), "HTTP/1.1 404 Not Found\r\n\r\n");
		}
		printf("%s", user_agent_value);
		
    }
	else if (strcmp(request_target, "/") != 0) {
		snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\n\r\n");
	}
	else {
		snprintf(response, sizeof(response), "HTTP/1.1 404 Not Found\r\n\r\n");
	}
	free(network_stream);

	// Send a response to the client
	int bytes_sent = send(fd, response, strlen(response), MSG_DONTWAIT);
}

int main() {
	/* Disable output buffering */
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	int server_fd, client_fd;
	int client_addr_len;
	struct sockaddr_in serv_addr, client_addr;
	
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	
	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}
	
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(4221);
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	// struct in_addr{(in_addr_t)htonl((in_addr_t)((int)0))}
	
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
	 	printf("Bind failed: %s \n", strerror(errno));
	 	return 1;
	}
	
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
	 	printf("Listen failed: %s \n", strerror(errno));
	 	return 1;
	}
	client_addr_len = sizeof(client_addr);
	printf("Waiting for a clients to connect...\n");

	while(1) {
		// Accepting an incoming socket connection from a file descriptor
		client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
		if (client_fd < 0) {
			printf("Error accepting connection %s \n", strerror(errno));
			return 1;
		}
		printf("Client connected\n");

		pthread_t thread_id;
		pthread_create(&thread_id, NULL, handle_client, &client_fd);
	}

	close(server_fd);

	return 0;
}
