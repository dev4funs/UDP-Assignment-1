#include <string.h>
#include <stdio.h>

#define START_PACKET_ID 0xFFFF
#define END_PACKET_ID 0xFFFF
#define DATA_PACKET 0xFFF1
#define ACK_PACKET 0xFFF2
#define REJECT_PACKET 0xFFF3
#define REJECT_OUT_OF_SEQUENCE 0xFFF4
#define REJECT_LENGTH_MISMATCH 0xFFF5
#define REJECT_END_OF_PACKET_MISSING 0xFFF6
#define REJECT_DUPLICATE_PACKET 0xFFF7

static int PORT_NO = 7891;

struct Payload
{
    char data[255];
    char length;
};

struct Data_Packet
{
    struct Payload payload;
    unsigned short end_packet_id;
    unsigned short start_packet_id;
    unsigned short packet_type;
    char client_id;
    char segment_no;
    char length;
};

struct Reject_Packet
{
    unsigned short packet_type;
    unsigned short reject_sub_code;
    char received_segment_no;
    unsigned short start_packet_id;
    unsigned short end_packet_id;
    char client_id;
};

struct Ack_Packet
{
    unsigned short packet_type;
    char received_segment_no;
    unsigned short start_packet_id;
    unsigned short end_packet_id;
    char client_id;
};

int initPacketFromBuffer(struct Data_Packet data_packet, char *buffer)
{
    int buffer_length = 0;

    memcpy(&buffer[buffer_length], &data_packet.start_packet_id, 2);
    buffer_length += 2;

    buffer[buffer_length] = data_packet.client_id;
    buffer_length += 1;

    memcpy(&buffer[buffer_length], &data_packet.packet_type, 2);
    buffer_length += 2;

    buffer[buffer_length] = data_packet.segment_no;
    buffer_length += 1;

    buffer[buffer_length] = data_packet.length;
    buffer_length += 1;

    memcpy(&buffer[buffer_length], &data_packet.payload.data, data_packet.payload.length);
    buffer_length += data_packet.payload.length;

    memcpy(&buffer[buffer_length], &data_packet.end_packet_id, 2);
    buffer_length += 2;

    buffer[buffer_length + 1] = '\0';

    return buffer_length;
}

/**
 * This function builds the buffer to be sent using the data in the ack_packet 
 * Input params are ack_packet and buffer
 * Output is the length of the data in the buffer
 **/
int generateBufferFromAckPacket(struct Ack_Packet ack_packet, char *buffer)
{
    int buffer_length = 0;

    memcpy(&buffer[buffer_length], &ack_packet.start_packet_id, 2);
    buffer_length += 2;

    buffer[buffer_length] = ack_packet.client_id;
    buffer_length += 1;

    memcpy(&buffer[buffer_length], &ack_packet.packet_type, 2);
    buffer_length += 2;

    buffer[buffer_length] = ack_packet.received_segment_no;
    buffer_length += 1;

    memcpy(&buffer[buffer_length], &ack_packet.end_packet_id, 2);
    buffer_length += 2;

    buffer[buffer_length + 1] = '\0';

    return buffer_length;
}

/**
 * This function builds the buffer to be sent using the data in the reject_packet 
 * Input params are reject_packet and buffer
 * Output is the length of the data in the buffer
 **/
int generateBufferFromRejectPacket(struct Reject_Packet reject_packet, char *buffer)
{

    int buffer_length = 0;

    memcpy(&buffer[buffer_length], &reject_packet.start_packet_id, 2);
    buffer_length += 2;

    buffer[buffer_length] = reject_packet.client_id;
    buffer_length += 1;

    memcpy(&buffer[buffer_length], &reject_packet.packet_type, 2);
    buffer_length += 2;

    memcpy(&buffer[buffer_length], &reject_packet.reject_sub_code, 2);
    buffer_length += 2;

    buffer[buffer_length] = reject_packet.received_segment_no;
    buffer_length += 1;

    memcpy(&buffer[buffer_length], &reject_packet.end_packet_id, 2);
    buffer_length += 2;

    buffer[buffer_length + 1] = '\0';

    return buffer_length;
}

struct Data_Packet generateDataPacketFromBuffer(char *buffer)
{
    struct Data_Packet data_packet;

    int buffer_length = 0;

    memcpy(&(data_packet.start_packet_id), buffer + buffer_length, 2);
    buffer_length += 2;

    data_packet.client_id = buffer[buffer_length];
    buffer_length += 1;

    memcpy(&(data_packet.packet_type), buffer + buffer_length, 2);
    buffer_length += 2;

    data_packet.segment_no = buffer[buffer_length];
    buffer_length += 1;

    data_packet.length = buffer[buffer_length];
    buffer_length += 1;

    int payload_length = 0;
    while (payload_length < 255)
    {
        unsigned short data;
        memcpy(&data, buffer + buffer_length, 2);
        if (data == END_PACKET_ID)
        {
            break;
        }
        else
        {
            data_packet.payload.data[payload_length] = buffer[buffer_length];
            buffer_length++;
            payload_length++;
        }
    }
    data_packet.payload.length = payload_length;

    memcpy(&(data_packet.end_packet_id), buffer + buffer_length, 2);

    return data_packet;
}

struct Reject_Packet generateRejectPacket(char segment_no, int reject_sub_code)
{
    struct Reject_Packet reject_packet;
    reject_packet.start_packet_id = START_PACKET_ID;
    reject_packet.client_id = 24;
    reject_packet.packet_type = REJECT_PACKET;
    reject_packet.reject_sub_code = reject_sub_code;
    reject_packet.received_segment_no = segment_no;
    reject_packet.end_packet_id = END_PACKET_ID;
    return reject_packet;
}

void initRejectPacketFromBuffer(struct Reject_Packet *reject_packet, char *buffer)
{

    int buffer_length = 0;

    memcpy(&(reject_packet->start_packet_id), buffer + buffer_length, 2);
    buffer_length += 2;

    reject_packet->client_id = buffer[buffer_length];
    buffer_length += 1;

    memcpy(&(reject_packet->packet_type), buffer + buffer_length, 2);
    buffer_length += 2;

    memcpy(&(reject_packet->reject_sub_code), buffer + buffer_length, 2);
    buffer_length += 2;

    reject_packet->received_segment_no = buffer[buffer_length];
    buffer_length += 1;

    memcpy(&(reject_packet->end_packet_id), buffer + buffer_length, 2);
}

char *getRejectDescription(int reject_sub_code)
{
    switch (reject_sub_code)
    {
    case 0xfff4:
        return "REJECT_OUT_OF_SEQUENCE";
        break;
    case 0xfff5:
        return "REJECT_LENGTH_MISMATCH";
        break;
    case 0xfff6:
        return "REJECT_END_OF_PACKET_MISSING";
        break;
    case 0xfff7:
        return "REJECT_DUPLICATE_PACKET";
        break;
    default:
        return "Unkown Reject Sub Code";
        break;
    }
}

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