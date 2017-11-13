#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <math.h>

struct IPHeader
{
	unsigned short vhl;
	unsigned short datagramLength;
	unsigned short identifier;
	unsigned short flagOff;
	unsigned char ttl;
	unsigned char ulp;
	unsigned short checksum;
	unsigned int sourceIP;
	unsigned int destinationIP;
}*p;

int main(int argc, char *argv[])
{
	FILE * forward_table;
	FILE * ipPacketsIn;
	FILE * ipPacketsOut;
	char line[1024];
	char* net;
	char IPBinary [2048];
	char* netID;
	char* mask;
	char* nextHop;
	struct sockaddr_in ante;
	struct in_addr addr;
	char ipPackIn [1024];
	char ipPackOut [1024];
	char packetHead [160];
	int startPos;
	int totalDataLength;
	unsigned int maskDest;
	unsigned int temp;
	int passes;
	char finalNext[512];
	int tempBitCount;
	int maskBitCount;
	int c;

	// If incorrect arg count, inform user of proper usage and exit with status 1.
	if(argc != 4)
	{
		printf("Proper usage: %s <forwarding_table.txt> <ip_packets> <ip_packets_out>\n", argv[0]);
		exit(1);
	}

	forward_table = fopen(argv[1], "rb");

	// If forwarding table file open fails, inform user and exit with status 1.
	if(!forward_table)
	{
		printf("forwarding_table open %s\n", "failed");
		exit(1);
	}

	ipPacketsIn = fopen(argv[2], "rb");

	// If ip_packets file open fails, inform user and exit with status 1.
	if(!ipPacketsIn)
	{
		printf("ip_packets open %s\n", "failed");
		exit(1);
	}

	// Open out file to be written to.
	ipPacketsOut = fopen(argv[3], "wb");

	startPos = 0;

	// Read through entire ip_packets file until it ends, skipping by proper datagramLength values.
	while((c = fgetc(ipPacketsIn)) != EOF)
	{
		// Reduce starting point of file ipPacketsIn to read next packet.
		fseek(ipPacketsIn, startPos, SEEK_SET);
		fread(packetHead, 20, 1, ipPacketsIn);

		// Get pointer p for struct IPHeader to be used to obtain values.
		p = (struct IPHeader*) packetHead;

		// Get full packet using the datagramLength specified.
		char packet[ntohs(p->datagramLength) * 8];
		fread(packet, ntohs(p->datagramLength) - 20, 1, ipPacketsIn);

		// Obtain ASCII value for sourceIP using inet_ntoa.
		addr.s_addr = p->sourceIP;
		char* src = inet_ntoa(addr);
		printf("Source IP: %s\n", src);

		// Obtain ASCII value for destinationIP using inet_ntoa.
		addr.s_addr = p->destinationIP;
		char* dest = inet_ntoa(addr);
		printf("Destination IP: %s\n", dest);

		printf("Packet Length: %u\n", ntohs(p->datagramLength));

		p->ttl -= 1;

		if(p->ttl < 1)
		{
			// Inform user that packet was dropped due to TTL being 0.
			printf("Dropped: %s\n\n", "Yes because TTL = 0");

			// Reset forwarding table and packet and move file startPos for next loop.
			fseek(forward_table, 0, SEEK_SET);
			startPos += ntohs(p->datagramLength);
			memset(packetHead, 0, 20);
			memset(packet, 0, ntohs(p->datagramLength));
			memset(finalNext, 0, 512);
			continue;
		}
		else
		{
			printf("Dropped: %s\n", "No");
		}

		// Set intitial values to temp and passes.
		temp = 0;
		passes = 0;
	
		// If packet is not dropped, find its nextHop.
		while(fgets(line, sizeof(line), forward_table))
		{
			// Get netID, mask, and nextHop values for further use.
			netID = strtok(line, " ");
			mask = strtok(NULL, " ");
			nextHop = strtok(NULL, " ");

			//printf("net %s\n", netID);

			// Get mask in bytes and network byte order and and with the destination IP.
			unsigned int maskBit = inet_addr(mask);
			ante.sin_addr.s_addr = p->destinationIP & maskBit;

			// Save that anded value and get value for netID.
			maskDest = ante.sin_addr.s_addr;
			ante.sin_addr.s_addr = inet_addr(netID);

			//if(maskDest == ante.sin_addr.s_addr)
			if((maskDest ^ ante.sin_addr.s_addr) == 0)
			{
				// If first run through, assign temp value and continue.
				if(passes == 0)
				{
					temp = maskBit;
					passes += 1;
					strcat(finalNext, nextHop);
					continue;
				}

				// Get bit count for future comparisons
				tempBitCount = floor(log2(temp)) + 1;
				maskBitCount = floor(log2(maskBit)) + 1;

				// If new mask is larger save new mask and next hop
				if(tempBitCount < maskBitCount)
				{
					temp = maskBit;
					memset(finalNext, 0, 512);
					strcat(finalNext, nextHop);
				}
			}
		}

		// Inform user of Next Hop value.
		printf("Next Hop: %s\n", finalNext);

		// Write to the outfile.
		fwrite(packetHead, 20, 1, ipPacketsOut);
		fwrite(packet, ntohs(p->datagramLength) - 20, 1, ipPacketsOut);

		// Reset forwarding table and packet and move file startPos for next loop.
		fseek(forward_table, 0, SEEK_SET);
		startPos += ntohs(p->datagramLength);
		memset(packetHead, 0, 20);
		memset(packet, 0, ntohs(p->datagramLength));
		memset(finalNext, 0, 512);
		c = fgetc(ipPacketsIn);

		// Print blankline for user readability.
		printf("%s\n", "");
	}

	// Properly close all files.
	fclose(forward_table);
	fclose(ipPacketsIn);
}
