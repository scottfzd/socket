#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>



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


  fd_set read_fds;
  FD_ZERO(&read_fds);
  FD_SET(server_fd, &read_fds);
  int max_fd = server_fd + 1;

  while (1) {

    select(max_fd, &read_fds, NULL, NULL, NULL);


    // Accept client connection
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
   
      
    if (FD_ISSET(server_fd, &read_fds)) {
      int client_fd = accept(server_fd, (struct  sockaddr *)&client_addr, &client_len);
      FD_SET(client_fd, &read_fds);
      max_fd = client_fd + 1;
    }

    for (int i = 0; i < max_fd; i++) {

      if (FD_ISSET(i, &read_fds)) {

        char buffer[1024];

        ssize_t n = read(i, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("Received from client: %s", buffer);
        }
      }
    }

  }
}
