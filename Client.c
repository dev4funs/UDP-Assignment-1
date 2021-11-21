#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/time.h>

#include "Packet.h"

static int PAYLOAD_LENGTH = 5;

// declare
void AssignInputToDataPacket(struct Data_Packets *data_packet, char segment_number, char length, char payload_length, unsigned short end_packet_id);
void receivePacket(int client_socket, struct sockaddr_in address, char *req_buffer, int packet_length);
struct sockaddr_in GetServerAddress(int port_no);

int main()
{
    // init the parameters
    int client_socket;
    struct sockaddr_in server_address;
    struct Data_Packets data_packet;
    struct Ack_Packets ack_packet;
    char buffer[1024];
    int segment_number = 1;
    int input;

    /*---- Show the instruction in console ----*/
    printf("\nPress 0 to send 5 correct packets\n");
    printf("\nPress 1 to send 1 correct packet and 4 wrong packets\nYour input = ");

    /*---- Get the Inputs from console ----*/
    scanf("%d", &input);

    // Create client socket
    client_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // Set timer options on socket to 3 seconds
    struct timeval timeout = {3, 0};

    // init the socket optional settings
    // setsockopt if success return 0, if fail return -1
    if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1)
    {
        // handle the error
        error(" setsockopt Error !");
    }

    // Init server address
    server_address = GetServerAddress(PORT_NO);

    int packet_number = 1;
    while (packet_number <= 5)
    {
        if (input == 0)
        {
            // Initialize data packet with  all 5 correct packets
            AssignInputToDataPacket(&data_packet, segment_number, PAYLOAD_LENGTH, PAYLOAD_LENGTH, END_PACKET_ID);
        }
        else if (input == 1)
        {

            switch (packet_number)
            {
            case 1:
                // Initialize data packet with 1 correct packet and 4 error packets
                AssignInputToDataPacket(&data_packet, segment_number, PAYLOAD_LENGTH, PAYLOAD_LENGTH, END_PACKET_ID);
                break;
            case 2:
                // Assign wrong length for 2nd packet
                AssignInputToDataPacket(&data_packet, segment_number, 8, PAYLOAD_LENGTH, END_PACKET_ID);
                break;
            case 3:
                // NULL end packet id for 3rd packet
                AssignInputToDataPacket(&data_packet, segment_number, PAYLOAD_LENGTH, PAYLOAD_LENGTH, '\0');
                break;
            case 4:
                // Duplicate packet
                AssignInputToDataPacket(&data_packet, segment_number - 1, PAYLOAD_LENGTH, PAYLOAD_LENGTH, END_PACKET_ID);
                break;
            case 5:
                // Assign wrong sequence number for 5th packet
                AssignInputToDataPacket(&data_packet, segment_number + 10, PAYLOAD_LENGTH, PAYLOAD_LENGTH, END_PACKET_ID);
                break;
            }
        }

        int packet_length = buildDataPacket(data_packet, buffer);

        // Send data packet to server
        sendto(client_socket, buffer, packet_length, 0, (struct sockaddr *)&server_address, sizeof server_address);

        printf("Packet %d Sent: \n", packet_number);
        receivePacket(client_socket, server_address, buffer, packet_length);
        printf("\n\n");

        /*---- increase the packet number and segment number ----*/
        packet_number++;
        segment_number++;
    }

    return 0;
}

void AssignInputToDataPacket(struct Data_Packets *data_packet, char segment_number, char length, char payload_length, unsigned short end_packet_id)
{

    data_packet->start_packet_id = START_PACKET_ID;
    data_packet->client_id = 24;
    data_packet->packet_type = DATA_PACKET;
    data_packet->segment_no = segment_number;
    data_packet->length = length;
    data_packet->payload.length = payload_length;
    memcpy(data_packet->payload.data, "Hello", data_packet->payload.length);
    data_packet->end_packet_id = end_packet_id;
}

void receivePacket(int client_socket, struct sockaddr_in address, char *req_buffer, int packet_length)
{
    // Receive ACK or REJECT packet from server
    char ack_buffer[1024];
    struct sockaddr sender;
    struct Reject_Packets reject_packet;

    socklen_t sendsize = sizeof(sender);
    memset(&sender, 0, sizeof(sender));

    /*---- Bind the address struct to the socket ----*/
    bind(client_socket, (struct sockaddr *)&sender, sendsize);

    int retry_counter = 0;

    while (retry_counter <= 3)
    {
        int recvlen = recvfrom(client_socket, ack_buffer, sizeof(ack_buffer), 0, &sender, &sendsize);

        if (recvlen >= 0)
        {
            // Message Received
            // Determine whether the received packet is ACK or REJECT
            unsigned short packet_type;
            memcpy(&packet_type, ack_buffer + 3, 2);

            if (packet_type == ACK_PACKET)
            {
                printf("ACK \n");
            }
            else if (packet_type == REJECT_PACKET)
            {

                int reject_code = decodeRejectPacket(ack_buffer, &reject_packet);
                printRejectCode(reject_code);
            }
            break;
        }
        else
        {
            // If there is still no response from server after the last try to retransmit
            if (retry_counter == 3)
            {
                printf("Server does not respond\n");
                break;
            }
            retry_counter++;
            printf("Trying to retransmit..... Attempt no. %d\n", retry_counter);
            sendto(client_socket, req_buffer, packet_length, 0, (struct sockaddr *)&address, sizeof address);
        }
    }
}

struct sockaddr_in GetServerAddress(int port_no)
{
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT_NO);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    return address;
};