#include "ft_ping.h"

unsigned short checksum(unsigned short* ptr, int bytes)
{
    unsigned int sum = 0;

    for (sum = 0; bytes > 1; bytes -= 2)
        sum += *ptr++;
    if (bytes == 1)
        sum += *(unsigned char *)ptr;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (~sum);
}

void    ft_ping(const char* target, int option)
{
    // check target
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, target, &addr.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid address: %s\n", target);
        exit(1);
    }

    // create socket
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // create packet
    char packet[64];
    struct icmp* icmp_packet = (struct icmp *)packet;
    icmp_packet->icmp_type = ICMP_ECHO;
    icmp_packet->icmp_code = 0;
    icmp_packet->icmp_cksum = 0;
    icmp_packet->icmp_id = getpid();
    icmp_packet->icmp_seq = 1;
    icmp_packet->icmp_cksum = checksum((unsigned short *)packet, sizeof(packet));

    // send packet
    if (sendto(sockfd, packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr)) <= 0)
    {
        perror("sendto");
        close(sockfd);
        exit(1);
    }

    // receive reply
    char reply[1024];
    if (recvfrom(sockfd, reply, sizeof(reply), 0, (struct sockaddr *) &reply_addr, &addr_len) <= 0)
    {
        perror("recvfrom");
    }
    else if (option)
    {
        printf("Received ICMP reply from %s\n", inet_ntoa(reply_addr.sin_addr));
    }

    close(sockfd);
}