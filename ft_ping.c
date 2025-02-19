#include "ft_ping.h"

uint16_t checksum(void *ptr, int len)
{
    uint16_t* data = ptr;
    uint32_t sum = 0;
    for (; len > 1; len -= 2)
        sum += *data++;
    if (len == 1)
        sum += *(uint8_t*)data;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return ~sum;
}

int    send_ping(char* arg)
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
    
    char buffer[PACKET_SIZE];
    memset(buffer, 0, PACKET_SIZE);
    memcpy(buffer, &icmp, sizeof(icmp));
 
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
        return 1;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    if (sendto(sockfd, buffer, sizeof(icmp), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0 )
    {
        close(sockfd);
        return 1;
    }
    socklen_t len = sizeof(addr);
    if (recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &len) <= 0)
    {
        close(sockfd);
        return 1;
    }
    gettimeofday(&end, NULL);
    double rtt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    printf("ping %d %.3f\n", seq, rtt);
    return 0;
}

int main(int argc, char** argv)
{
    send_ping(argv[1]);
    send_ping(argv[1]);
    return 0;
}