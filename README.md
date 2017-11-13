# IP-Forwarding
IP Forwarding program written in C

### Files:

  ipforward.c
  
  makefile
  
This program consists of a c program and a makefile that can be used to compile it. The ipforward program accepts three arguments: the forwarding table to use for incoming packets, the binary file that contains packets to be sent, and a binary file to take the output from the forwarding. Using make clean removes the compiled program as well as the output file. The program can be run with:

    "./ipforward <forwarding_table.txt> <ip_packets> <ip_packets_out>"

The program first uses a struct called IPHeader to get access of each value from the packets. Once the files are each opened a loop runs through the ip_packets binary file and appropriate values are obtained then the TTL is reduced by 1. The destination IP address is then used in the proper byte order and compared to all of the tuples in the forwarding table. A loop within the first one goes through each tuple and converts each string value into the proper byte order. The mask and destination IP are anded together then compared to the net ID. If there is a match the next hop value is saved, if a match is later found with a longer mask the next hop is overwritten. The program displays all proper values to the user and whether the packet is dropped or not. If the packet is not dropped it is written to the ip_packets_out file.
