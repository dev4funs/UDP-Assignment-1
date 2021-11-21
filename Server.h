#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct sockaddr_in GetServerAddress(int port_no)
{
  struct sockaddr_in address;
  memset(&address, 0, sizeof(address));
  address.sin_family = AF_INET;
  // htons() which converts a port number in host byte order to a port number in network byte order
  address.sin_port = htons(port_no);
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  return address;
};