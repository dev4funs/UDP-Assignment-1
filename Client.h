#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "Packet.h"

struct Data_Packets GenerateDataPacketToSend(char segment_no, char length, char payload_length, unsigned short end_packet_id)
{
  struct Data_Packets data_packet;
  data_packet.start_packet_id = START_PACKET_ID;
  data_packet.client_id = 24;
  data_packet.packet_type = DATA_PACKET;
  data_packet.segment_no = segment_no;
  data_packet.length = length;
  data_packet.payload.length = payload_length;
  memcpy(data_packet.payload.data, "Test", data_packet.payload.length);
  data_packet.end_packet_id = end_packet_id;
  return data_packet;
}

void receiveHandler(int client_socket, struct sockaddr_in address, int packet_length, char *req_buffer)
{
  char response_buffer[1024];
  struct sockaddr sender;
  const int max_retry_times = 3;

  socklen_t sendsize = sizeof(sender);
  memset(&sender, 0, sizeof(sender));

  /*---- Bind the address struct to the socket ----*/
  bind(client_socket, (struct sockaddr *)&sender, sendsize);

  int retry_no = 0;

  while (retry_no <= max_retry_times)
  {
    // Upon successful completion, recvfrom() shall return the length of the message in bytes.
    // If no messages are available to be received and the peer has performed an orderly shutdown,
    // recvfrom() shall return 0. Otherwise, the function shall return -1 and set errno to indicate the error.
    int response = recvfrom(client_socket, response_buffer, sizeof(response_buffer), 0, &sender, &sendsize);

    if (response < 0)
    {
      if (retry_no == max_retry_times)
      {
        printf("No respond from server. \n");
        break;
      }
      retry_no++;
      printf("%d. Packet Retransmiting..... \n", retry_no);
      sendto(client_socket, req_buffer, packet_length, 0, (struct sockaddr *)&address, sizeof address);
    }
    else
    {
      // Message Received
      // Determine whether the received packet is ACK or REJECT
      struct Reject_Packets reject_packet;
      resolvePacket(&reject_packet, response_buffer);

      if (reject_packet.packet_type == ACK_PACKET)
      {
        printf("status: ACK \n");
      }
      else if (reject_packet.packet_type == REJECT_PACKET)
      {

        printf("status: %04x %s \n", reject_packet.reject_sub_code, getRejectDescription(reject_packet.reject_sub_code));
      }
      break;
    }
  }
}