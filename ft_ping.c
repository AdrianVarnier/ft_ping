#include "ft_ping.h"

unsigned short checksum(void *b, int len)
{
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    
    if (len == 1)
        sum += *(unsigned char *)buf;

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

void    send_ping(char* arg)
{
    struct sockaddr_in addr;
    struct hostent *he = gethostbyname(arg);
    memcpy(&addr.sin_addr, he->h_addr, he->h_length);

    static int seq = 0;
    struct icmphdr icmp;
    memset(&icmp, 0, sizeof(icmp));
    icmp.type = ICMP_ECHO;
    icmp.code = 0;
    icmp.un.echo.id = getpid() & 0xFFFF;
    icmp.un.echo.sequence = seq++;
    icmp.checksum = 0;
    icmp.checksum = checksum(&icmp, sizeof(icmp));
    
    char packet[PACKET_SIZE];
    memset(packet, 0, PACKET_SIZE);
    memcpy(packet, &icmp, sizeof(icmp));
 
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
        return ;
    if (sendto(sockfd, packet, sizeof(icmp), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
        return ;
}

int main(int argc, char** argv)
{
    send_ping(argv[1]);
    return 0;
}