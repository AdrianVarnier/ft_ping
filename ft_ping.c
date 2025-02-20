#include "ft_ping.h"

static int seq;

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

void    set_header(struct icmphdr* icmp)
{
    memset(icmp, 0, sizeof(struct icmphdr));
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.id = getpid() & 0xFFFF;
    icmp->un.echo.sequence = seq++;
    icmp->checksum = checksum(icmp, sizeof(struct icmphdr));
}

int    send_ping(char* arg)
{
    struct addrinfo* res;
    struct addrinfo  hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_RAW;
    hints.ai_protocol = IPPROTO_ICMP;
    if (getaddrinfo(arg, NULL, &hints, &res) != 0)
    {
        fprintf(stderr, "ft_ping: unknow host\n");
        return -1;
    }
    
    struct icmphdr icmp;
    set_header(&icmp);

    char buffer[PACKET_SIZE];
    memset(buffer, 0, PACKET_SIZE);
    memcpy(buffer, &icmp, sizeof(icmp));
 
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
        return 1;

    struct timeval start, end;
    gettimeofday(&start, NULL);
    if (sendto(sockfd, buffer, sizeof(icmp), 0, res->ai_addr, res->ai_addrlen) < 0 )
    {
        close(sockfd);
        return 1;
    }

    socklen_t recv_len = res->ai_addrlen;
    if (recvfrom(sockfd, buffer, sizeof(buffer), 0, res->ai_addr, &res->ai_addrlen) <= 0)
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
    seq = 0;
    send_ping(argv[1]);

    return 0;
}