#include <stdio.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include "Tool.h"
#include "Packet.h"
int main()
{
    int serverSocket;
    char buffer[1024];
    int current_segment_number = 0;
    struct sockaddr_in server_addr;
    struct Data_Packets data_packet;
    struct Ack_Packets ack_packet;
    struct Reject_Packets reject_packet;
    int input;

    /*---- Show the instruction in console ----*/
    printf("\nPress 0 to simulate normal transmission\n");
    printf("\nPress 1 to simulate re-transmission\nYour input = ");

    /*---- Get the Inputs from console ----*/
    scanf("%d", &input);

    /*---- Init the socket ----*/
    serverSocket = socket(AF_INET, SOCK_DGRAM, 0);

    /*---- Configure server address struct ----*/
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT_NO);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /*---- Bind the address struct to the socket ----*/
    bind(serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    struct sockaddr sender;
    socklen_t sendsize = sizeof(sender);
    memset(&sender, 0, sizeof(sender));

    // Start to listen for packets from client
    while (1)
    {
        int packet_length = 0;

        recvfrom(serverSocket, buffer, sizeof(buffer), 0, &sender, &sendsize);

        if (input == 0)
        {
            // Normal packet transmission

            int response = decodeDataPacket(buffer, &data_packet);

            current_segment_number++;

            if (response == 0)
            {

                printf("Received Packet %d , Expecting Packet %d\n", data_packet.segment_no, current_segment_number);

                // Check Error case - Packet out of sequence
                if (data_packet.segment_no > current_segment_number)
                {

                    initializeRejectPacket(data_packet, &reject_packet, REJECT_OUT_OF_SEQUENCE);
                    packet_length = buildRejectPacket(reject_packet, buffer);
                }
                else if (data_packet.length != data_packet.payload.length)
                {

                    // Check Error Case - Length Mismatch
                    initializeRejectPacket(data_packet, &reject_packet, REJECT_LENGTH_MISMATCH);
                    packet_length = buildRejectPacket(reject_packet, buffer);
                }
                else if (data_packet.segment_no < current_segment_number)
                {

                    // Check Error Case - Duplicate Packet
                    initializeRejectPacket(data_packet, &reject_packet, REJECT_DUPLICATE_PACKET);
                    packet_length = buildRejectPacket(reject_packet, buffer);
                }
                else
                {

                    //Initialize ACK packet
                    ack_packet.start_packet_id = START_PACKET_ID;
                    ack_packet.client_id = 24;
                    ack_packet.packet_type = ACK_PACKET;
                    ack_packet.received_segment_no = data_packet.segment_no;
                    ack_packet.end_packet_id = END_PACKET_ID;

                    packet_length = buildAckPacket(ack_packet, buffer);
                    printf("Sending ACK\n");
                }
            }
            else if (response == REJECT_END_OF_PACKET_MISSING)
            {
                //Check Error case 3 - End packet missing
                printf("REJECT CODE - %x - REJECT_END_OF_PACKET_MISSING\n", REJECT_END_OF_PACKET_MISSING);

                // Initialize END PACKET MISSING REJECT packet
                initializeRejectPacket(data_packet, &reject_packet, REJECT_END_OF_PACKET_MISSING);
                packet_length = buildRejectPacket(reject_packet, buffer);
            }

            // Send ACK or REJECT packet to client.
            sendto(serverSocket, buffer, packet_length, 0, (struct sockaddr *)&sender, sendsize);

            printf("\n\n");
        }
        else if (input == 1)
        {
            // Simulate re-transmission
            input = 0;
        }
    }

    return 0;
}