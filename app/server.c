#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main() {
	// Disable output buffering
	setbuf(stdout, NULL);
 	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;
	
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
	
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
	 								 .sin_port = htons(4221),
	 								 .sin_addr = { htonl(INADDR_ANY) },
	 								};
	
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
	 	printf("Bind failed: %s \n", strerror(errno));
	 	return 1;
	}
	
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
	 	printf("Listen failed: %s \n", strerror(errno));
	 	return 1;
	}
	
	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);
	
	// Accepting an incoming socket connection from a file descriptor
	int fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	if (fd < 0) {
		printf("Error accepting connection %s \n", strerror(errno));
		return 1;
	}
	printf("Client connected\n");

	// Recieving network stream from incoming socket connection
	char* network_stream = malloc(1025);
	int ret = recv(fd, network_stream, 1024, 0);
	if (ret < 0) {
		printf("Error recieving socket stream: %s \n", strerror(errno));
		return 1;
	}

	// Check url
	char* method = strtok(network_stream, " ");
	char* request_target = strtok(NULL, " ");
	char response[1024];

	
	if (strncmp(request_target, "/echo/", 6) == 0 && strlen(request_target) > 6) {
		// Get the request body
		char* body = request_target + 6;
		// Dynamically a request
		snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: 3\r\n\r\n%s", body);
	}
	else if (strcmp(request_target, "/") != 0) {
		snprintf(response, sizeof(response), "HTTP/1.1 404 Not Found\r\n\r\n");
	}
	else {
		snprintf(response, sizeof(response), "HTTP/1.1 200 OK\r\n\r\n");
	}
	printf("{%s}\n", response);
	free(network_stream);

	// Send a response to the client
	int bytes_sent = send(fd, response, strlen(response), MSG_DONTWAIT);

	close(server_fd);

	return 0;
}
