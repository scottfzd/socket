#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <arpa/inet.h>

typedef struct {
  int id;
  int fd;
  char ip[INET_ADDRSTRLEN];
  // buffer
  // state
} client_t;

void handle_incoming_connection(int server_fd, fd_set *all_fds, int *max_fd, client_t *clients, int *next_client_id) {

  // Accept client connection
  struct sockaddr_in client_addr;
  socklen_t client_len = sizeof(client_addr);

  int client_fd = accept(server_fd, (struct  sockaddr *)&client_addr, &client_len);

  if (client_fd < 0) {
    perror("accept");
    return;
  }

  FD_SET(client_fd, all_fds);

  if (client_fd > *max_fd) { 
    *max_fd = client_fd;
  }

  client_t client;
  client.id = *next_client_id;
  client.fd = client_fd;
  inet_ntop(AF_INET, &client_addr.sin_addr, client.ip, INET_ADDRSTRLEN);


  clients[client_fd] = client;

  (*next_client_id) ++;

  printf("%i\n", client.id);
  printf("%s\n", client.ip);

}

void handle_client_data(int client_fd, client_t *clients, fd_set *all_fds) {

  char buffer[1024];

  ssize_t n = read(client_fd, buffer, sizeof(buffer) - 1);
  if (n > 0) {
      buffer[n] = '\0';
      char *client_ip = clients[client_fd].ip;
      printf("Received from client %s: %s", client_ip, buffer);
  } else if (n == 0) {
      // EOF
      FD_CLR(client_fd, all_fds);
      memset(&clients[client_fd], 0, sizeof(client_t));
      close(client_fd);
  } else {
    // error
    perror("read");
    FD_CLR(client_fd, all_fds);
    memset(&clients[client_fd], 0, sizeof(client_t));
    close(client_fd);
  }

}

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


  int opt = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  // Bind socket to the address
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("bind");
  }

  // Listen for incoming connections
  listen(server_fd, 5);


  fd_set read_fds;
  FD_ZERO(&read_fds);

  // permanent set
  fd_set all_fds;
  FD_ZERO(&all_fds);
  FD_SET(server_fd, &all_fds);
  FD_SET(STDIN_FILENO, &all_fds);
  int max_fd = server_fd;

  client_t clients[FD_SETSIZE];
  memset(clients, 0, sizeof(clients));

  int next_client_id = 1;

  while (1) {

    read_fds = all_fds;

    // Blocks until a fd is ready
    select(max_fd + 1, &read_fds, NULL, NULL, NULL);
    
    
    // Iterate over all fds
    for (int i = 0; i <= max_fd; i++) {

      // Handle fd only if readable
      if (FD_ISSET(i, &read_fds)) {

        // If fd is server_fd, handle new connection
        if (i == server_fd) {


          handle_incoming_connection(server_fd, &all_fds, &max_fd, clients, &next_client_id);

        } else if (i == STDIN_FILENO) {

          // else if STDIN handle terminal input
          char input[1024];

          if (fgets(input, sizeof(input), stdin) != NULL) {

            char *action = strtok(input, " ");
            char *id_str  = strtok(NULL, " ");
            char *msg = strtok(NULL, "");

            if (!action || !id_str || !msg) {
                printf("Usage: action <id> <msg>\n");
                continue;
            }

            int id = atoi(id_str);

            int client_fd = -1;
            for (int j = 0; j < sizeof(clients)/sizeof(client_t); j++) {
              if (clients[j].fd != 0 && clients[j].id == id) {
                client_fd = clients[j].fd;
                break;
              }
            }

            if (client_fd == -1) {
              printf("Client not found\n");
              continue;
            }

            if (strcmp(action, "sendcmd") == 0) {

              size_t sent = 0;
              size_t msg_len = strlen(msg);
              while (sent < msg_len) {
                
                ssize_t n_bytes = send(client_fd, msg + sent, msg_len - sent, 0);

                if (n_bytes > 0) {
                  sent += n_bytes;
                } else {
                  perror("send");
                  break;
                }

              }
            }

          }


        } else {


          handle_client_data(i, clients, &all_fds);

        }

      }
    }

  }
}
