#include "ft_ping.h"

t_data data;

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
    data->packet.header.type = ICMP_ECHO;
    data->packet.header.code = 0;
    data->packet.header.un.echo.id = getpid() & 0xFFFF;
    data->packet.header.un.echo.sequence = data->seq;
    data->packet.header.checksum = checksum(&data->packet, sizeof(data->packet));
}

void    set_addr_hint(t_data* data)
{
    data->hints.ai_family = AF_INET;
    data->hints.ai_socktype = SOCK_RAW;
    data->hints.ai_protocol = IPPROTO_ICMP;
}

void    update_packet(t_data *data)
{
    data->packet.header.un.echo.sequence = ++data->seq;
    data->packet.header.checksum = 0;
    data->packet.header.checksum = checksum(&data->packet, sizeof(data->packet));
}

void    init_data(t_data* data)
{
    memset(data, 0, sizeof(t_data));
    set_addr_hint(data);
    set_echo_header(data);
    if ((data->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) < 0)
        exit(1);
    setsockopt(data->sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    setsockopt(data->sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
}

void    display_dest_info(t_data *data, char* arg)
{
    inet_ntop(data->res->ai_family, (struct sockaddr_in *)data->res->ai_addr, data->ip, sizeof(data->ip));
    printf("PING %s (%s): %d bytes", arg, data->ip, PACKET_SIZE - 8);
    if (data->v)
        printf(", id %x = %d", data->packet.header.un.echo.id, data->packet.header.un.echo.id);
    printf("\n");
}

void    display_ping_info(t_data *data)
{
    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n", PACKET_SIZE, data->ip, data->seq, data->ttl, data->rtt);
}


void display_stats(t_data *data)
{
    double mean = data->rtt_sum / data->rtt_count;
    double stddev = sqrt((data->rtt_sqr_sum / data->rtt_count) - pow(mean, 2));
    int loss = 100 - (data->rtt_count / (data->seq + 1)) * 100;
    printf("--- ft_ping statistics ---\n");
    printf("%d packets transmitted, %d packets received, %d%% packet loss\n", data->rtt_count + data->errors, data->rtt_count, loss);
    printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", data->rtt_min, mean, data->rtt_max, stddev);
}

void    update_rtt(t_data* data)
{
    data->rtt_count++;
    data->rtt_sum += data->rtt;
    data->rtt_sqr_sum += pow(data->rtt, 2);
    if (data->rtt_min > data->rtt || data->rtt_min == 0)
        data->rtt_min = data->rtt;
    if (data->rtt_max < data->rtt)
        data->rtt_max = data->rtt;   
}

void    handle_response(t_data* data)
{
    struct icmphdr *response = (struct icmphdr *)(data->response + (((struct ip *)data->response)->ip_hl * 4));
    if (response->type == ICMP_ECHOREPLY)
    {
        update_rtt(data);
        display_ping_info(data);
    }
    else
    {
        data->errors++;
        if (data->v)
        {
           if (response->type == ICMP_DEST_UNREACH)
                printf("icmp_seq=%d Host Unreachable\n", data->seq);
            if (response->type == ICMP_TIME_EXCEEDED)
                printf("icmp_seq=%d Time to live exceeded\n", data->seq);
            if (response->type == ICMP_REDIRECT)
                printf("icmp_seq=%d Redirect Host\n", data->seq);
            if (response->type == ICMP_PARAMETERPROB)
                printf("icmp_seq=%d Parameter problem\n", data->seq);
        }
    }
}

void    free_data(t_data *data)
{
    close(data->sockfd);
    freeaddrinfo(data->res);  
}

void    exit_clean(int n)
{
    free_data(&data);
    exit(n);
}

void handle_sigint(int sig)
{
    display_stats(&data);
    exit_clean(0);
}

int    send_ping(t_data* data)
{
    struct timeval start, end;
    gettimeofday(&start, NULL);
    if (sendto(data->sockfd, &data->packet, sizeof(data->packet), 0, data->res->ai_addr, data->res->ai_addrlen) < 0)
        exit_clean(1);
    struct sockaddr_in sender_addr;
    socklen_t sender_len = sizeof(sender_addr);
    memset(&data->response, 0, sizeof(data->response));
    if ((data->bytes_received = recvfrom(data->sockfd, data->response, PACKET_SIZE + IP_HEADER_SIZE, 0, (struct sockaddr*)&sender_addr, &sender_len)) <= 0)
        exit_clean(1);
    gettimeofday(&end, NULL);
    data->ttl = ((struct ip *)data->response)->ip_ttl;
    data->rtt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    return 0;
}

int     resolve_addr(t_data* data, char* addr)
{
    if (getaddrinfo(addr, NULL, &data->hints, &data->res) != 0)
    {
        fprintf(stderr, "ft_ping: unknow host\n");
        exit_clean(1);
    }
    return 0;
}

int     main(int argc, char** argv)
{
    signal(SIGINT, handle_sigint);
    init_data(&data);
    resolve_addr(&data, argv[1]);
    display_dest_info(&data, argv[1]);
    while (1)
    {
        send_ping(&data);
        handle_response(&data);
        update_packet(&data);
        sleep(1);
    }
    return 0;
}