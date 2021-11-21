#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "Packet.h"

struct Ack_Packet generateAckPacket(char segment_no)
{
  struct Ack_Packet ack_packet;
  ack_packet.start_packet_id = START_PACKET_ID;
  ack_packet.client_id = 24;
  ack_packet.packet_type = ACK_PACKET;
  ack_packet.received_segment_no = segment_no;
  ack_packet.end_packet_id = END_PACKET_ID;

  return ack_packet;
};