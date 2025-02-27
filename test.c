#include "ft_ping.h"

t_data data;
int ttl = 1;

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

void set_echo_header(t_data* data)
{
    data->header.type = ICMP_ECHO;
    data->header.code = 0;
    data->header.un.echo.id = getpid() & 0xFFFF;
    data->header.un.echo.sequence = 0;
    data->header.checksum = checksum(&data->header, sizeof(struct icmphdr));
}

void set_addr_hint(t_data* data)
{
    data->hints.ai_family = AF_INET;
    data->hints.ai_socktype = SOCK_RAW;
    data->hints.ai_protocol = IPPROTO_ICMP;
}

void fill_buffer(t_data *data)
{
    memcpy(data->buffer_send, &data->header, sizeof(struct icmphdr));
}

void increment_seq(t_data* data)
{
    data->header.un.echo.sequence++;
    data->header.checksum = checksum(&data->header, sizeof(struct icmphdr));
    fill_buffer(data);  
}

void init_data(t_data* data)
{
    memset(data, 0, sizeof(t_data));
    data->seq = -1;
    set_echo_header(data);
    set_addr_hint(data);
    fill_buffer(data);
}

int resolve_addr(t_data* data, char* addr)
{
    if (getaddrinfo(addr, NULL, &data->hints, &data->res) != 0)
    {
        fprintf(stderr, "ft_ping: unknown host\n");
        return -1;
    }
    return 0;
}

int send_ping(t_data* data, int sockfd)
{
    // Set TTL before sending the packet

    struct timeval start, end;
    gettimeofday(&start, NULL);

    // Debug: Print current TTL
    printf("Sending ICMP packet with TTL=%d\n", ttl);

    if (sendto(sockfd, data->buffer_send, sizeof(struct icmphdr), 0, 
               data->res->ai_addr, data->res->ai_addrlen) < 0)
    {
        perror("sendto failed");
        return 1;
    }

    socklen_t recv_len = data->res->ai_addrlen;
    memset(&data->buffer_received, 0, PACKET_SIZE + IP_HEADER_SIZE);
    data->bytes_received = recvfrom(sockfd, data->buffer_received, sizeof(data->buffer_received), 0,
                                    data->res->ai_addr, &data->res->ai_addrlen);

    if (data->bytes_received <= 0)
    {
        perror("recvfrom failed");
        return 1;
    }
    gettimeofday(&end, NULL);

    data->ttl = ((struct ip *)data->buffer_received)->ip_ttl;
    data->rtt = (end.tv_sec - start.tv_sec) * 1000.0 + (end.tv_usec - start.tv_usec) / 1000.0;
    return 0;
}

void display_dest_info(t_data *data, char* arg)
{
    inet_ntop(data->res->ai_family, (struct sockaddr_in *)data->res->ai_addr, data->ip, sizeof(data->ip));
    printf("PING %s (%s): %d bytes\n", arg, data->ip, PACKET_SIZE - 8);
}

void display_ping_info(t_data *data)
{
    printf("%d bytes from %s: icmp_seq=%d ttl=%d time=%.3f ms\n", 
           PACKET_SIZE, data->ip, data->seq, data->ttl, data->rtt);
}

void display_stats(t_data *data)
{
    double mean = data->rtt_sum / data->rtt_count;
    double stddev = sqrt((data->rtt_sqr_sum / data->rtt_count) - pow(mean, 2));
    int loss = 100 - (data->rtt_count / (data->seq + 1)) * 100;
    printf("--- ft_ping statistics ---\n");
    printf("%d packets transmitted, %d packets received, %d%% packet loss\n", 
           data->rtt_count + data->errors, data->rtt_count, loss);
    printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", 
           data->rtt_min, mean, data->rtt_max, stddev);
}

void update_rtt(t_data* data)
{
    data->rtt_count++;
    data->rtt_sum += data->rtt;
    data->rtt_sqr_sum += pow(data->rtt, 2);
    if (data->rtt_min > data->rtt || data->rtt_min == 0)
        data->rtt_min = data->rtt;
    if (data->rtt_max < data->rtt)
        data->rtt_max = data->rtt;   
}

void handle_response(t_data* data)
{
    struct icmphdr *response = (struct icmphdr *)(data->buffer_received + (((struct ip *)data->buffer_received)->ip_hl * 4));
    if (response->type == ICMP_ECHOREPLY)
    {
        update_rtt(data);
        display_ping_info(data);
    }
    else
        data->errors++;

    if (response->type == ICMP_DEST_UNREACH)
    {
        printf("Host Unreachable\n");
    }
    if (response->type == ICMP_TIME_EXCEEDED)
    {
        printf("icmp_seq=%d Time to live exceeded\n", data->seq);
    }
    if (response->type == ICMP_REDIRECT)
    {
        printf("Redirect Host\n");
    }
    if (response->type == ICMP_PARAMETERPROB)
    {
        printf("Parameter problem\n");
    }
}

void handle_sigint(int sig)
{
    display_stats(&data);
    freeaddrinfo(data.res);
    exit(0);
}

int main(int argc, char** argv)
{
    signal(SIGINT, handle_sigint);
    init_data(&data);
    
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <hostname>\n", argv[0]);
        return 1;
    }

    if (resolve_addr(&data, argv[1]) < 0)
        return 1;

    display_dest_info(&data, argv[1]);

    // Open socket once and reuse
    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }
    if (setsockopt(sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl)) != 0) {
        perror("setsockopt failed");
        return 1;
    }

    while (1)
    {
        data.seq++;
        ttl++;  // Increase TTL for each packet

        if (send_ping(&data, sockfd) == 0)
            handle_response(&data);
        
        sleep(1);
    }

    close(sockfd);
    return 0;
}
