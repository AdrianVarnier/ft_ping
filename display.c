#include "ft_ping.h"

void    display_dest_info(t_data *data, char* arg)
{
    struct sockaddr_in *addr_in = (struct sockaddr_in *)data->res->ai_addr;
    inet_ntop(AF_INET, &addr_in->sin_addr, data->ip, sizeof(data->ip));
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
    int loss = 100 - (((data->seq + 1) - data->errors) /(data->seq + 1)) * 100;
    if (loss == 100)
        mean = stddev = 0;
    printf("--- ft_ping statistics ---\n");
    printf("%d packets transmitted, %d packets received, %d%% packet loss\n", data->rtt_count + data->errors, data->rtt_count, loss);
    printf("round-trip min/avg/max/stddev = %.3f/%.3f/%.3f/%.3f ms\n", data->rtt_min, mean, data->rtt_max, stddev);
}

void    display_help(void)
{
    printf("Usage:\n");
    printf("\tft_ping [options] <destination>\n");
    printf("Options:\n");
    printf("\t-?\t\tdisplay help message\n");
    printf("\t-v\t\tverbose output\n");
    printf("\t-q\t\tquiet output\n");
    printf("\t-t <ttl>\tdefine time to live\n");
}