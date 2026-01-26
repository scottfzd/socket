#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>



int main() {

  // Create socket
  int server_fd;
  server_fd = socket(AF_INET, SOCK_STREAM, 0);

  if (server_fd < 0) {
    perror("socket");
    exit(EXIT_FAILURE);
  }



  // Socket address
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr)); 
  
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(8080);


  // Bind socket to the address
  bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));

  // Listen for incoming connections
  listen(server_fd, 5);


  // Accept a client connection

  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);

  int client_fd = accept(server_fd, (struct  sockaddr *)&client_addr, &client_len);

  printf("Client connected!\n");

  while (1) {
    
  }
}
