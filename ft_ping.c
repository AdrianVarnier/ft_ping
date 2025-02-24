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

void    set_echo_header(t_data* data)
{
    data->header.type = ICMP_ECHO;
    data->header.code = 0;
    data->header.un.echo.id = getpid() & 0xFFFF;
    data->header.un.echo.sequence = 0;
    data->header.checksum = checksum(&data->header, sizeof(struct icmphdr));
}

void    set_addr_hint(t_data* data)
{
    data->hints.ai_family = AF_INET;
    data->hints.ai_socktype = SOCK_RAW;
    data->hints.ai_protocol = IPPROTO_ICMP;
}

void    fill_buffer(t_data *data)
{
    memcpy(data->buffer, &data->header, sizeof(struct icmphdr));
}

void    increment_seq(t_data* data)
{
    data->header.un.echo.sequence = 0;
    data->header.checksum = checksum(&data->header, sizeof(struct icmphdr));
    fill_buffer(data);  
}

void    init_data(t_data* data)
{
    memset(data, 0, sizeof(t_data));
    set_echo_header(data);
    set_addr_hint(data);
    fill_buffer(data);
}

int     resolve_addr(t_data* data, char* addr)
{
    if (getaddrinfo(addr, NULL, &data->hints, &data->res) != 0)
    {
        fprintf(stderr, "ft_ping: unknow host\n");
        return -1;
    }
    return 0;
}

int    send_ping(t_data* data)
{
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0)
        return 1;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    if (sendto(sockfd, data->buffer, sizeof(struct icmphdr), 0, data->res->ai_addr, data->res->ai_addrlen) < 0)
    {
        close(sockfd);
        return 1;
    }
    socklen_t recv_len = data->res->ai_addrlen;
    if (recvfrom(sockfd, data->buffer, sizeof(data->buffer), 0, data->res->ai_addr, &data->res->ai_addrlen) <= 0)
    {
        close(sockfd);
        return 1;
    }
    gettimeofday(&end, NULL);
    data->ttl = ((struct ip *)data->buffer)->ip_ttl;
    data->rtt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    return 0;
}

void    display_dest_info(t_data *data, char* arg)
{
    char ip[INET_ADDRSTRLEN];
    inet_ntop(data->res->ai_family, (struct sockaddr_in *)data->res->ai_addr, ip, sizeof(ip));
    printf("PING %s (%s)\n", arg, ip);
}

void    display_ping_info(t_data *data)
{
    printf("icmp_seq=%d ttl=%d time=%.3f ms\n", data->seq, data->ttl, data->rtt);
}

int main(int argc, char** argv)
{
    t_data data;
    init_data(&data);
    if (resolve_addr(&data, argv[1]) < 0)
        return 1;
    int ret = send_ping(&data);
    display_dest_info(&data, argv[1]);
    display_ping_info(&data);
    printf("%d\n", ret);
    return 0;
}